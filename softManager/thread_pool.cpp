#include "thread_pool.h"

ThreadPool::ThreadPool(int max_thread_num) : max_thread_num_{ max_thread_num } {

}

int ThreadPool::AddJob(pFnJobCallback callback, void* arg) {
	auto job = new Job(callback, arg);

	//pthread_mutex_lock();
	return 0;
}