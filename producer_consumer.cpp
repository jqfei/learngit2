#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class BoundedBuffer {
public:
    explicit BoundedBuffer(std::size_t capacity) : capacity_(capacity) {}

    void push(int value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return queue_.size() < capacity_; });

        queue_.push(value);
        std::cout << "Produced: " << value << '\n';

        lock.unlock();
        not_empty_.notify_one();
    }

    // pop
    int pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return !queue_.empty(); });

        int value = queue_.front();
        queue_.pop();
        std::cout << "Consumed: " << value << '\n';

        lock.unlock();
        not_full_.notify_one();
        return value;
    }

private:
    std::size_t capacity_;
    std::queue<int> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
};

int main() {
    BoundedBuffer buffer(5);
    const int item_count = 10;

    std::thread producer([&buffer, item_count] {
        for (int i = 1; i <= item_count; ++i) {
            buffer.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::thread consumer([&buffer, item_count] {
        for (int i = 0; i < item_count; ++i) {
            buffer.pop();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    });

    producer.join();
    consumer.join();

    std::cout << "Producer and consumer finished.\n";
    return 0;
}

std::lock_guard<std::mutex> lock()


class Observer {
public:
    virtual void update() = 0;
};

class Subject {
    std::vector<Observer *> observers_;
public:
    void attach(Observer *o) {
        observers_.push_back(o);
    }

    void detach(Observer *o) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), o), observers_.end());
    }

    void notify() {
        for (auto *o : observers_) {
            o->update();
        }
    }
};