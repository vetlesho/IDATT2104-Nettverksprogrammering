#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

using namespace std;

// to compile
// g++ -std=c++17 -o workers_program workers.cpp -pthread
// to run
// ./workers_program


class Workers {
    public:
        explicit Workers(size_t numThreads) : stopFlag(false) {
            for (size_t i = 0; i < numThreads; ++i) {
                threads.emplace_back([this] { workerLoop(); });
            }
        }

        void start() {
            for (size_t i = 0; i < numThreads; i++)
            {
                threads.emplace_back([this] {workerLoop(); });
            }
        }

        void post(function<void()> task) {
            {
                lock_guard<mutex> lock(queueMutex);
                taskQueue.push(task);
            }
            cv.notify_one();
        }
    
        // adds task to the queue with a delay
        void post_timeout(function<void()> task, int seconds) {
            {
                lock_guard<mutex> lock(queueMutex);
                taskQueue.push([seconds, task] {
                    this_thread::sleep_for(chrono::seconds(seconds));
                    task();
                });
            }
            cv.notify_one(); // notifies next worker thread 
        }

        void stop() {
            {
                lock_guard<mutex> lock(queueMutex);
                stopFlag = true;
            }
            cv.notify_all(); // notify remainig threads to end
        }

        void join() {
            stop();
            for (auto &thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }
        

    private:
        size_t numThreads;
        vector<thread> threads; // Dynamic threads, needs multiple worker threads
        queue<function<void()>> taskQueue; // taskqueue for storing functions a task in a worker thread will do
        mutex queueMutex; // ensure one thread accesses the taskqueue at a time
        condition_variable cv; // condition variable, sleep until notified
        bool stopFlag;

        void workerLoop() {
            while (true) {
                function<void()> task;
                {
                    // Each thread safely chooses a task
                    unique_lock<mutex> lock(queueMutex);
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

    worker_threads.post_timeout([] {
        cout << "Task A\n"; 
    }, 3);
    worker_threads.post_timeout([] {
        cout << "Task B\n";
    }, 1);

    event_loop.post_timeout([] {
        cout << "Task C\n";
    }, 1);
    event_loop.post_timeout([] {
        cout << "Task D\n";
    }, 1);

    //this_thread::sleep_for(chrono::seconds(3));

    worker_threads.join(); // Calls join() on the worker threads
    event_loop.join();     // Calls join() on the event thread

    /*worker_threads.stop();
    worker_threads.join();

    event_loop.stop();
    event_loop.join();*/

    return 0;
}