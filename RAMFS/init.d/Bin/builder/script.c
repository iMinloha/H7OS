/**
 * script.c — 异步嵌入式脚本引擎
 *
 * 内置 I/O (直接调用软件层 dev_* 函数):
 *   open/close/read/write <dev> [args]
 *   var = read <dev> <len>        (读取并存到变量)
 *   write 支持 0xNN 十六进制字节
 *   if 支持 > < >= <= == != 比较
 */
#include "script.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "usbd_cdc_if.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define VN_MAX   16
#define VC_MAX   32

typedef struct { char n[VN_MAX]; int v; } var_t;
static var_t  _v[VC_MAX];
static int    _vc;

static var_t *vf(const char *name) {
    for (int i = 0; i < _vc; i++) if (!strcmp(_v[i].n, name)) return &_v[i];
    return NULL;
}
static var_t *va(const char *name) {
    var_t *x = vf(name); if (x) return x;
    if (_vc >= VC_MAX) return NULL;
    x = &_v[_vc++]; strncpy(x->n, name, VN_MAX-1); x->n[VN_MAX-1]=0; x->v=0;
    return x;
}

static void subst(char *out, int sz, const char *s) {
    char *o = out, *end = out + sz - 1;
    while (*s && o < end) {
        while (*s == ' ' || *s == '\t') { if (o<end) *o++ = *s; s++; }
        if (!*s) break;
        char tok[64]; int t=0;
        while (*s && *s!=' ' && *s!='\t' && t<63) tok[t++]=*s++;
        tok[t]=0;
        var_t *x = vf(tok);
        if (x) o += snprintf(o, end-o+1, "%d", x->v);
        else   { for (int i=0;i<t&&o<end;i++) *o++=tok[i]; }
    }
    *o=0;
}

/* ── 运算符 ─────────────────────────────────────────── */
typedef enum { OP_EQ, OP_NE, OP_GT, OP_LT, OP_GE, OP_LE } cmp_t;

static int cmp_val(int a, cmp_t op, int b) {
    switch(op){case OP_EQ:return a==b;case OP_NE:return a!=b;case OP_GT:return a>b;
    case OP_LT:return a<b;case OP_GE:return a>=b;case OP_LE:return a<=b;default:return 0;}
}

/* ── hex + ascii → binary ───────────────────────────── */
static int parse_bin(uint8_t *out, int max, const char *s) {
    int len = 0;
    while (*s && len < max) {
        while (*s == ' ') s++;
        if (!*s) break;
        if (s[0]=='0' && (s[1]=='x'||s[1]=='X') && s[2]) {
            char h[3]={s[2], (s[3]&&s[3]!=' ')?s[3]:0, 0};
            out[len++] = (uint8_t)strtol(h, NULL, 16);
            s += (s[3]&&s[3]!=' ') ? 4 : 3;
        } else {
            out[len++] = (uint8_t)*s++;
        }
    }
    return len;
}

/* ══════════════════════════════════════════════════════════════ */
#define LN_MAX  256

typedef struct { char *lines[LN_MAX]; int count; } lines_t;

static void lines_parse(lines_t *L, char *buf) {
    L->count = 0;
    char *save, *tok = strtok_r(buf, "\n", &save);
    while (tok && L->count < LN_MAX) {
        while (*tok==' '||*tok=='\t') tok++;
        if (*tok && *tok!='#') {
            char *e = tok+strlen(tok)-1;
            while (e>=tok && (*e=='\r'||*e==' '||*e=='\t')) *e--=0;
            if (*tok) L->lines[L->count++]=tok;
        }
        tok = strtok_r(NULL, "\n", &save);
    }
}

static int find_endfor(lines_t *L, int i) {
    int d=1;
    while (++i < L->count) {
        if      (!strncmp(L->lines[i],"for ",4))   d++;
        else if (!strncmp(L->lines[i],"while ",6))  d++;
        else if (!strcmp(L->lines[i],"endfor"))     {if(!--d) return i;}
        else if (!strcmp(L->lines[i],"endwhile"))   {if(!--d) return i;}
    }
    return -1;
}
static int find_endwhile(lines_t *L, int i) {
    int d=1;
    while (++i < L->count) {
        if      (!strncmp(L->lines[i],"while ",6))  d++;
        else if (!strncmp(L->lines[i],"for ",4))    d++;
        else if (!strcmp(L->lines[i],"endwhile"))   {if(!--d) return i;}
        else if (!strcmp(L->lines[i],"endfor"))     {if(!--d) return i;}
    }
    return -1;
}
static int find_else_endif(lines_t *L, int i, int *he) {
    int d=1; *he=0;
    while (++i < L->count) {
        if      (!strncmp(L->lines[i],"if ",3))   d++;
        else if (!strcmp(L->lines[i],"endif"))    {if(!--d) return i;}
        else if (d==1&&!strcmp(L->lines[i],"else")){*he=1; return i;}
    }
    return -1;
}

/* ══════════════════════════════════════════════════════════════ */
typedef struct { volatile int killed; int errs; const char *name;
                  uint8_t *wbuf; int wbuf_sz;        /* 预分配工作缓冲区, 避免反复 kernel_alloc */
} script_ctx_t;
#include "script_config.h"

#define MAX_ERRS  10

typedef enum { L_CMD, L_ASSIGN, L_FOR, L_IF, L_WHILE, L_DELAY,
               L_OPEN, L_CLOSE, L_READ, L_WRITE, L_READ_VAR, L_INC } lk_t;

typedef struct {
    lk_t kind;   char dev[128], buf[256];  int v_int;  cmp_t if_op;
    char fv[VN_MAX]; int fs, fe;           char iv[VN_MAX];
} li_t;

static const char *write_data_ptr(const char *raw) {
    int skip = (!strncmp(raw,"use ",4)) ? 3 : 2;
    while (*raw==' '||*raw=='\t') raw++;
    for (int i=0;i<skip;i++) {
        while (*raw&&*raw!=' '&&*raw!='\t') raw++;
        while (*raw==' '||*raw=='\t') raw++;
    }
    return raw;
}

/* ── 解析比较运算符 ─────────────────────────────────── */
static int parse_cmp(const char *op, cmp_t *c) {
    if      (!strcmp(op,"==")||!strcmp(op,"is")) *c=OP_EQ;
    else if (!strcmp(op,"!=")) *c=OP_NE;
    else if (!strcmp(op,">=")) *c=OP_GE;
    else if (!strcmp(op,"<=")) *c=OP_LE;
    else if (!strcmp(op,">"))  *c=OP_GT;
    else if (!strcmp(op,"<"))  *c=OP_LT;
    else return 0;
    return 1;
}

/* ── 简单表达式求值: subst 替换变量后, 算 + - * / ── */
static int eval_expr(const char *s) {
    char buf[128]; subst(buf,sizeof(buf),s);
    int a,b; char op;
    /* 两操作数: a op b */
    if (sscanf(buf,"%d %c %d",&a,&op,&b)==3) {
        switch(op){case '+':return a+b;case '-':return a-b;
        case '*':return a*b;case '/':return b?a/b:0;}
    }
    /* 单数字 */
    if (sscanf(buf,"%d",&a)==1) return a;
    return 0;
}

static void li_parse(li_t *info, const char *raw) {
    memset(info,0,sizeof(*info));
    info->if_op = OP_EQ;  /* default */
    char vn[VN_MAX], path[128], op[16]; int v1,v2; char eq;

    /* ── built-in I/O ──────────────────────────────── */
    if (sscanf(raw,"delay(%d)",&v1)==1&&v1>0)  {info->kind=L_DELAY; info->v_int=v1; return;}
    if (sscanf(raw,"open %127s",path)==1)       {info->kind=L_OPEN; subst(info->dev,sizeof(info->dev),path); return;}
    if (sscanf(raw,"close %127s",path)==1)      {info->kind=L_CLOSE; subst(info->dev,sizeof(info->dev),path); return;}
    if (sscanf(raw,"read %127s %d",path,&v1)==2&&v1>0)
        {info->kind=L_READ; subst(info->dev,sizeof(info->dev),path); info->v_int=v1; return;}
    if (sscanf(raw,"write %127s",path)==1)
        {info->kind=L_WRITE; subst(info->dev,sizeof(info->dev),path); return;}
    /* var = read <dev> <len> */
    if (sscanf(raw,"%15s = read %127s %d",vn,path,&v1)==3 && v1>0)
        {info->kind=L_READ_VAR; strncpy(info->iv,vn,VN_MAX-1);
         subst(info->dev,sizeof(info->dev),path); info->v_int=v1; return;}

    /* ── use 兼容 ──────────────────────────────────── */
    if (sscanf(raw,"use %127s %15s",path,op)==2) {
        subst(info->dev,sizeof(info->dev),path);
        if      (!strcmp(op,"open"))  {info->kind=L_OPEN; return;}
        else if (!strcmp(op,"close")) {info->kind=L_CLOSE; return;}
        else if (!strcmp(op,"read")&&sscanf(raw,"use %*s read %d",&v1)==1)
            {info->kind=L_READ; info->v_int=v1>0?v1:64; return;}
        else if (!strcmp(op,"write")) {info->kind=L_WRITE; return;}
    }

    /* ── control flow ──────────────────────────────── */
    if (sscanf(raw,"for %15s %d:%d",vn,&v1,&v2)==3)
        {info->kind=L_FOR; strncpy(info->fv,vn,VN_MAX-1); info->fs=v1; info->fe=v2; return;}
    /* if var op val — op 可能是 >  <  >=  <=  ==  !=  is */
    if (sscanf(raw,"if %15s %3s %d",vn,op,&v1)==3 && parse_cmp(op,&info->if_op))
        {info->kind=L_IF; strncpy(info->iv,vn,VN_MAX-1); info->v_int=v1; return;}
    /* while var op val */
    if (sscanf(raw,"while %15s %3s %d",vn,op,&v1)==3 && parse_cmp(op,&info->if_op))
        {info->kind=L_WHILE; strncpy(info->iv,vn,VN_MAX-1); info->v_int=v1; return;}
    if (sscanf(raw,"while %d",&v1)==1 && v1!=0)
        {info->kind=L_WHILE; info->iv[0]='\0'; info->v_int=1; return;}
    /* var++ / var-- */
    if (sscanf(raw,"%15s++",vn)==1)
        {info->kind=L_INC;strncpy(info->iv,vn,VN_MAX-1);info->v_int=1;return;}
    if (sscanf(raw,"%15s--",vn)==1)
        {info->kind=L_INC;strncpy(info->iv,vn,VN_MAX-1);info->v_int=-1;return;}
    /* var = expr (算术表达式) */
    if (sscanf(raw,"%15s %c",vn,&eq)==2 && eq=='=') {
        info->kind=L_ASSIGN;
        strncpy(info->iv,vn,VN_MAX-1);
        /* 提取 = 右侧表达式 */
        const char *rhs = strchr(raw,'=')+1;
        while (*rhs==' ')rhs++;
        subst(info->buf,sizeof(info->buf),rhs);
        return;
    }

    info->kind=L_CMD; subst(info->buf,sizeof(info->buf),raw);
}

/* ══════════════════════════════════════════════════════════════ */

static int  exec_line(script_ctx_t *ctx, li_t *in, const char *raw);
static void exec_range(script_ctx_t *ctx, lines_t *L, int start, int end);
static void exec_for(script_ctx_t *ctx, lines_t *L, int i);
static void exec_if(script_ctx_t *ctx, lines_t *L, int i);
static void exec_while(script_ctx_t *ctx, lines_t *L, int i);

static int exec_line(script_ctx_t *ctx, li_t *in, const char *raw) {
    int r = 0;
    switch (in->kind) {
    case L_ASSIGN: {
        if (in->iv[0]) { var_t *x=va(in->iv); if(x)x->v=eval_expr(in->buf); }
        break;
    }
    case L_INC: {
        var_t *x=va(in->iv); if(x)x->v+=in->v_int;
        break;
    }
    case L_OPEN:  r=dev_open(in->dev); break;
    case L_CLOSE: r=dev_close(in->dev); break;
    case L_READ: case L_READ_VAR: {
        r = dev_open(in->dev);
        if (r==0 || r==-2) {
            int n = in->v_int>0 ? in->v_int : 64;
            if (n>512) n=512;
            if (ctx->wbuf && n < ctx->wbuf_sz) {
                memset(ctx->wbuf, 0, n+1);
                r = dev_read(in->dev, ctx->wbuf, n);
                if (r>0 && in->kind==L_READ_VAR) {
                    ctx->wbuf[r]='\0';
                    var_t *v = va(in->iv);
                    if (v) v->v = atoi((char*)ctx->wbuf);
                }
            }
        }
        break;
    }
    case L_WRITE: {
        r = dev_open(in->dev);
        if (r==0 || r==-2) {
            const char *dp = write_data_ptr(raw);
            char ds[128]; subst(ds,sizeof(ds),dp);
            int blen = parse_bin(ctx->wbuf, ctx->wbuf_sz, ds);
            if (blen>0) r = dev_write(in->dev, ctx->wbuf, blen);
        }
        break;
    }
    case L_DELAY: osDelay(in->v_int); return 0;
    case L_CMD:   if (in->buf[0]) execCMD(in->buf); return 0;
    default: return 0;
    }

    /* error tracking */
    if (r < 0) {
        ctx->errs++;
        if (ctx->errs==1) {
            const char *op="?";
            switch(in->kind){case L_OPEN:op="open";break;case L_CLOSE:op="close";break;
            case L_READ:case L_READ_VAR:op="read";break;case L_WRITE:op="write";break;
            default:break;}
            USB_color_printf(LIGHT_RED,"  [%s] %s %s -> %d\n",
                             ctx->name?ctx->name:"?",op,in->dev,r);
        }
        if (ctx->errs >= MAX_ERRS) {
            USB_color_printf(LIGHT_RED,"  [%s] aborted (%d errors)\n",
                             ctx->name?ctx->name:"?",ctx->errs);
            ctx->killed=1;
        }
    } else { ctx->errs=0; }
    return r;
}

static void exec_range(script_ctx_t *ctx, lines_t *L, int start, int end) {
    for (int i=start; i<end && !ctx->killed; ) {
        char *ln = L->lines[i];
        if (!strncmp(ln,"for ",4)) {
            exec_for(ctx,L,i); i=find_endfor(L,i); if(i<0)i=end;else i++;
        } else if (!strncmp(ln,"while ",6)) {
            exec_while(ctx,L,i); i=find_endwhile(L,i); if(i<0)i=end;else i++;
        } else if (!strncmp(ln,"if ",3)) {
            int he; int sp=find_else_endif(L,i,&he);
            if (sp<0){i=end;continue;}
            exec_if(ctx,L,i);
            if (he) i=find_else_endif(L,sp,&he); else i=sp;
            if (i<0)i=end;else i++;
        } else if (!strcmp(ln,"else")||!strcmp(ln,"endif")||
                   !strcmp(ln,"endfor")||!strcmp(ln,"endwhile")) {
            i++;
        } else {
            li_t info; li_parse(&info,ln);
            exec_line(ctx,&info,ln); i++;
        }
    }
}

static void exec_for(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info,L->lines[i]);
    var_t *v=va(info.fv); if(!v)return;
    int tail=find_endfor(L,i); if(tail<0)return;
    for (int k=info.fs;k<=info.fe&&!ctx->killed;k++) {v->v=k; exec_range(ctx,L,i+1,tail);}
}

static void exec_if(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info,L->lines[i]);
    var_t *v=vf(info.iv);
    int cond=v ? cmp_val(v->v,info.if_op,info.v_int) : 0;
    int he; int sp=find_else_endif(L,i,&he); if(sp<0)return;
    if (cond) exec_range(ctx,L,i+1,sp);
    else if (he) { int ei=find_else_endif(L,sp,&he); if(ei>=0)exec_range(ctx,L,sp+1,ei); }
}

static void exec_while(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info,L->lines[i]);
    int tail=find_endwhile(L,i); if(tail<0)return;
    int inf=(info.iv[0]=='\0');
    while (!ctx->killed) {
        if (!inf) { var_t *v=vf(info.iv); if(!v||!cmp_val(v->v,info.if_op,info.v_int)) break; }
        exec_range(ctx,L,i+1,tail);
    }
}

/* ══════════════════════════════════════════════════════════════ */

typedef struct { osThreadId handle; Task_t task; char name[20]; script_ctx_t ctx; } script_job_t;
static script_job_t _jobs[SCRIPT_MAX_JOBS];
static int _job_cnt;

static script_job_t *job_find_by_name(const char *n) {
    for (int i=0;i<_job_cnt;i++) if(!strcmp(_jobs[i].name,n)) return &_jobs[i];
    return NULL;
}

typedef struct { char *script; char name[20]; } script_arg_t;

static void script_thread(void const *arg) {
    script_arg_t *sa=(script_arg_t*)arg;
    script_job_t *job=job_find_by_name(sa->name);
    if(!job){kernel_free(sa->script);kernel_free(sa);return;}
    job->handle=osThreadGetId();
    if(job->task)job->task->handle=job->handle;

    _vc=0;
    char *buf=kernel_alloc(strlen(sa->script)+1);
    strcpy(buf,sa->script);
    lines_t *L=kernel_alloc(sizeof(lines_t));
    lines_parse(L,buf);
    exec_range(&job->ctx,L,0,L->count);
    kernel_free(L); kernel_free(buf);

    if(job->task){job->task->status=TASK_STOP;job->task->handle=NULL;}
    if(job->ctx.wbuf){kernel_free(job->ctx.wbuf);job->ctx.wbuf=NULL;}
    for (int i=0;i<_job_cnt;i++) if(&_jobs[i]==job){_jobs[i]=_jobs[--_job_cnt];break;}
    kernel_free(sa->script); kernel_free(sa);
    osThreadTerminate(NULL);
}

osThreadId script_run_async(const char *script, const char *name) {
    if(!script||!*script||!name) return NULL;
    if(_job_cnt>=SCRIPT_MAX_JOBS) return NULL;
    extern uint8_t PID_Global;

    script_job_t *job=&_jobs[_job_cnt++];
    memset(job,0,sizeof(*job));
    strncpy(job->name,name,19);job->name[19]=0;
    job->ctx.killed=0;job->ctx.name=job->name;
    job->ctx.wbuf=kernel_alloc(SCRIPT_WBUF_SZ);job->ctx.wbuf_sz=SCRIPT_WBUF_SZ;

    Task_t t=(Task_t)kernel_alloc(sizeof(struct Task));
    t->name=kernel_alloc(strlen(name)+1);strcpy(t->name,name);
    t->status=TASK_READY;t->priority=TASK_PRIORITY_NORMAL;
    t->handle=NULL;t->cpu=0;t->next=NULL;
    t->lastWakeTime=0;t->accumulatedTime=0;t->PID=PID_Global++;
    job->task=t; addThread(t);

    script_arg_t *sa=kernel_alloc(sizeof(script_arg_t));
    sa->script=kernel_alloc(strlen(script)+1);
    strcpy(sa->script,script);
    strncpy(sa->name,name,19);sa->name[19]=0;

    osThreadDef(scriptT,script_thread,osPriorityBelowNormal,0,SCRIPT_STACK_SZ);
    osThreadId h=osThreadCreate(osThread(scriptT),sa);
    if(!h){kernel_free(sa->script);kernel_free(sa);kernel_free(t->name);kernel_free(t);_job_cnt--;return NULL;}
    job->handle=h;t->handle=h;
    return h;
}

int script_kill(const char *name) {
    script_job_t *job=job_find_by_name(name);
    if(!job)return -1;
    job->ctx.killed=1;
    if(job->handle){osThreadTerminate(job->handle);job->handle=NULL;}
    if(job->ctx.wbuf){kernel_free(job->ctx.wbuf);job->ctx.wbuf=NULL;}
    FS_t proc=getFSChild(RAM_FS,"proc");
    if(proc&&job->task){
        Task_t p=proc->tasklist,prev=NULL;
        while(p){if(p==job->task){if(prev)prev->next=p->next;else proc->tasklist=p->next;break;}prev=p;p=p->next;}
        kernel_free(job->task->name);kernel_free(job->task);job->task=NULL;
    }
    for(int i=0;i<_job_cnt;i++)if(&_jobs[i]==job){_jobs[i]=_jobs[--_job_cnt];break;}
    return 0;
}

void script_list(void) {
    if(_job_cnt==0){USB_printf("(no running scripts)\n");return;}
    for(int i=0;i<_job_cnt;i++){
        script_job_t *j=&_jobs[i];
        USB_printf("%-20s  %s\n",j->name,j->ctx.killed?"KILLED":"RUNNING");
    }
}
