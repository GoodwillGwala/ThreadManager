#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <future>
#include <functional>
#include <cstdint>
class ThreadManager
{

	public:

	    template<typename Function, typename... Args>
	    using JobReturnType = typename std::result_of<Function(Args...)>::type;

	    typedef std::function<void()> Job;
	    typedef  size_t  ThreadType;
	    typedef std::uint_fast32_t Type32;
	    typedef std::unique_lock<std::mutex> Lock;

        template<typename Function, typename... Args>
        using Future = typename std::future<JobReturnType<Function, Args...>>;
	    enum class Priority : ThreadType
	    {
		    Normal,
		    High,
		    Critical
	    };

	public:

		ThreadManager(ThreadType threadCount = std::thread::hardware_concurrency());

		// Defaulted default constructor:
		ThreadManager(ThreadManager&&) = default;
		// Defualt move constructor
		ThreadManager& operator=(ThreadManager&&) = default;

		~ThreadManager();
        // No copy constructor - Singleton Idiom
		ThreadManager(const ThreadManager&) = delete;
		ThreadManager& operator=(const ThreadManager&) = delete;

		/// Adds a job for a given priority level. Returns a future.
		template <typename Function, typename... Args>
		auto Schedule(Priority priority, Function&& function, Args&&... args)->std::future<JobReturnType<Function, Args...>>;

		template <typename Function, typename... Args>
		auto Schedule(Function&& function, Args&&... args)->std::future<JobReturnType<Function, Args...>>;
		// Provide loop parallelization
		template <typename T, typename F>
        void ParallelizeLoop(T first_index, T last_index, const F &loop, Type32 num_tasks = 0);

        auto ThreadCount()->Type32;

	private:

		class impl;
		void AddJob(std::function<void()> job, Priority priority);
        Type32 thread_count;
		std::unique_ptr<impl> m_impl;
};
#include "ThreadManager.inl"
#endif
