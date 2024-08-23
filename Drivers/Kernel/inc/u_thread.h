#ifndef H7OS_U_THREAD_H
#define H7OS_U_THREAD_H

#include <stdatomic.h>

typedef struct param* param_t;

/**
 * @brief º¯ÊıÖ¸Õë
 * */
typedef void (*Func_t)(param_t param);

/**
 * @brief »¥³âËø
 * @param lockflag Ëø±êÖ¾
 * @param count Ëø¼ÆÊı
 * @param owner ËøÓµÓĞÕß
 * @namespace lock ¼ÓËø·½·¨
 * @namespace unlock ½âËø
 * @namespace status Ëø×´Ì¬
 * */
struct _mutex {
    atomic_int lockflag;
    atomic_int count;
    atomic_int owner;

    void (*lock)(struct _mutex *self);
    void (*unlock)(struct _mutex *self);
    int (*status)(struct _mutex *self);
};
/**
 * @brief »¥³âËøÀàĞÍ
 * */
typedef struct _mutex* Mutex_t;

/**
 * @brief »¥³âËø³õÊ¼»¯
 * @param self »¥³âËø
 */
void mutex_init(Mutex_t self);

/**
 * @brief »¥³âËø¼ÓËø
 * @param self »¥³âËø
 */
void mutex_lock(Mutex_t self);

/**
 * @brief »¥³âËø½âËø
 * @param self »¥³âËø
 * */
void mutex_unlock(Mutex_t self);

/**
 * @brief »¥³âËø×´Ì¬
 * @param self »¥³âËø
 * */
int mutex_status(Mutex_t self);

/**
 * @brief »¥³âËøÖ´ĞĞº¯Êı
 * @param self »¥³âËø
 * @param func º¯Êı
 * */
void LockFunc(Mutex_t self, Func_t func, struct param *param);

#endif
