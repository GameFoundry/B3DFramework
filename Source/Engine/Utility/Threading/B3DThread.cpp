//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Threading/B3DThread.h"

#include <algorithm>

// Platform-agnostic threading logic. The OS-specific Thread::Implementation and the platform-dependent
// members (affinity, naming, ids, core enumeration) live in the per-platform overlays under
// Platform/<Platform>/Source/Engine/Utility/Private/<Platform>/B3D<Platform>Thread.cpp.

using namespace b3d;

ThreadCoreMask::ThreadCoreMask(std::initializer_list<CPUCore> initializerList)
{
	mCores.reserve(initializerList.size());
	for (auto core : initializerList)
		mCores.Add(core);
}

ThreadCoreMask& ThreadCoreMask::Add(const ThreadCoreMask& other)
{
	UnorderedSet<CPUCore> set;
	for (auto core : mCores)
		set.emplace(core);

	for (auto core : other.mCores)
	{
		if (set.count(core) != 0)
			continue;

		mCores.Add(core);
	}

	std::sort(mCores.begin(), mCores.end());
	return *this;
}

ThreadCoreMask& ThreadCoreMask::Remove(const ThreadCoreMask& other)
{
	UnorderedSet<CPUCore> set;
	for (auto core : other.mCores)
		set.emplace(core);

	for (size_t coreIndex = 0; coreIndex < mCores.size(); coreIndex++)
	{
		if (set.count(mCores[coreIndex]) != 0)
		{
			mCores[coreIndex] = mCores.back();
			mCores.resize(mCores.size() - 1);
		}
	}

	std::sort(mCores.begin(), mCores.end());
	return *this;
}

ThreadCoreMask OneOfThreadAffinityPolicy::GetMaskForThread(u32 threadIndex) const
{
	const size_t availableCoreCount = mAvailableCores.GetCoreCount();
	if (availableCoreCount == 0)
		return mAvailableCores;

	return ThreadCoreMask({ mAvailableCores[threadIndex % mAvailableCores.GetCoreCount()] });
}

thread_local u32 Thread::CurrentId = 0;

Thread::Thread(Function<void()>&& workerFunction)
	: Thread(ThreadCoreMask::CreateAnyThreadMask(), std::move(workerFunction))
{
}
