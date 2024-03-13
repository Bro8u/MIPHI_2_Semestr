#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>

class TimeOut : public std::exception {
    const char* what() const noexcept override {
            return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T& data) {
        std::unique_lock<std::mutex> lock(mtx);
        put_cv.wait(lock, [this] {return empty;});
        value = data;
        empty = false;
        get_cv.notify_one();
        put_cv.wait(lock, [this] {return empty;}); // while weren't read
    }
    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)){
        std::unique_lock<std::mutex> lock(mtx);
        bool hasData = get_cv.wait_for(lock, timeout, [this] {return !empty;});
        if (!hasData) {
            throw TimeOut();
        }
        empty = true;
        put_cv.notify_one(); // show Put that we read his query
        put_cv.notify_one(); 
        return value;
    }
private:
    T value;
    bool empty = true;
    std::mutex mtx;
    std::condition_variable put_cv, get_cv;
};
