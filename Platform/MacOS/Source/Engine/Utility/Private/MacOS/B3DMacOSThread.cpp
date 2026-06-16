//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include <pthread.h>
#include <unistd.h>
#include <cstdarg>
#include <cstdint>

#include "Threading/B3DThread.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

ThreadCoreMask ThreadCoreMask::CreateAnyThreadMask()
{
	ThreadCoreMask output;

	// macOS exposes no API to pin a thread to a specific core (affinity is only an advisory hint),
	// so the "any" mask simply enumerates every logical core.
	const long coreCount = sysconf(_SC_NPROCESSORS_ONLN);
	for (long coreIndex = 0; coreIndex < coreCount; coreIndex++)
	{
		CPUCore core;
		core.Pthread.Index = (u16)coreIndex;
		output.mCores.Add(core);
	}

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

	static void* Run(void* arg)
	{
		Implementation* implementation = static_cast<Implementation*>(arg);

		uint64_t threadId = 0;
		pthread_threadid_np(pthread_self(), &threadId);
		Thread::CurrentId = (u32)threadId;

		// macOS thread affinity is advisory-only and not enforceable, so it is intentionally not set here.
		implementation->WorkerFunction();
		return nullptr;
	}

	ThreadCoreMask Affinity;
	Function<void()> WorkerFunction;
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
	if (!m || !m->Created)
		return 0;

	uint64_t threadId = 0;
	pthread_threadid_np(m->ThreadHandle, &threadId);
	return (u32)threadId;
}

void Thread::SetName(const char* format, ...)
{
	char name[1024];
	va_list vararg;
	va_start(vararg, format);
	vsnprintf(name, sizeof(name), format, vararg);
	va_end(vararg);

	// macOS can only name the calling thread.
	pthread_setname_np(name);
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
