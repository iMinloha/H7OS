#ifndef H7OS_U_THREAD_H
#define H7OS_U_THREAD_H

#include <stdatomic.h>

typedef enum{
    True,
    False,
} Bool;

typedef struct param* param_t;

/**
 * @brief ����ָ��
 * @param param �����ṹ��
 * */
typedef void (*Func_t)(param_t param);

/**
 * @brief ������
 * @param lockflag ����־
 * @param count ������
 * @param owner ��ӵ����
 * @namespace lock ��������
 * @namespace unlock ����
 * @namespace status ��״̬
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
 * @brief ����������
 * */
typedef struct _mutex* Mutex_t;

#define MUTEX_SIZE sizeof(struct _mutex)

/**
 * @brief ��������ʼ��
 * @param self ������
 */
void mutex_init(Mutex_t self);

/**
 * @brief ����������
 * @param self ������
 */
void mutex_lock(Mutex_t self);

/**
 * @brief ����������
 * @param self ������
 * */
void mutex_unlock(Mutex_t self);

/**
 * @brief ������״̬
 * @param self ������
 * */
int mutex_status(Mutex_t self);

/**
 * @brief ������ִ�к���
 * @param self ������
 * @param func ����
 * */
void LockFunc(Mutex_t self, Func_t func, param_t param);

#endif
