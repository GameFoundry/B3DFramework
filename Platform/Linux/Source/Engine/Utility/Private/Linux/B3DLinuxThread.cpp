//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef _GNU_SOURCE
#define _GNU_SOURCE  // Required for cpu_set_t, CPU_SET and pthread_*affinity_np
#endif

#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <atomic>
#include <cstdarg>

#include "Threading/B3DThread.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

ThreadCoreMask ThreadCoreMask::CreateAnyThreadMask()
{
	ThreadCoreMask output;

	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);
	if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet) == 0)
	{
		// CPU_COUNT only gives the number of set bits; the permitted cores may be non-contiguous
		// (cpusets/cgroups/taskset), so test membership of each candidate index explicitly.
		for (int coreIndex = 0; coreIndex < CPU_SETSIZE; coreIndex++)
		{
			if (!CPU_ISSET(coreIndex, &cpuSet))
				continue;

			CPUCore core;
			core.Pthread.Index = (u16)coreIndex;
			output.mCores.Add(core);
		}
	}
	else
		B3D_LOG(Error, LogGeneric, "pthread_getaffinity_np() failed; returning an empty core mask.");

	return output;
}

ThreadCoreMask AnyOfThreadAffinityPolicy::GetMaskForThread(u32 threadIndex) const
{
	return mAvailableCores;
}

class Thread::Implementation
{
public:
	Implementation(const ThreadCoreMask& affinity, Function<void()>&& workerFunction)
		: Affinity(affinity)
		, WorkerFunction(std::move(workerFunction))
	{
		const int result = pthread_create(&ThreadHandle, nullptr, &Implementation::Run, this);
		if (result != 0)
			B3D_LOG(Error, LogGeneric, "pthread_create() failed with error {0}.", result);
		else
			Created = true;
	}

	Implementation(const Implementation&) = delete;
	Implementation(Implementation&&) = delete;
	Implementation& operator=(const Implementation&) = delete;
	Implementation& operator=(Implementation&&) = delete;

	void Join()
	{
		if (Created && !Joined)
		{
			pthread_join(ThreadHandle, nullptr);
			Joined = true;
		}
	}

	void SetAffinity()
	{
		const size_t coreCount = Affinity.GetCoreCount();
		if (coreCount == 0)
			return;

		cpu_set_t cpuSet;
		CPU_ZERO(&cpuSet);

		for (size_t coreIndex = 0; coreIndex < coreCount; coreIndex++)
			CPU_SET(Affinity[coreIndex].Pthread.Index, &cpuSet);

		pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet);
	}

	static void* Run(void* arg)
	{
		Implementation* implementation = static_cast<Implementation*>(arg);

		const u32 threadId = (u32)syscall(SYS_gettid);
		Thread::CurrentId = threadId;
		implementation->KernelThreadId.store(threadId, std::memory_order_release);

		implementation->SetAffinity();
		implementation->WorkerFunction();
		return nullptr;
	}

	ThreadCoreMask Affinity;
	Function<void()> WorkerFunction;
	std::atomic<u32> KernelThreadId{ 0 };
	pthread_t ThreadHandle{};
	bool Created = false;
	bool Joined = false;
};

Thread::Thread(const ThreadCoreMask& affinity, Function<void()>&& workerFunction)
{
	m = B3DNew<Implementation>(affinity, std::move(workerFunction));
}

Thread::~Thread()
{
	if (m != nullptr)
	{
		m->Join();
		B3DDelete(m);
	}
}

void Thread::WaitUntilComplete()
{
	if (!m)
		return;

	m->Join();
}

u32 Thread::GetId() const
{
	if (!m)
		return 0;

	return m->KernelThreadId.load(std::memory_order_acquire);
}

void Thread::SetName(const char* format, ...)
{
	char name[1024];
	va_list vararg;
	va_start(vararg, format);
	vsnprintf(name, sizeof(name), format, vararg);
	va_end(vararg);

	pthread_setname_np(pthread_self(), name);
}

u32 Thread::GetLogicalCoreCount()
{
	const long coreCount = sysconf(_SC_NPROCESSORS_ONLN);
	return coreCount > 0 ? (u32)coreCount : 0;
}

Thread::Thread(Thread&& rhs) : m(rhs.m)
{
	rhs.m = nullptr;
}

Thread& Thread::operator=(Thread&& rhs)
{
	if (this == &rhs)
		return *this;

	if (m)
	{
		m->Join();
		B3DDelete(m);
		m = nullptr;
	}

	m = rhs.m;

	rhs.m = nullptr;
	return *this;
}
