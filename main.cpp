#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <fstream>
#include <chrono>
#include <functional>
#include "BlockingQueue.hpp"

int count_lines_in_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return 0;
    }
    
    int line_count = 0;
    std::string line;
    while (std::getline(file, line)) {
        line_count++;
    }
    
    file.close();
    return line_count;
}

struct Task {
    std::string filepath;
};

class ThreadPool {
private:
    BlockingQueue<Task> taskQueue;
    BlockingQueue<int> resultQueue;
    std::vector<std::thread> workers;
    std::atomic<bool> stopFlag{false};
    std::atomic<int> activeWorkers{0};
    std::function<int(const std::string&)> processFunc;
    
public:
    ThreadPool(size_t numThreads, 
               std::function<int(const std::string&)> func) 
        : processFunc(func) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this]() {
                activeWorkers++;
                while (true) {
                    auto taskOpt = taskQueue.pop();
                    if (!taskOpt) break;
                    
                    Task task = std::move(*taskOpt);
                    int result = processFunc(task.filepath);
                    resultQueue.push(result);
                }
                activeWorkers--;
            });
        }
    }
    
    void submit(Task task) {
        taskQueue.push(std::move(task));
    }
    
    std::optional<int> getResult() {
        return resultQueue.pop();
    }
    
    void waitAll() {
        taskQueue.stop();
        while (activeWorkers > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void stop() {
        taskQueue.stop();
    }
    
    ~ThreadPool() {
        stop();
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }
};