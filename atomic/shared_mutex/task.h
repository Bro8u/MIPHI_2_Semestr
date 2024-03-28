#include <atomic>
class SharedMutex {
public:
    SharedMutex(): write_lock(false), reader_count(0) {}
    
    void lock() {
        while (write_lock.exchange(true, std::memory_order_acquire)) {}
        while (reader_count.load() != 0) {} 
    }

    void unlock() {
        write_lock.store(false, std::memory_order_release);
    }

    void lock_shared() {
        while (true) {
            while (write_lock.load()) {} 
            reader_count.fetch_add(1);
            if (!write_lock.load()){
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
