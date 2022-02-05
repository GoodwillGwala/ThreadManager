

template <typename Function, typename... Args>
auto ThreadManager::Schedule(Priority priority, Function&& function, Args&&... args)
->std::future<JobReturnType<Function, Args...>>
{
	auto job = std::make_shared<std::packaged_task<JobReturnType<Function, Args...>()>>
    (
        std::bind(std::forward<Function>(function), std::forward<Args>(args)...)
    );

	AddJob( [job]{ (*job)();}, priority );
	return job->get_future();
}

template <typename Function, typename... Args>
auto ThreadManager::Schedule(Function&& function, Args&&... args)->std::future<JobReturnType<Function, Args...>>
{
	return Schedule(Priority::Normal, std::forward<Function>(function), std::forward<Args>(args)...);
}


template <typename T, typename F>
void ThreadManager::ParallelizeLoop(T first_index, T last_index, const F &loop, Type32 num_tasks)
{
    if (num_tasks == 0)
        num_tasks = thread_count;

    if (last_index < first_index)
        std::swap(last_index, first_index);

    size_t total_size = last_index - first_index + 1;
    size_t block_size = total_size / num_tasks;

    if (block_size == 0)
    {
        block_size = 1;
        num_tasks = std::max((Type32)1, (Type32)total_size);
    }

    std::atomic<Type32> blocks_running = 0;

   for (Type32 t = 0; t < num_tasks; t++)
   {

        T start = (T)(t * block_size + first_index);
        T end = (t == num_tasks - 1) ? last_index : (T)((t + 1) * block_size + first_index - 1);

        blocks_running++;

        Schedule([&start, &end, &loop, &blocks_running]
        {
            for (T i = start; i <= end; i++)
                loop(i);
                blocks_running--;
        });

        while (blocks_running != 0)
            std::this_thread::yield();

    }
}
