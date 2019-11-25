#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class RWLock {
public:
    RWLock();

    void read_lock();
    void read_unlock();
    void write_lock();
    void write_unlock();

private:
    boost::interprocess::interprocess_semaphore sem, wsem;
    uint64_t readers = 0;
};

class ReadLock {
public:
    ReadLock(RWLock& rwlock);
    ~ReadLock();

private:
    RWLock* rwlock;
};

class WriteLock {
public:
    WriteLock(RWLock& rwlock);
    ~WriteLock();

private:
    RWLock* rwlock;
};
