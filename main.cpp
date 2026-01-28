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

int main(int argc, char* argv[]) {
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    
    std::vector<std::string> files;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" && i + 1 < argc) {
            numThreads = std::stoi(argv[i + 1]);
            i++;
        } else {
            files.push_back(arg);
        }
    }
    
    if (files.empty()) {
        for (int i = 1; i <= 3; ++i) {
            files.push_back("test_file_" + std::to_string(i) + ".txt");
        }
    }
    
    std::cout << "Starting parallel text processing with " 
              << numThreads << " threads..." << std::endl;
    std::cout << "Processing " << files.size() << " files..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    ThreadPool pool(numThreads, count_lines_in_file);
    
    for (const auto& file : files) {
        pool.submit(Task{file});
    }
    
    int totalLines = 0;
    int processedFiles = 0;
    
    while (processedFiles < files.size()) {
        auto resultOpt = pool.getResult();
        if (resultOpt) {
            totalLines += *resultOpt;
            processedFiles++;
            std::cout << "Processed file " << processedFiles 
                      << "/" << files.size() 
                      << ", lines: " << *resultOpt << std::endl;
        }
    }
    
    pool.waitAll();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    std::cout << "\n===== RESULTS =====" << std::endl;
    std::cout << "Total lines: " << totalLines << std::endl;
    std::cout << "Files processed: " << processedFiles << std::endl;
    std::cout << "Threads used: " << numThreads << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    
    return 0;
}