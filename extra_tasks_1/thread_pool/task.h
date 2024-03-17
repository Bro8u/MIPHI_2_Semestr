#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>

/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * 
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */


class ThreadPool {
public:
    
           
    ThreadPool(size_t threadCount) : active(true) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this] {
                WorkerThread();
            });
        }
    }

    ~ThreadPool() {
        if (active.load()) {
            Terminate(true);
        }
    }

    void PushTask(const std::function<void()>& task) {
        if (!active.load()) {
            throw std::runtime_error("enough");
        }
        {
            std::lock_guard<std::mutex> lock(qMutex);
            tasks.emplace(task);
        }
        condition.notify_one();
    }

    void Terminate(bool wait) {
        {
            std::lock_guard<std::mutex> lock(qMutex);
            if (!wait) {
                while (!tasks.empty()) {
                    tasks.pop();
                }
            }
            active = false;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            worker.join();
        }
        
    }

    bool IsActive() const {
        return active.load();
    }

    size_t QueueSize() const {
        std::lock_guard<std::mutex> lock(qMutex);
        return tasks.size();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    mutable std::mutex qMutex;
    std::condition_variable condition;
    std::atomic<bool> active;

    void WorkerThread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(qMutex);
                condition.wait(lock, [this] {return !active.load() || !tasks.empty();});
                if (!active.load() && tasks.empty()) {
                    return;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }
};




