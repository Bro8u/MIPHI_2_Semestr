#include "task.h"
// Проверка, что данное число присутствует в множестве простых чисел
PrimeNumbersSet::PrimeNumbersSet() {
    nanoseconds_under_mutex_ = 0;
    nanoseconds_waiting_mutex_ = 0;
}

bool PrimeNumbersSet::IsPrime(uint64_t number) const {
    std::shared_lock<std::shared_mutex> lock(set_mutex_);
    if (primes_.find(number) != primes_.end()) {
        return true;
    }
    return false;
}

// Получить следующее по величине простое число из множества
uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const {
    std::shared_lock<std::shared_mutex> lock(set_mutex_);
    auto it = primes_.find(number); 
    auto next = std::next(it);
    if (next == primes_.end()) {
        throw std::invalid_argument("limit");
    }
    return *next;
}

bool PrimeNumbersSet::Prime(uint64_t number) const {
    if (number == 0 || number == 1) {
        return false;
    }
    int k = 2;
    int flag = 1;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_dist(1, 2); // Adjust range as needed
    int size = size_dist(gen);
    for (size_t i = 0; i < size; ++i) {
        flag++;
        flag %= 2;
    }

    // for (int i = 0; i < 1000; ++i) {
    //     flag++;
    //     flag %= 2;
    // }
    while (k * k <= number) {
        if (number % k == 0) {
            return false;
        }
        ++k;
    }
    return true;
}
// likelyhood
/*
    * Найти простые числа в диапазоне [from, to) и добавить в множество
    * Во время работы этой функции нужно вести учет времени, затраченного на ожидание лока мюьтекса,
    * а также времени, проведенного в секции кода под локом
    */
void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to) {

    for (auto v = from; v < to; ++v) {
        if (Prime(v)) {
            auto start = std::chrono::steady_clock::now(); // Начало измерения времени
            std::unique_lock<std::shared_mutex> lock(set_mutex_);
            auto end = std::chrono::steady_clock::now(); // Мьютекс захвачен, фиксируем время
            auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            nanoseconds_waiting_mutex_ += elapsed;
            primes_.insert(v);
            lock.unlock();
            auto endend = std::chrono::steady_clock::now(); // Мьютекс отпущен, фиксируем время
            auto elapsedelapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endend - end).count();
            nanoseconds_under_mutex_ += elapsedelapsed;
        }
    }
}
// extra to push
// Посчитать количество простых чисел в диапазоне [from, to)
size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const {
    std::shared_lock<std::shared_mutex> lock(set_mutex_);
    size_t cnt = 0;
    for (auto it = primes_.lower_bound(from); it != primes_.end() && *it < to; ++it) {
        ++cnt;
    }
    return cnt;
}

// Получить наибольшее простое число из множества
uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const {
    std::shared_lock<std::shared_mutex> lock(set_mutex_);
    if (primes_.size() == 0) {
        return 1;
    }
    return *(--primes_.end());
}

// Получить суммарное время, проведенное в ожидании лока мьютекса во время работы функции AddPrimesInRange
std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const {
    return std::chrono::nanoseconds(nanoseconds_waiting_mutex_.load());
}

// Получить суммарное время, проведенное в коде под локом во время работы функции AddPrimesInRange
std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const {
    return std::chrono::nanoseconds(nanoseconds_under_mutex_.load());
}

