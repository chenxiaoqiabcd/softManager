#pragma once

#include <cstdio>
typedef void* (pFnJobCallback)(void* arg);

class Job {
public:
	Job(pFnJobCallback* callback, void* data) : callback_rountine_(callback), data_(data) {

	}
private:
	pFnJobCallback* callback_rountine_;
	void* data_;
};

class ThreadPool {
public:
	ThreadPool(int max_thread_num);

	int AddJob(pFnJobCallback callback, void* arg);
private:
	int max_thread_num_;

	//pthread_mutex_t queue_mutex_;
};