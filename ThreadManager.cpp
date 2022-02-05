 #include "ThreadManager.hpp"

ThreadManager::ThreadManager(ThreadType threadCount)
: m_impl(std::make_unique<ThreadManager::impl>())
{
	m_impl->InitPriorityQueue();
    m_impl->InitWorkers(threadCount);
    m_impl->Init(threadCount);
    thread_count = threadCount;
}

ThreadManager::~ThreadManager()
{
    m_impl->Shutdown();

}

auto ThreadManager::ThreadCount()->Type32
{
    return thread_count;
}

void ThreadManager::impl::Shutdown()
{
	m_running = false;

	m_cv.notify_all();


	for (auto& worker : m_workers)
		if (worker.joinable())
			worker.join();

}


void ThreadManager::AddJob(Job job, Priority priority)
{
	m_impl->AddJob(std::move(job), priority);
}


void ThreadManager::impl::InitWorkers(ThreadType threadCount)
{
    m_workers.reserve(threadCount);
}

void ThreadManager::impl::InitPriorityQueue()
{
    m_jobs[Priority::Normal] = {};
	m_jobs[Priority::High] = {};
	m_jobs[Priority::Critical] = {};

}


void ThreadManager::impl::SynchronizationPoint()
{

    Lock  unlock(m_guard);
    m_cv.wait(unlock, [this]()
	{
		// stop processing jobs from the queue
		if (m_running == false)
			return true;


        // the chosen thread loops through all queues to find job
	    bool allQueuesEmpty = true;
		for (const auto& kvp : m_jobs)
			allQueuesEmpty &= kvp.second.empty();

		return !allQueuesEmpty;
	});

}


auto ThreadManager::impl::GetJob()->Job
{
    Job job;
    Lock  unlock(m_guard);
    for (auto& kvp : m_jobs)
	{
		auto& jobs = kvp.second;
		if (jobs.empty())
			continue;

		job = std::move(jobs.front());
		jobs.pop();
		break;
	}

	return job;
}


void ThreadManager::impl::ExecuteJob(Job job)
{

    if (job != nullptr)
	{
		job();
	}

}

void ThreadManager::impl::Init(ThreadType threadCount)
{

	for (int i = 0; i < threadCount; i++)
	{

		m_workers.push_back(std::thread([this]()
		{

		    while (m_running)
			{
				SynchronizationPoint();
                auto job = GetJob();
                ExecuteJob(job);

			}
		}));
    }
}



void ThreadManager::impl::AddJob(Job&& job, Priority priority)
{
	std::unique_lock<std::mutex> unlock(m_guard);

	m_jobs[priority].emplace(std::move(job));

	m_cv.notify_one();
}
