#include "rwlock.h"

RWLock::RWLock() : sem(1), wsem(1) {}

void RWLock::read_lock() {
    sem.wait();
    if (++readers == 1) {
        wsem.wait();
    }
    sem.post();
}

void RWLock::read_unlock() {
    sem.wait();
    if (--readers == 0) {
        wsem.post();
    }
    sem.post();
}

void RWLock::write_lock() {
    wsem.wait();
}

void RWLock::write_unlock() {
    wsem.post();
}

ReadLock::ReadLock(RWLock& rwlock) : rwlock(&rwlock) {
    rwlock.read_lock();
}

ReadLock::~ReadLock() {
    rwlock->read_unlock();
}

WriteLock::WriteLock(RWLock& rwlock) : rwlock(&rwlock) {
    rwlock.write_lock();
}

WriteLock::~WriteLock() {
    rwlock->write_unlock();
}
