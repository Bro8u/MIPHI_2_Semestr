#pragma once
#include <atomic>

class Mutex {
public:
    Mutex() : locked(false) {}

    void lock() {
        while (locked.exchange(true, std::memory_order_acquire)) {}
    }

    void unlock() {
        locked.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> locked;
};

class SharedMutex {
public:
    SharedMutex(): write_lock(false), reader_count(0) {}
    void lock() {
        mtx.lock();
        while (reader_count.load()){}
        while (write_lock.exchange(true, std::memory_order_acquire)){}
        mtx.unlock();
    }

    void unlock() {
        write_lock.store(false, std::memory_order_release);
    }

    void lock_shared() {
        mtx.lock();
        reader_count.fetch_add(1);
        mtx.unlock();
    }

    void unlock_shared() {
        mtx.lock();
        reader_count.fetch_sub(1);
        mtx.unlock();
        
    }
private:
    std::atomic<bool> write_lock;
    std::atomic<int> reader_count;
    Mutex mtx;
};
