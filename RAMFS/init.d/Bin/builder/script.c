/**
 * script.c — 异步嵌入式脚本引擎
 *
 * 内置 I/O (直接调用软件层 dev_* 函数, 不经过 execCMD):
 *   open  <device>          → dev_open()
 *   close <device>          → dev_close()
 *   read  <device> <len>    → dev_read()
 *   write <device> <data>   → dev_write()
 *   delay(ms)               → osDelay()
 *
 * 其他命令 → execCMD() 分发
 */
#include "script.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "usbd_cdc_if.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ══════════════════════════════════════════════════════════════
 *  变量表
 * ══════════════════════════════════════════════════════════════ */
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

/* ══════════════════════════════════════════════════════════════
 *  行数组
 * ══════════════════════════════════════════════════════════════ */
#define LN_MAX  256

typedef struct {
    char *lines[LN_MAX];
    int   count;
} lines_t;

static void lines_parse(lines_t *L, char *buf) {
    L->count = 0;
    char *save;
    char *tok = strtok_r(buf, "\n", &save);
    while (tok && L->count < LN_MAX) {
        while (*tok==' '||*tok=='\t') tok++;
        if (*tok && *tok!='#') {
            char *e = tok+strlen(tok)-1;
            while (e>=tok && (*e=='\r'||*e==' '||*e=='\t')) *e--=0;
            if (*tok) L->lines[L->count++] = tok;
        }
        tok = strtok_r(NULL, "\n", &save);
    }
}

static int find_endfor(lines_t *L, int i) {
    int d = 1;
    while (++i < L->count) {
        if      (!strncmp(L->lines[i],"for ",4))   d++;
        else if (!strncmp(L->lines[i],"while ",6))  d++;
        else if (!strcmp(L->lines[i],"endfor"))     { if (!--d) return i; }
        else if (!strcmp(L->lines[i],"endwhile"))   { if (!--d) return i; }
    }
    return -1;
}

static int find_endwhile(lines_t *L, int i) {
    int d = 1;
    while (++i < L->count) {
        if      (!strncmp(L->lines[i],"while ",6))  d++;
        else if (!strncmp(L->lines[i],"for ",4))    d++;
        else if (!strcmp(L->lines[i],"endwhile"))   { if (!--d) return i; }
        else if (!strcmp(L->lines[i],"endfor"))     { if (!--d) return i; }
    }
    return -1;
}

static int find_else_endif(lines_t *L, int i, int *has_else) {
    int d = 1;  *has_else = 0;
    while (++i < L->count) {
        if      (!strncmp(L->lines[i],"if ",3))   d++;
        else if (!strcmp(L->lines[i],"endif"))    { if (!--d) return i; }
        else if (d==1 && !strcmp(L->lines[i],"else")) { *has_else=1; return i; }
    }
    return -1;
}

/* ══════════════════════════════════════════════════════════════
 *  中断标志 — 每个脚本独立
 * ══════════════════════════════════════════════════════════════ */
typedef struct {
    volatile int killed;
    int          errs;
    const char  *name;       /* 脚本名称 (用于错误输出) */
} script_ctx_t;

#define MAX_CONSECUTIVE_ERRS  10   /* 连续错误超此数自动终止 */

/* ══════════════════════════════════════════════════════════════
 *  行分类
 * ══════════════════════════════════════════════════════════════ */
typedef enum { L_CMD, L_ASSIGN, L_FOR, L_IF, L_WHILE, L_DELAY,
               L_OPEN, L_CLOSE, L_READ, L_WRITE } lk_t;

typedef struct {
    lk_t kind;
    char dev[128];
    char buf[256];
    int  v_int;               /* delay_ms / read_len / if_cmp / while_cmp */
    char fv[VN_MAX]; int fs, fe;            /* L_FOR   */
    char iv[VN_MAX];                       /* L_IF / L_WHILE */
} li_t;

/* 从 raw 行提取 data 指针. 支持:
 *   "write <dev> <data>"  跳过 2 个 token
 *   "use <dev> write <data>" 跳过 3 个 token */
static const char *write_data_ptr(const char *raw) {
    int skip = (!strncmp(raw,"use ",4)) ? 3 : 2;
    while (*raw==' '||*raw=='\t') raw++;
    for (int i=0; i<skip; i++) {
        while (*raw && *raw!=' ' && *raw!='\t') raw++;  /* skip token */
        while (*raw==' '||*raw=='\t') raw++;             /* skip space */
    }
    return raw;
}

static void li_parse(li_t *info, const char *raw) {
    memset(info,0,sizeof(*info));
    char vn[VN_MAX], path[128], op[16]; int v1,v2; char eq;

    /* ── built-in I/O ──────────────────────────────── */
    if (sscanf(raw,"delay(%d)",&v1)==1 && v1>0)
        { info->kind=L_DELAY; info->v_int=v1; return; }
    if (sscanf(raw,"open %127s",path)==1)
        { info->kind=L_OPEN; subst(info->dev,sizeof(info->dev),path); return; }
    if (sscanf(raw,"close %127s",path)==1)
        { info->kind=L_CLOSE; subst(info->dev,sizeof(info->dev),path); return; }
    if (sscanf(raw,"read %127s %d",path,&v1)==2 && v1>0)
        { info->kind=L_READ; subst(info->dev,sizeof(info->dev),path);
          info->v_int=v1; return; }
    if (sscanf(raw,"write %127s",path)==1)
        { info->kind=L_WRITE; subst(info->dev,sizeof(info->dev),path); return; }

    /* ── 兼容旧 use 语法: use <dev> open/close/read/write ─ */
    if (sscanf(raw,"use %127s %15s",path,op)==2) {
        subst(info->dev,sizeof(info->dev),path);
        if      (!strcmp(op,"open"))  { info->kind=L_OPEN;  return; }
        else if (!strcmp(op,"close")) { info->kind=L_CLOSE; return; }
        else if (!strcmp(op,"read") && sscanf(raw,"use %*s read %d",&v1)==1)
            { info->kind=L_READ; info->v_int=v1>0?v1:64; return; }
        else if (!strcmp(op,"write")) { info->kind=L_WRITE; return; }
        /* 其他 use 操作 → execCMD */
    }

    /* ── control flow ──────────────────────────────── */
    if (sscanf(raw,"for %15s %d:%d",vn,&v1,&v2)==3)
        { info->kind=L_FOR; strncpy(info->fv,vn,VN_MAX-1);
          info->fs=v1; info->fe=v2; return; }
    if (sscanf(raw,"if %15s is %d",vn,&v1)==2)
        { info->kind=L_IF; strncpy(info->iv,vn,VN_MAX-1); info->v_int=v1; return; }
    /* while var is val  /  while 1 (无限循环) */
    if (sscanf(raw,"while %15s is %d",vn,&v1)==2)
        { info->kind=L_WHILE; strncpy(info->iv,vn,VN_MAX-1); info->v_int=v1; return; }
    if (sscanf(raw,"while %d",&v1)==1 && v1 != 0)
        { info->kind=L_WHILE; info->iv[0]='\0'; info->v_int=1; return; }
    if (sscanf(raw,"%15s %c %d",vn,&eq,&v1)==3 && eq=='=')
        { info->kind=L_ASSIGN; return; }

    /* fallback → execCMD */
    info->kind=L_CMD; subst(info->buf,sizeof(info->buf),raw);
}

/* ══════════════════════════════════════════════════════════════
 *  解释器 — 递归下降, 每行检查 kill 标志
 * ══════════════════════════════════════════════════════════════ */

static int  exec_line(script_ctx_t *ctx, li_t *in, const char *raw);
static void exec_range(script_ctx_t *ctx, lines_t *L, int start, int end);
static void exec_for(script_ctx_t *ctx, lines_t *L, int i);
static void exec_if(script_ctx_t *ctx, lines_t *L, int i);
static void exec_while(script_ctx_t *ctx, lines_t *L, int i);

/* exec_line: 返回 0=成功, <0=失败 */
static int exec_line(script_ctx_t *ctx, li_t *in, const char *raw) {
    int r = 0;
    switch (in->kind) {
    case L_ASSIGN: {
        /* 从 raw 行解析, 不通过 subst (避免变量名被替换) */
        char vn[VN_MAX], eq; int vv;
        if (sscanf(raw,"%15s %c %d",vn,&eq,&vv)==3 && eq=='=') {
            var_t *x = va(vn); if (x) x->v = vv;
        }
        break;
    }
    case L_OPEN:   r = dev_open(in->dev);   break;
    case L_CLOSE:  r = dev_close(in->dev);  break;
    case L_READ: {
        r = dev_open(in->dev);
        if (r == 0 || r == -2) {  /* -2 = 已经打开 (可能是自己) */
            int n = in->v_int > 0 ? in->v_int : 64;
            if (n > 512) n = 512;
            uint8_t *rb = kernel_alloc(n + 1);
            memset(rb, 0, n + 1);
            r = dev_read(in->dev, rb, n);
            kernel_free(rb);
        }
        break;
    }
    case L_WRITE: {
        r = dev_open(in->dev);
        if (r == 0 || r == -2) {
            const char *dp = write_data_ptr(raw);
            char ds[128]; subst(ds, sizeof(ds), dp);
            r = dev_write(in->dev, (const uint8_t*)ds, strlen(ds));
        }
        break;
    }
    case L_DELAY:  osDelay(in->v_int);  return 0;
    case L_CMD:    if (in->buf[0]) execCMD(in->buf);  return 0;
    default: return 0;
    }

    /* 错误处理: 超限自动终止, 终止时输出原因 */
    if (r < 0) {
        ctx->errs++;
        if (ctx->errs == 1) {  /* 仅第一个错误输出, 不刷屏 */
            const char *op = "?";
            switch (in->kind) {
            case L_OPEN: op="open"; break; case L_CLOSE: op="close"; break;
            case L_READ: op="read"; break; case L_WRITE: op="write"; break;
            default: break;
            }
            USB_color_printf(LIGHT_RED, "  [%s] %s %s → %d\n",
                             ctx->name ? ctx->name : "?", op, in->dev, r);
        }
        if (ctx->errs >= MAX_CONSECUTIVE_ERRS) {
            USB_color_printf(LIGHT_RED, "  [%s] aborted (%d errors)\n",
                             ctx->name ? ctx->name : "?", ctx->errs);
            ctx->killed = 1;
        }
    } else {
        ctx->errs = 0;
    }
    return r;
}

static void exec_range(script_ctx_t *ctx, lines_t *L, int start, int end) {
    for (int i = start; i < end && !ctx->killed; ) {
        char *ln = L->lines[i];
        if (!strncmp(ln, "for ", 4)) {
            exec_for(ctx, L, i);
            i = find_endfor(L, i);
            if (i < 0) i = end; else i++;
        } else if (!strncmp(ln, "while ", 6)) {
            exec_while(ctx, L, i);
            i = find_endwhile(L, i);
            if (i < 0) i = end; else i++;
        } else if (!strncmp(ln, "if ", 3)) {
            int has_else;
            int split = find_else_endif(L, i, &has_else);
            if (split < 0) { i = end; continue; }
            exec_if(ctx, L, i);
            if (has_else) i = find_else_endif(L, split, &has_else);
            else          i = split;
            if (i < 0) i = end; else i++;
        } else if (!strcmp(ln, "else") || !strcmp(ln, "endif") || !strcmp(ln, "endfor") || !strcmp(ln, "endwhile")) {
            i++;
        } else {
            li_t info; li_parse(&info, ln);
            exec_line(ctx, &info, ln);
            i++;
        }
    }
}

static void exec_for(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info, L->lines[i]);
    var_t *v = va(info.fv); if (!v) return;
    int tail = find_endfor(L, i);
    if (tail < 0) return;
    for (int k = info.fs; k <= info.fe && !ctx->killed; k++) {
        v->v = k;
        exec_range(ctx, L, i + 1, tail);
    }
}

static void exec_if(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info, L->lines[i]);
    var_t *v = vf(info.iv);
    int cond = v ? (v->v == info.v_int) : 0;
    int has_else;
    int split = find_else_endif(L, i, &has_else);
    if (split < 0) return;
    if (cond) {
        exec_range(ctx, L, i + 1, split);
    } else if (has_else) {
        int endi = find_else_endif(L, split, &has_else);
        if (endi >= 0) exec_range(ctx, L, split + 1, endi);
    }
}

static void exec_while(script_ctx_t *ctx, lines_t *L, int i) {
    li_t info; li_parse(&info, L->lines[i]);
    int tail = find_endwhile(L, i);
    if (tail < 0) return;

    /* while 1 → 无限循环; while var is val → 条件循环 */
    int inf = (info.iv[0] == '\0');  /* 无变量名 = while 1 */

    while (!ctx->killed) {
        if (!inf) {
            var_t *v = vf(info.iv);
            if (!v || v->v != info.v_int) break;
        }
        exec_range(ctx, L, i + 1, tail);
    }
}

/* ══════════════════════════════════════════════════════════════
 *  脚本任务管理
 * ══════════════════════════════════════════════════════════════ */

typedef struct {
    osThreadId  handle;
    Task_t      task;       /* RAMFS 任务节点 */
    char        name[20];
    script_ctx_t ctx;
} script_job_t;

#define MAX_JOBS 4
static script_job_t _jobs[MAX_JOBS];
static int           _job_cnt;

static script_job_t *job_find_by_handle(osThreadId h) {
    for (int i=0;i<_job_cnt;i++) if (_jobs[i].handle==h) return &_jobs[i];
    return NULL;
}

static script_job_t *job_find_by_name(const char *name) {
    for (int i=0;i<_job_cnt;i++) if (!strcmp(_jobs[i].name,name)) return &_jobs[i];
    return NULL;
}

/* ── 脚本线程入口 ──────────────────────────────────── */
typedef struct {
    char   *script;
    char    name[20];
} script_arg_t;

static void script_thread(void const *arg) {
    script_arg_t *sa = (script_arg_t*)arg;

    /* 查找 job */
    script_job_t *job = NULL;
    for (int i=0;i<_job_cnt;i++) {
        if (!strcmp(_jobs[i].name, sa->name)) { job=&_jobs[i]; break; }
    }
    if (!job) { kernel_free(sa->script); kernel_free(sa); return; }
    job->handle = osThreadGetId();

    /* 更新 RAMFS 任务节点的 handle */
    if (job->task) job->task->handle = job->handle;

    /* 执行脚本 (lines_t 太大不能放栈上, 堆分配) */
    _vc = 0;
    char *buf = kernel_alloc(strlen(sa->script) + 1);
    strcpy(buf, sa->script);
    lines_t *L = kernel_alloc(sizeof(lines_t));
    lines_parse(L, buf);
    exec_range(&job->ctx, L, 0, L->count);
    kernel_free(L);
    kernel_free(buf);

    /* 标记任务为停止 (保留在 /proc 列表中, tree 可见) */
    if (job->task) {
        job->task->status = TASK_STOP;
        job->task->handle = NULL;
    }

    /* 从 job 表移除 */
    for (int i=0;i<_job_cnt;i++) {
        if (&_jobs[i] == job) { _jobs[i] = _jobs[--_job_cnt]; break; }
    }

    kernel_free(sa->script);
    kernel_free(sa);
    osThreadTerminate(NULL);
}

/* ══════════════════════════════════════════════════════════════
 *  public API
 * ══════════════════════════════════════════════════════════════ */

osThreadId script_run_async(const char *script, const char *name) {
    if (!script || !*script || !name) return NULL;
    if (_job_cnt >= MAX_JOBS) return NULL;

    extern uint8_t PID_Global;

    /* 注册 job */
    script_job_t *job = &_jobs[_job_cnt++];
    memset(job, 0, sizeof(*job));
    strncpy(job->name, name, 19); job->name[19]=0;
    job->ctx.killed = 0;
    job->ctx.name   = job->name;

    /* 创建 RAMFS 任务节点 (info 可见) */
    Task_t t = (Task_t)kernel_alloc(sizeof(struct Task));
    t->name = kernel_alloc(strlen(name) + 1);
    strcpy(t->name, name);
    t->status = TASK_READY;
    t->priority = TASK_PRIORITY_NORMAL;
    t->handle = NULL;
    t->cpu = 0;
    t->next = NULL;
    t->lastWakeTime = 0;
    t->accumulatedTime = 0;
    t->PID = PID_Global++;
    job->task = t;

    /* 注册到 /proc */
    addThread(t);

    /* 准备参数 */
    script_arg_t *sa = kernel_alloc(sizeof(script_arg_t));
    sa->script = kernel_alloc(strlen(script) + 1);
    strcpy(sa->script, script);
    strncpy(sa->name, name, 19); sa->name[19]=0;

    /* 创建线程 (优先级低于 Shell, 不阻塞命令输入) */
    osThreadDef(scriptT, script_thread, osPriorityBelowNormal, 0, 2048);
    osThreadId h = osThreadCreate(osThread(scriptT), sa);
    if (!h) {
        kernel_free(sa->script); kernel_free(sa);
        kernel_free(t->name); kernel_free(t);
        _job_cnt--;
        return NULL;
    }
    job->handle = h;
    t->handle = h;
    return h;
}

int script_kill(const char *name) {
    script_job_t *job = job_find_by_name(name);
    if (!job) return -1;

    /* 设置 kill 标志 (如果线程还在运行) */
    job->ctx.killed = 1;
    if (job->handle) {
        osThreadTerminate(job->handle);
        job->handle = NULL;
    }

    /* 从 /proc 移除任务节点 */
    FS_t proc = getFSChild(RAM_FS, "proc");
    if (proc && job->task) {
        Task_t p = proc->tasklist, prev = NULL;
        while (p) {
            if (p == job->task) {
                if (prev) prev->next = p->next;
                else      proc->tasklist = p->next;
                break;
            }
            prev = p;
            p = p->next;
        }
        kernel_free(job->task->name);
        kernel_free(job->task);
        job->task = NULL;
    }

    /* 释放 job 槽位 */
    for (int i = 0; i < _job_cnt; i++) {
        if (&_jobs[i] == job) { _jobs[i] = _jobs[--_job_cnt]; break; }
    }
    return 0;
}

void script_list(void) {
    if (_job_cnt == 0) { USB_printf("(no running scripts)\n"); return; }
    for (int i=0;i<_job_cnt;i++) {
        script_job_t *j = &_jobs[i];
        const char *st = j->ctx.killed ? "KILLED" : "RUNNING";
        USB_printf("%-20s  %s\n", j->name, st);
    }
}
