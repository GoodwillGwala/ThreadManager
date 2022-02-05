#include "ThreadManager.h"
#include <thread>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>


class ThreadManager::impl
{

    public:
		// the main function for initializing the pool and starting the threads
		void Init(size_t threadCount);
		void InitPriorityQueue();
        void InitWorkers(size_t threadCount);
		void Shutdown();
		void AddJob(std::function<void()>&& job, Priority priority);

	private:
		bool m_running = true;

        void SynchronizationPoint();
        std::function<void()> GetJob();
        void ExecuteJob(std::function<void()> job);

		std::mutex m_guard;
		std::condition_variable m_cv;

		std::vector<std::thread> m_workers;

		std::map<Priority, std::queue<std::function<void()>>, std::greater<Priority> > m_jobs;
};
