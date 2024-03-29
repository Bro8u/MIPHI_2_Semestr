#include <atomic>
class SharedMutex {
public:
    SharedMutex(): write_lock(false), reader_count(0) {}
    void lock() {
        while (write_lock.exchange(true)) {}
        while (reader_count.load() != 0) {} 
    }
    void unlock() {
        write_lock.store(false);
    }
    void lock_shared() {
        while (true) {
            while (write_lock.load()) {} 
            reader_count.fetch_add(1);
            if (write_lock.load() == false){
                break; 
            }
            reader_count.fetch_sub(1);
        }
    }
    void unlock_shared() {
        reader_count.fetch_sub(1);
    }

private:
    std::atomic<bool> write_lock;
    std::atomic<int> reader_count;
};

// #pragma once
// #include <atomic>
// class SharedMutex {
//     std::atomic<int> lock_count;
// public:
//     SharedMutex()
//     : lock_count(0)
//     {}
//     void lock() {
//         int expected = 0;
//         while (!lock_count.compare_exchange_weak(expected, -1)) {
//             expected = 0;
//         }
//         lock_count.store(-1);
//     }
//     void unlock() {
//         lock_count.store(0);
//     }
//     void lock_shared() {
//         if (lock_count.load() >= 0) {
//             lock_count.fetch_add(1);
//             return;
//         }
//         int expected = 0;
//         while (!lock_count.compare_exchange_weak(expected, 0)) {
//             expected = 0;
//         }
//         lock_count.store(1);
//     }
//     void unlock_shared() {
//         lock_count.fetch_sub(1);
//     }
// };