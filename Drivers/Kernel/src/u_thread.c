#include "u_thread.h"

void mutex_init(Mutex_t self){
    self->lockflag = 0;
    self->count = 0;
    self->owner = 0;

    self->lock = mutex_lock;
    self->unlock = mutex_unlock;
    self->status = mutex_status;
}

void mutex_lock(Mutex_t self){
    atomic_int flag = atomic_exchange(&self->lockflag, 1);
    while(flag == 1) flag = atomic_exchange(&self->lockflag, 1);
    atomic_fetch_add(&self->count, 1);
    self->owner = 1;
}

void mutex_unlock(Mutex_t self){
    if(self->owner == 0) return;
    atomic_fetch_add(&self->count, -1);
    if(self->count == 0) atomic_exchange(&self->lockflag, 0);
    self->owner = 0;
}

int mutex_status(Mutex_t self){
    atomic_int flag = atomic_load(&self->lockflag);
    return flag == 1 ? 1 : 0;
}

void LockFunc(Mutex_t self, Func_t func, param_t param){
    mutex_lock(self);
    func(param);
    mutex_unlock(self);
}