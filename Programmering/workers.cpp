#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

// to compile
// g++ -std=c++17 -o workers_program workers.cpp -pthread
// to run
// ./workers_program

class Workers {
    public:
        explicit Workers(size_t numThreads) : stopFlag(false){
            this->numThreads = numThreads;
        }

        void start() {
            for (size_t i = 0; i < numThreads; i++)
            {
                threads.emplace_back([this] {workerLoop(); });
            }
        }

        void post(std::function<void()> task) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                taskQueue.push(std::move(task));
            }
            cv.notify_one();
        }

        void stop() {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stopFlag = true;
            }
            cv.notify_all(); // notify remainig threads to end
        }

        void join() {
            for (auto &thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }




    private:
        size_t numThreads;
        std::vector<std::thread> threads; // Dynamic threads, needs multiple worker threads
        std::queue<std::function<void()>> taskQueue; // taskqueue for storing functions a task in a worker thread will do
        std::mutex queueMutex; // ensure one thread accesses the taskqueue at a time
        std::condition_variable cv; // condition variable, sleep until notified
        bool stopFlag;

        void workerLoop() {
            while (true) {
                std::function<void()> task;
                {
                    // Each thread safely chooses a task
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [this] { return stopFlag || !taskQueue.empty(); });

                    if (stopFlag && taskQueue.empty()) return;

                    task = std::move(taskQueue.front());
                    taskQueue.pop();
                }
                task(); // the thread run the selected task
            }
        }
};

int main() {
    Workers worker_threads(4); // 4 threads
    Workers event_loop(1);     // 1 thread

    worker_threads.start(); // create 4 internal thread
    event_loop.start();     // create 1 internal thread

    worker_threads.post([] {
        std::cout << "Task A\n"; 
    });
    worker_threads.post([] {
        std::cout << "Task B\n";
    });

    event_loop.post([] {
        std::cout << "Task C\n";
    });
    event_loop.post([] {
        std::cout << "Task D\n";
    });

    worker_threads.stop();
    worker_threads.join();

    event_loop.stop();
    event_loop.join();

    //worker_threads.join(); // Calls join() on the worker threads
    //event_loop.join();     // Calls join() on the event thread

    return 0;
}