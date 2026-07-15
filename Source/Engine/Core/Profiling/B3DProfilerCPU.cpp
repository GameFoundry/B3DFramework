//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/B3DProfilerCPU.h"
#include "Debug/B3DDebug.h"
#include "Platform/B3DPlatform.h"
#include <chrono>

#if B3D_COMPILER_MSVC
#	include <intrin.h>
#endif

#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32
#	if B3D_COMPILER_GCC || B3D_COMPILER_CLANG
#		include "B3DCpuid.h"
#	endif

#	if B3D_COMPILER_CLANG
#		if B3D_PLATFORM_WIN32
#			include "intrin.h"
#		else
#			include <x86intrin.h>
#		endif
#	endif
#endif

using namespace std::chrono;

using namespace b3d;

ProfilerCPU::Timer::Timer()
{
	Time = 0.0f;
}

void ProfilerCPU::Timer::Start()
{
	startTime = GetCurrentTime();
}

void ProfilerCPU::Timer::Stop()
{
	Time += GetCurrentTime() - startTime;
}

void ProfilerCPU::Timer::Reset()
{
	Time = 0.0f;
}

inline double ProfilerCPU::Timer::GetCurrentTime() const
{
	high_resolution_clock::time_point timeNow = mHRClock.now();
	nanoseconds timeNowNs = timeNow.time_since_epoch();

	return timeNowNs.count() * 0.000001;
}

ProfilerCPU::TimerPrecise::TimerPrecise()
{
	Cycles = 0;
}

void ProfilerCPU::TimerPrecise::Start()
{
	startCycles = GetNumCycles();
}

void ProfilerCPU::TimerPrecise::Stop()
{
	Cycles += GetNumCycles() - startCycles;
}

void ProfilerCPU::TimerPrecise::Reset()
{
	Cycles = 0;
}

inline u64 ProfilerCPU::TimerPrecise::GetNumCycles()
{
#if (B3D_COMPILER_GCC || B3D_COMPILER_CLANG) && B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_ARM64
	// Virtual counter register: constant-frequency, user-space readable (unlike the cycle counter PMCCNTR_EL0,
	// which traps unless the kernel enables user access).
	u64 x;
	__asm__ __volatile__("mrs %0, cntvct_el0"
						 : "=r"(x));
	return x;
#elif B3D_COMPILER_GCC || B3D_COMPILER_CLANG
	unsigned int a = 0;
	unsigned int b[4];
	__get_cpuid(a, &b[0], &b[1], &b[2], &b[3]);

#	if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
	u32 __a, __d;
	__asm__ __volatile__("rdtsc"
						 : "=a"(__a), "=d"(__d));
	return (u64(__a) | u64(__d) << 32);
#	else
	u64 x;
	__asm__ volatile(".byte 0x0f, 0x31"
					 : "=A"(x));
	return x;
#	endif
#elif B3D_COMPILER_MSVC
	int a[4];
	int b = 0;
	__cpuid(a, b);
	return __rdtsc();
#else
	static_assert(false, "Unsupported compiler");
#endif
}

ProfilerCPU::ProfileData::ProfileData(FrameAllocator* alloc)
	: Samples(alloc)
{}

void ProfilerCPU::ProfileData::BeginSample()
{
	MemAllocs = MemoryCounter::GetAllocationCount();
	MemFrees = MemoryCounter::GetFreeCount();

	Timer.Reset();
	Timer.Start();
}

void ProfilerCPU::ProfileData::EndSample()
{
	Timer.Stop();

	u64 numAllocs = MemoryCounter::GetAllocationCount() - MemAllocs;
	u64 numFrees = MemoryCounter::GetFreeCount() - MemFrees;

	Samples.push_back(ProfileSample(Timer.Time, numAllocs, numFrees));
}

void ProfilerCPU::ProfileData::ResumeLastSample()
{
	Timer.Start();
	Samples.erase(Samples.end() - 1);
}

ProfilerCPU::PreciseProfileData::PreciseProfileData(FrameAllocator* alloc)
	: Samples(alloc)
{}

void ProfilerCPU::PreciseProfileData::BeginSample()
{
	MemAllocs = MemoryCounter::GetAllocationCount();
	MemFrees = MemoryCounter::GetFreeCount();

	Timer.Reset();
	Timer.Start();
}

void ProfilerCPU::PreciseProfileData::EndSample()
{
	Timer.Stop();

	u64 numAllocs = MemoryCounter::GetAllocationCount() - MemAllocs;
	u64 numFrees = MemoryCounter::GetFreeCount() - MemFrees;

	Samples.push_back(PreciseProfileSample(Timer.Cycles, numAllocs, numFrees));
}

void ProfilerCPU::PreciseProfileData::ResumeLastSample()
{
	Timer.Start();
	Samples.erase(Samples.end() - 1);
}

B3D_THREADLOCAL ProfilerCPU::ThreadInfo* ProfilerCPU::ThreadInfo::activeThread = nullptr;

ProfilerCPU::ThreadInfo::ThreadInfo()
	: FrameAllocator(1024 * 512)
{
}

void ProfilerCPU::ThreadInfo::Begin(const char* _name)
{
	if(IsActive)
	{
		B3D_LOG(Warning, LogProfiler, "Profiler::beginThread called on a thread that was already being sampled");
		return;
	}

	if(RootBlock == nullptr)
		RootBlock = GetBlock(_name);

	ActiveBlock = ProfilerCPU::ActiveBlock(ActiveSamplingType::Basic, RootBlock);
	if(ActiveBlocks == nullptr)
		ActiveBlocks = FrameAllocator.Construct<Stack<ProfilerCPU::ActiveBlock, StdFrameAlloc<ProfilerCPU::ActiveBlock>>>(StdFrameAlloc<ProfilerCPU::ActiveBlock>(&FrameAllocator));

	ActiveBlocks->push(ActiveBlock);

	RootBlock->Basic.BeginSample();
	IsActive = true;
}

void ProfilerCPU::ThreadInfo::End()
{
	if(ActiveBlock.Type == ActiveSamplingType::Basic)
		ActiveBlock.Block->Basic.EndSample();
	else
		ActiveBlock.Block->Precise.EndSample();

	ActiveBlocks->pop();

	if(!IsActive)
		B3D_LOG(Warning, LogProfiler, "Profiler::endThread called on a thread that isn't being sampled.");

	if(ActiveBlocks->size() > 0)
	{
		B3D_LOG(Warning, LogProfiler, "Profiler::endThread called but not all sample pairs were closed. "
								  "Sampling data will not be valid.");

		while(ActiveBlocks->size() > 0)
		{
			ProfilerCPU::ActiveBlock& curBlock = ActiveBlocks->top();
			if(curBlock.Type == ActiveSamplingType::Basic)
				curBlock.Block->Basic.EndSample();
			else
				curBlock.Block->Precise.EndSample();

			ActiveBlocks->pop();
		}
	}

	IsActive = false;
	ActiveBlock = ProfilerCPU::ActiveBlock();

	FrameAllocator.Free(ActiveBlocks);
	ActiveBlocks = nullptr;
}

void ProfilerCPU::ThreadInfo::Reset()
{
	if(IsActive)
		End();

	if(RootBlock != nullptr)
		ReleaseBlock(RootBlock);

	RootBlock = nullptr;
	FrameAllocator.Clear(); // Note: This never actually frees memory
}

ProfilerCPU::ProfiledBlock* ProfilerCPU::ThreadInfo::GetBlock(const char* name)
{
	ProfiledBlock* block = FrameAllocator.Construct<ProfiledBlock>(&FrameAllocator);
	block->Name = (char*)FrameAllocator.Allocate(((u32)strlen(name) + 1) * sizeof(char));
	strcpy(block->Name, name);

	return block;
}

void ProfilerCPU::ThreadInfo::ReleaseBlock(ProfiledBlock* block)
{
	FrameAllocator.Free((u8*)block->Name);
	FrameAllocator.Free(block);
}

ProfilerCPU::ProfiledBlock::ProfiledBlock(FrameAllocator* alloc)
	: Basic(alloc), Precise(alloc), Children(alloc)
{}

ProfilerCPU::ProfiledBlock::~ProfiledBlock()
{
	ThreadInfo* thread = ThreadInfo::activeThread;

	for(auto& child : Children)
		thread->ReleaseBlock(child);

	Children.clear();
}

ProfilerCPU::ProfiledBlock* ProfilerCPU::ProfiledBlock::FindChild(const char* name) const
{
	for(auto& child : Children)
	{
		if(strcmp(child->Name, name) == 0)
			return child;
	}

	return nullptr;
}

ProfilerCPU::ProfilerCPU()
{
	// TODO - We only estimate overhead on program start. It might be better to estimate it each time beginThread is called,
	// and keep separate values per thread.
	EstimateTimerOverhead();
}

ProfilerCPU::~ProfilerCPU()
{
	Reset();

	Lock lock(mThreadSync);

	for(auto& threadInfo : mActiveThreads)
		B3DDelete<ThreadInfo, ProfilerAllocatorTag>(threadInfo);
}

void ProfilerCPU::BeginThread(const char* name)
{
	ThreadInfo* thread = ThreadInfo::activeThread;
	if(thread == nullptr)
	{
		ThreadInfo::activeThread = B3DNew<ThreadInfo, ProfilerAllocatorTag>();
		thread = ThreadInfo::activeThread;

		{
			Lock lock(mThreadSync);

			mActiveThreads.push_back(thread);
		}
	}

	thread->Begin(name);
}

void ProfilerCPU::EndThread()
{
	// I don't do a nullcheck where on purpose, so endSample can be called ASAP
	ThreadInfo::activeThread->End();
}

void ProfilerCPU::BeginSample(const char* name)
{
	ThreadInfo* thread = ThreadInfo::activeThread;
	if(thread == nullptr || !thread->IsActive)
	{
		BeginThread("Unknown");
		thread = ThreadInfo::activeThread;
	}

	ProfiledBlock* parent = thread->ActiveBlock.Block;
	ProfiledBlock* block = nullptr;

	if(parent != nullptr)
		block = parent->FindChild(name);

	if(block == nullptr)
	{
		block = thread->GetBlock(name);

		if(parent != nullptr)
			parent->Children.push_back(block);
		else
			thread->RootBlock->Children.push_back(block);
	}

	thread->ActiveBlock = ActiveBlock(ActiveSamplingType::Basic, block);
	thread->ActiveBlocks->push(thread->ActiveBlock);

	block->Basic.BeginSample();
}

void ProfilerCPU::EndSample(const char* name)
{
	ThreadInfo* thread = ThreadInfo::activeThread;
	ProfiledBlock* block = thread->ActiveBlock.Block;

#if B3D_DEBUG
	if(block == nullptr)
	{
		B3D_LOG(Warning, LogProfiler, "Mismatched CPUProfiler::endSample. No beginSample was called.");
		return;
	}

	if(thread->ActiveBlock.Type == ActiveSamplingType::Precise)
	{
		B3D_LOG(Warning, LogProfiler, "Mismatched CPUProfiler::endSample. Was expecting Profiler::endSamplePrecise.");
		return;
	}

	if(strcmp(block->Name, name) != 0)
	{
		B3D_LOG(Warning, LogProfiler, "Mismatched CPUProfiler::endSample. Was expecting \"{0}\" but got \"{1}\". "
								  "Sampling data will not be valid.",
			   block->Name, name);
		return;
	}
#endif

	block->Basic.EndSample();

	thread->ActiveBlocks->pop();

	if(!thread->ActiveBlocks->empty())
		thread->ActiveBlock = thread->ActiveBlocks->top();
	else
		thread->ActiveBlock = ActiveBlock();
}

void ProfilerCPU::BeginSamplePrecise(const char* name)
{
	// Note: There is a (small) possibility a context switch will happen during this measurement in which case result will be skewed.
	// Increasing thread priority might help. This is generally only a problem with code that executes a long time (10-15+ ms - depending on OS quant length)

	ThreadInfo* thread = ThreadInfo::activeThread;
	if(thread == nullptr || !thread->IsActive)
		BeginThread("Unknown");

	ProfiledBlock* parent = thread->ActiveBlock.Block;
	ProfiledBlock* block = nullptr;

	if(parent != nullptr)
		block = parent->FindChild(name);

	if(block == nullptr)
	{
		block = thread->GetBlock(name);

		if(parent != nullptr)
			parent->Children.push_back(block);
		else
			thread->RootBlock->Children.push_back(block);
	}

	thread->ActiveBlock = ActiveBlock(ActiveSamplingType::Precise, block);
	thread->ActiveBlocks->push(thread->ActiveBlock);

	block->Precise.BeginSample();
}

void ProfilerCPU::EndSamplePrecise(const char* name)
{
	ThreadInfo* thread = ThreadInfo::activeThread;
	ProfiledBlock* block = thread->ActiveBlock.Block;

#if B3D_DEBUG
	if(block == nullptr)
	{
		B3D_LOG(Warning, LogProfiler, "Mismatched Profiler::endSamplePrecise. No beginSamplePrecise was called.");
		return;
	}

	if(thread->ActiveBlock.Type == ActiveSamplingType::Basic)
	{
		B3D_LOG(Warning, LogProfiler, "Mismatched CPUProfiler::endSamplePrecise. Was expecting Profiler::endSample.");
		return;
	}

	if(strcmp(block->Name, name) != 0)
	{
		B3D_LOG(Warning, LogProfiler, "Mismatched Profiler::endSamplePrecise. Was expecting \"{0}\" but got \"{1}\". "
								  "Sampling data will not be valid.",
			   block->Name, name);
		return;
	}
#endif

	block->Precise.EndSample();

	thread->ActiveBlocks->pop();

	if(!thread->ActiveBlocks->empty())
		thread->ActiveBlock = thread->ActiveBlocks->top();
	else
		thread->ActiveBlock = ActiveBlock();
}

void ProfilerCPU::Reset()
{
	ThreadInfo* thread = ThreadInfo::activeThread;

	if(thread != nullptr)
		thread->Reset();
}

CPUProfilerReport ProfilerCPU::GenerateReport()
{
	CPUProfilerReport report;

	ThreadInfo* thread = ThreadInfo::activeThread;
	if(thread == nullptr)
		return report;

	if(thread->IsActive)
		thread->End();

	// We need to separate out basic and precise data and form two separate hierarchies
	if(thread->RootBlock == nullptr)
		return report;

	struct TempEntry
	{
		TempEntry(ProfiledBlock* _parentBlock, u32 _entryIdx)
			: ParentBlock(_parentBlock), EntryIdx(_entryIdx)
		{}

		ProfiledBlock* ParentBlock;
		u32 EntryIdx;
		ProfilerVector<u32> ChildIndexes;
	};

	ProfilerVector<CPUProfilerBasicSamplingEntry> basicEntries;
	ProfilerVector<CPUProfilerPreciseSamplingEntry> preciseEntries;

	// Fill up flatHierarchy array in a way so we always process children before parents
	ProfilerStack<u32> todo;
	ProfilerVector<TempEntry> flatHierarchy;

	u32 entryIdx = 0;
	todo.push(entryIdx);
	flatHierarchy.push_back(TempEntry(thread->RootBlock, entryIdx));

	entryIdx++;
	while(!todo.empty())
	{
		u32 curDataIdx = todo.top();
		ProfiledBlock* curBlock = flatHierarchy[curDataIdx].ParentBlock;

		todo.pop();

		for(auto& child : curBlock->Children)
		{
			flatHierarchy[curDataIdx].ChildIndexes.push_back(entryIdx);

			todo.push(entryIdx);
			flatHierarchy.push_back(TempEntry(child, entryIdx));

			entryIdx++;
		}
	}

	// Calculate sampling data for all entries
	basicEntries.resize(flatHierarchy.size());
	preciseEntries.resize(flatHierarchy.size());

	for(auto iter = flatHierarchy.rbegin(); iter != flatHierarchy.rend(); ++iter)
	{
		TempEntry& curData = *iter;
		ProfiledBlock* curBlock = curData.ParentBlock;

		CPUProfilerBasicSamplingEntry* entryBasic = &basicEntries[curData.EntryIdx];
		CPUProfilerPreciseSamplingEntry* entryPrecise = &preciseEntries[curData.EntryIdx];

		// Calculate basic data
		entryBasic->Data.Name = String(curBlock->Name);

		entryBasic->Data.MemAllocs = 0;
		entryBasic->Data.MemFrees = 0;
		entryBasic->Data.TotalTimeMs = 0.0;
		entryBasic->Data.MaxTimeMs = 0.0;
		for(auto& sample : curBlock->Basic.Samples)
		{
			entryBasic->Data.TotalTimeMs += sample.Time;
			entryBasic->Data.MaxTimeMs = std::max(entryBasic->Data.MaxTimeMs, sample.Time);
			entryBasic->Data.MemAllocs += sample.NumAllocs;
			entryBasic->Data.MemFrees += sample.NumFrees;
		}

		entryBasic->Data.NumCalls = (u32)curBlock->Basic.Samples.size();

		if(entryBasic->Data.NumCalls > 0)
			entryBasic->Data.AvgTimeMs = entryBasic->Data.TotalTimeMs / entryBasic->Data.NumCalls;

		double totalChildTime = 0.0;
		for(auto& childIdx : curData.ChildIndexes)
		{
			CPUProfilerBasicSamplingEntry* childEntry = &basicEntries[childIdx];
			totalChildTime += childEntry->Data.TotalTimeMs;
			childEntry->Data.PctOfParent = (float)(childEntry->Data.TotalTimeMs / entryBasic->Data.TotalTimeMs);

			entryBasic->Data.EstimatedOverheadMs += childEntry->Data.EstimatedOverheadMs;
		}

		entryBasic->Data.EstimatedOverheadMs += curBlock->Basic.Samples.size() * mBasicSamplingOverheadMs;
		entryBasic->Data.EstimatedOverheadMs += curBlock->Precise.Samples.size() * mPreciseSamplingOverheadMs;

		entryBasic->Data.TotalSelfTimeMs = entryBasic->Data.TotalTimeMs - totalChildTime;

		if(entryBasic->Data.NumCalls > 0)
			entryBasic->Data.AvgSelfTimeMs = entryBasic->Data.TotalSelfTimeMs / entryBasic->Data.NumCalls;

		entryBasic->Data.EstimatedSelfOverheadMs = mBasicTimerOverhead;

		// Calculate precise data
		entryPrecise->Data.Name = String(curBlock->Name);

		entryPrecise->Data.MemAllocs = 0;
		entryPrecise->Data.MemFrees = 0;
		entryPrecise->Data.TotalCycles = 0;
		entryPrecise->Data.MaxCycles = 0;
		for(auto& sample : curBlock->Precise.Samples)
		{
			entryPrecise->Data.TotalCycles += sample.Cycles;
			entryPrecise->Data.MaxCycles = std::max(entryPrecise->Data.MaxCycles, sample.Cycles);
			entryPrecise->Data.MemAllocs += sample.NumAllocs;
			entryPrecise->Data.MemFrees += sample.NumFrees;
		}

		entryPrecise->Data.NumCalls = (u32)curBlock->Precise.Samples.size();

		if(entryPrecise->Data.NumCalls > 0)
			entryPrecise->Data.AvgCycles = entryPrecise->Data.TotalCycles / entryPrecise->Data.NumCalls;

		u64 totalChildCycles = 0;
		for(auto& childIdx : curData.ChildIndexes)
		{
			CPUProfilerPreciseSamplingEntry* childEntry = &preciseEntries[childIdx];
			totalChildCycles += childEntry->Data.TotalCycles;
			childEntry->Data.PctOfParent = childEntry->Data.TotalCycles / (float)entryPrecise->Data.TotalCycles;

			entryPrecise->Data.EstimatedOverhead += childEntry->Data.EstimatedOverhead;
		}

		entryPrecise->Data.EstimatedOverhead += curBlock->Precise.Samples.size() * mPreciseSamplingOverheadCycles;
		entryPrecise->Data.EstimatedOverhead += curBlock->Basic.Samples.size() * mBasicSamplingOverheadCycles;

		entryPrecise->Data.TotalSelfCycles = entryPrecise->Data.TotalCycles - totalChildCycles;

		if(entryPrecise->Data.NumCalls > 0)
			entryPrecise->Data.AvgSelfCycles = entryPrecise->Data.TotalSelfCycles / entryPrecise->Data.NumCalls;

		entryPrecise->Data.EstimatedSelfOverhead = mPreciseTimerOverhead;
	}

	// Prune empty basic entries
	ProfilerStack<u32> finalBasicHierarchyTodo;
	ProfilerStack<u32> parentBasicEntryIndexes;
	ProfilerVector<TempEntry> newBasicEntries;

	finalBasicHierarchyTodo.push(0);

	entryIdx = 0;
	parentBasicEntryIndexes.push(entryIdx);
	newBasicEntries.push_back(TempEntry(nullptr, entryIdx));

	entryIdx++;

	while(!finalBasicHierarchyTodo.empty())
	{
		u32 parentEntryIdx = parentBasicEntryIndexes.top();
		parentBasicEntryIndexes.pop();

		u32 curEntryIdx = finalBasicHierarchyTodo.top();
		TempEntry& curEntry = flatHierarchy[curEntryIdx];
		finalBasicHierarchyTodo.pop();

		for(auto& childIdx : curEntry.ChildIndexes)
		{
			finalBasicHierarchyTodo.push(childIdx);

			CPUProfilerBasicSamplingEntry& basicEntry = basicEntries[childIdx];
			if(basicEntry.Data.NumCalls > 0)
			{
				newBasicEntries.push_back(TempEntry(nullptr, childIdx));
				newBasicEntries[parentEntryIdx].ChildIndexes.push_back(entryIdx);

				parentBasicEntryIndexes.push(entryIdx);

				entryIdx++;
			}
			else
				parentBasicEntryIndexes.push(parentEntryIdx);
		}
	}

	if(newBasicEntries.size() > 0)
	{
		ProfilerVector<CPUProfilerBasicSamplingEntry*> finalBasicEntries;

		report.mBasicSamplingRootEntry = basicEntries[newBasicEntries[0].EntryIdx];
		finalBasicEntries.push_back(&report.mBasicSamplingRootEntry);

		finalBasicHierarchyTodo.push(0);

		while(!finalBasicHierarchyTodo.empty())
		{
			u32 curEntryIdx = finalBasicHierarchyTodo.top();
			finalBasicHierarchyTodo.pop();

			TempEntry& curEntry = newBasicEntries[curEntryIdx];

			CPUProfilerBasicSamplingEntry* basicEntry = finalBasicEntries[curEntryIdx];

			basicEntry->ChildEntries.resize(curEntry.ChildIndexes.size());
			u32 idx = 0;

			for(auto& childIdx : curEntry.ChildIndexes)
			{
				TempEntry& childEntry = newBasicEntries[childIdx];
				basicEntry->ChildEntries[idx] = basicEntries[childEntry.EntryIdx];

				finalBasicEntries.push_back(&(basicEntry->ChildEntries[idx]));
				finalBasicHierarchyTodo.push(childIdx);
				idx++;
			}
		}
	}

	// Prune empty precise entries
	ProfilerStack<u32> finalPreciseHierarchyTodo;
	ProfilerStack<u32> parentPreciseEntryIndexes;
	ProfilerVector<TempEntry> newPreciseEntries;

	finalPreciseHierarchyTodo.push(0);

	entryIdx = 0;
	parentPreciseEntryIndexes.push(entryIdx);
	newPreciseEntries.push_back(TempEntry(nullptr, entryIdx));

	entryIdx++;

	while(!finalPreciseHierarchyTodo.empty())
	{
		u32 parentEntryIdx = parentPreciseEntryIndexes.top();
		parentPreciseEntryIndexes.pop();

		u32 curEntryIdx = finalPreciseHierarchyTodo.top();
		TempEntry& curEntry = flatHierarchy[curEntryIdx];
		finalPreciseHierarchyTodo.pop();

		for(auto& childIdx : curEntry.ChildIndexes)
		{
			finalPreciseHierarchyTodo.push(childIdx);

			CPUProfilerPreciseSamplingEntry& preciseEntry = preciseEntries[childIdx];
			if(preciseEntry.Data.NumCalls > 0)
			{
				newPreciseEntries.push_back(TempEntry(nullptr, childIdx));
				newPreciseEntries[parentEntryIdx].ChildIndexes.push_back(entryIdx);

				parentPreciseEntryIndexes.push(entryIdx);

				entryIdx++;
			}
			else
				parentPreciseEntryIndexes.push(parentEntryIdx);
		}
	}

	if(newPreciseEntries.size() > 0)
	{
		ProfilerVector<CPUProfilerPreciseSamplingEntry*> finalPreciseEntries;

		report.mPreciseSamplingRootEntry = preciseEntries[newPreciseEntries[0].EntryIdx];
		finalPreciseEntries.push_back(&report.mPreciseSamplingRootEntry);

		finalPreciseHierarchyTodo.push(0);

		while(!finalPreciseHierarchyTodo.empty())
		{
			u32 curEntryIdx = finalPreciseHierarchyTodo.top();
			finalPreciseHierarchyTodo.pop();

			TempEntry& curEntry = newPreciseEntries[curEntryIdx];

			CPUProfilerPreciseSamplingEntry* preciseEntry = finalPreciseEntries[curEntryIdx];

			preciseEntry->ChildEntries.resize(curEntry.ChildIndexes.size());
			u32 idx = 0;

			for(auto& childIdx : curEntry.ChildIndexes)
			{
				TempEntry& childEntry = newPreciseEntries[childIdx];
				preciseEntry->ChildEntries[idx] = preciseEntries[childEntry.EntryIdx];

				finalPreciseEntries.push_back(&preciseEntry->ChildEntries.back());
				finalPreciseHierarchyTodo.push(childIdx);
				idx++;
			}
		}
	}

	return report;
}

void ProfilerCPU::EstimateTimerOverhead()
{
	// Get an idea of how long timer calls and RDTSC takes
	const u32 reps = 1000, sampleReps = 20;

	mBasicTimerOverhead = 1000000.0;
	mPreciseTimerOverhead = 1000000;
	for(u32 tries = 0; tries < 20; tries++)
	{
		Timer timer;
		for(u32 i = 0; i < reps; i++)
		{
			timer.Start();
			timer.Stop();
		}

		double avgTime = double(timer.Time) / double(reps);
		if(avgTime < mBasicTimerOverhead)
			mBasicTimerOverhead = avgTime;

		TimerPrecise timerPrecise;
		for(u32 i = 0; i < reps; i++)
		{
			timerPrecise.Start();
			timerPrecise.Stop();
		}

		u64 avgCycles = timerPrecise.Cycles / reps;
		if(avgCycles < mPreciseTimerOverhead)
			mPreciseTimerOverhead = avgCycles;
	}

	mBasicSamplingOverheadMs = 1000000.0;
	mPreciseSamplingOverheadMs = 1000000.0;
	mBasicSamplingOverheadCycles = 1000000;
	mPreciseSamplingOverheadCycles = 1000000;
	for(u32 tries = 0; tries < 3; tries++)
	{
		/************************************************************************/
		/* 				AVERAGE TIME IN MS FOR BASIC SAMPLING                   */
		/************************************************************************/

		Timer timerA;
		timerA.Start();

		BeginThread("Main");

		// Two different cases that can effect performance, one where
		// sample already exists and other where new one needs to be created
		for(u32 i = 0; i < sampleReps; i++)
		{
			BeginSample("TestAvg1");
			EndSample("TestAvg1");
			BeginSample("TestAvg2");
			EndSample("TestAvg2");
			BeginSample("TestAvg3");
			EndSample("TestAvg3");
			BeginSample("TestAvg4");
			EndSample("TestAvg4");
			BeginSample("TestAvg5");
			EndSample("TestAvg5");
			BeginSample("TestAvg6");
			EndSample("TestAvg6");
			BeginSample("TestAvg7");
			EndSample("TestAvg7");
			BeginSample("TestAvg8");
			EndSample("TestAvg8");
			BeginSample("TestAvg9");
			EndSample("TestAvg9");
			BeginSample("TestAvg10");
			EndSample("TestAvg10");
		}

		for(u32 i = 0; i < sampleReps * 5; i++)
		{
			BeginSample(("TestAvg#" + ToString(i)).c_str());
			EndSample(("TestAvg#" + ToString(i)).c_str());
		}

		EndThread();

		timerA.Stop();

		Reset();

		double avgTimeBasic = double(timerA.Time) / double(sampleReps * 10 + sampleReps * 5) - mBasicTimerOverhead;
		if(avgTimeBasic < mBasicSamplingOverheadMs)
			mBasicSamplingOverheadMs = avgTimeBasic;

		/************************************************************************/
		/* 					AVERAGE CYCLES FOR BASIC SAMPLING                   */
		/************************************************************************/

		TimerPrecise timerPreciseA;
		timerPreciseA.Start();

		BeginThread("Main");

		// Two different cases that can effect performance, one where
		// sample already exists and other where new one needs to be created
		for(u32 i = 0; i < sampleReps; i++)
		{
			BeginSample("TestAvg1");
			EndSample("TestAvg1");
			BeginSample("TestAvg2");
			EndSample("TestAvg2");
			BeginSample("TestAvg3");
			EndSample("TestAvg3");
			BeginSample("TestAvg4");
			EndSample("TestAvg4");
			BeginSample("TestAvg5");
			EndSample("TestAvg5");
			BeginSample("TestAvg6");
			EndSample("TestAvg6");
			BeginSample("TestAvg7");
			EndSample("TestAvg7");
			BeginSample("TestAvg8");
			EndSample("TestAvg8");
			BeginSample("TestAvg9");
			EndSample("TestAvg9");
			BeginSample("TestAvg10");
			EndSample("TestAvg10");
		}

		for(u32 i = 0; i < sampleReps * 5; i++)
		{
			BeginSample(("TestAvg#" + ToString(i)).c_str());
			EndSample(("TestAvg#" + ToString(i)).c_str());
		}

		EndThread();
		timerPreciseA.Stop();

		Reset();

		u64 avgCyclesBasic = timerPreciseA.Cycles / (sampleReps * 10 + sampleReps * 5) - mPreciseTimerOverhead;
		if(avgCyclesBasic < mBasicSamplingOverheadCycles)
			mBasicSamplingOverheadCycles = avgCyclesBasic;

		/************************************************************************/
		/* 				AVERAGE TIME IN MS FOR PRECISE SAMPLING                 */
		/************************************************************************/

		Timer timerB;
		timerB.Start();
		BeginThread("Main");

		// Two different cases that can effect performance, one where
		// sample already exists and other where new one needs to be created
		for(u32 i = 0; i < sampleReps; i++)
		{
			BeginSamplePrecise("TestAvg1");
			EndSamplePrecise("TestAvg1");
			BeginSamplePrecise("TestAvg2");
			EndSamplePrecise("TestAvg2");
			BeginSamplePrecise("TestAvg3");
			EndSamplePrecise("TestAvg3");
			BeginSamplePrecise("TestAvg4");
			EndSamplePrecise("TestAvg4");
			BeginSamplePrecise("TestAvg5");
			EndSamplePrecise("TestAvg5");
			BeginSamplePrecise("TestAvg6");
			EndSamplePrecise("TestAvg6");
			BeginSamplePrecise("TestAvg7");
			EndSamplePrecise("TestAvg7");
			BeginSamplePrecise("TestAvg8");
			EndSamplePrecise("TestAvg8");
			BeginSamplePrecise("TestAvg9");
			EndSamplePrecise("TestAvg9");
			BeginSamplePrecise("TestAvg10");
			EndSamplePrecise("TestAvg10");
		}

		for(u32 i = 0; i < sampleReps * 5; i++)
		{
			BeginSamplePrecise(("TestAvg#" + ToString(i)).c_str());
			EndSamplePrecise(("TestAvg#" + ToString(i)).c_str());
		}

		EndThread();
		timerB.Stop();

		Reset();

		double avgTimesPrecise = timerB.Time / (sampleReps * 10 + sampleReps * 5);
		if(avgTimesPrecise < mPreciseSamplingOverheadMs)
			mPreciseSamplingOverheadMs = avgTimesPrecise;

		/************************************************************************/
		/* 				AVERAGE CYCLES FOR PRECISE SAMPLING                     */
		/************************************************************************/

		TimerPrecise timerPreciseB;
		timerPreciseB.Start();
		BeginThread("Main");

		// Two different cases that can effect performance, one where
		// sample already exists and other where new one needs to be created
		for(u32 i = 0; i < sampleReps; i++)
		{
			BeginSamplePrecise("TestAvg1");
			EndSamplePrecise("TestAvg1");
			BeginSamplePrecise("TestAvg2");
			EndSamplePrecise("TestAvg2");
			BeginSamplePrecise("TestAvg3");
			EndSamplePrecise("TestAvg3");
			BeginSamplePrecise("TestAvg4");
			EndSamplePrecise("TestAvg4");
			BeginSamplePrecise("TestAvg5");
			EndSamplePrecise("TestAvg5");
			BeginSamplePrecise("TestAvg6");
			EndSamplePrecise("TestAvg6");
			BeginSamplePrecise("TestAvg7");
			EndSamplePrecise("TestAvg7");
			BeginSamplePrecise("TestAvg8");
			EndSamplePrecise("TestAvg8");
			BeginSamplePrecise("TestAvg9");
			EndSamplePrecise("TestAvg9");
			BeginSamplePrecise("TestAvg10");
			EndSamplePrecise("TestAvg10");
		}

		for(u32 i = 0; i < sampleReps * 5; i++)
		{
			BeginSamplePrecise(("TestAvg#" + ToString(i)).c_str());
			EndSamplePrecise(("TestAvg#" + ToString(i)).c_str());
		}

		EndThread();
		timerPreciseB.Stop();

		Reset();

		u64 avgCyclesPrecise = timerPreciseB.Cycles / (sampleReps * 10 + sampleReps * 5);
		if(avgCyclesPrecise < mPreciseSamplingOverheadCycles)
			mPreciseSamplingOverheadCycles = avgCyclesPrecise;
	}
}

namespace b3d
{
ProfilerCPU& GetProfilerCPU()
{
	return ProfilerCPU::Instance();
}
} // namespace b3d
