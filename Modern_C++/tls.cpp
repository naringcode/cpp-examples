#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

// https://en.cppreference.com/w/c/thread/thread_local
thread_local int tls_ThreadID = 0;

int kNumThread = 10;

void ThreadMain(int threadID)
{
    tls_ThreadID = threadID;

    while (true)
    {
        std::stringstream ss;

        ss << "Thread : " << tls_ThreadID << '\n';

        std::cout << ss.str();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    std::vector<std::thread> threads;

    for (int i = 0; i < kNumThread; i++)
    {
        int threadID = i + 1;

        threads.push_back(std::thread(ThreadMain, threadID));
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    return 0;
}
