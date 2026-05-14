#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

// Represents a raw video frame
struct VideoFrame {
    int id;
    std::vector<uint8_t> pixelData; 
    // In production, use a pre-allocated pool for pixelData
};

class FrameBuffer {
private:
    std::vector<std::unique_ptr<VideoFrame>> buffer;
    size_t head = 0;
    size_t tail = 0;
    size_t maxSize;
    size_t currentSize = 0;

    std::mutex mtx;
    std::condition_variable notFull;
    std::condition_variable notEmpty;

public:
    explicit FrameBuffer(size_t size) : buffer(size), maxSize(size) {}

    // Producer calls this
    void push(std::unique_ptr<VideoFrame> frame) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Handle "Fast Producer": Wait until there is space
        notFull.wait(lock, [this]() { return currentSize < maxSize; });

        buffer[tail] = std::move(frame);
        tail = (tail + 1) % maxSize;
        currentSize++;

        // Signal "Slow Consumer" that data is ready
        notEmpty.notify_one();
    }

    // Consumer calls this
    std::unique_ptr<VideoFrame> pop() {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until there is data
        notEmpty.wait(lock, [this]() { return currentSize > 0; });

        auto frame = std::move(buffer[head]);
        head = (head + 1) % maxSize;
        currentSize--;

        // Signal Producer that a slot is free
        notFull.notify_one();
        return frame;
    }
};

// decoderTask
void decoderTask(FrameBuffer& fb) {
    for (int i = 0; i < 100; ++i) {
        auto frame = std::make_unique<VideoFrame>();
        frame->id = i;
        fb.push(std::move(frame));
        std::cout << "Decoded frame: " << i << std::endl;
    }
}

void encoderTask(FrameBuffer& fb) {
    while (true) {
        auto frame = fb.pop();
        std::cout << "Encoded frame: " << frame->id << std::endl;
        // Simulate heavy encoding work
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    }
}

int main() {
    FrameBuffer sharedBuffer(10); // Buffer holds 10 frames
    std::thread producer(decoderTask, std::ref(sharedBuffer));
    std::thread consumer(encoderTask, std::ref(sharedBuffer));

    producer.join();
    consumer.join();
    return 0;
}