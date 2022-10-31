//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/BsProfilingManager.h"
#include "Math/BsMath.h"

using namespace bs;

const u32 ProfilingManager::kNumSavedFrames = 200;

ProfilingManager::ProfilingManager()
{
	mSavedSimReports = bs_newN<ProfilerReport, ProfilerAlloc>(kNumSavedFrames);
	mSavedCoreReports = bs_newN<ProfilerReport, ProfilerAlloc>(kNumSavedFrames);
}

ProfilingManager::~ProfilingManager()
{
	if(mSavedSimReports != nullptr)
		bs_deleteN<ProfilerReport, ProfilerAlloc>(mSavedSimReports, kNumSavedFrames);

	if(mSavedCoreReports != nullptr)
		bs_deleteN<ProfilerReport, ProfilerAlloc>(mSavedCoreReports, kNumSavedFrames);
}

void ProfilingManager::UpdateInternal()
{
#if BS_PROFILING_ENABLED
	mSavedSimReports[mNextSimReportIdx].CpuReport = GetProfilerCPU().GenerateReport();

	GetProfilerCPU().Reset();

	mNextSimReportIdx = (mNextSimReportIdx + 1) % kNumSavedFrames;
#endif
}

void ProfilingManager::UpdateCoreInternal()
{
#if BS_PROFILING_ENABLED
	Lock lock(mSync);
	mSavedCoreReports[mNextCoreReportIdx].CpuReport = GetProfilerCPU().GenerateReport();

	GetProfilerCPU().Reset();

	mNextCoreReportIdx = (mNextCoreReportIdx + 1) % kNumSavedFrames;
#endif
}

const ProfilerReport& ProfilingManager::GetReport(ProfiledThread thread, u32 idx) const
{
	idx = Math::Clamp(idx, 0U, (u32)(kNumSavedFrames - 1));

	if(thread == ProfiledThread::Core)
	{
		Lock lock(mSync);

		u32 reportIdx = mNextCoreReportIdx + (u32)((i32)kNumSavedFrames - ((i32)idx + 1));
		reportIdx = (reportIdx) % kNumSavedFrames;

		return mSavedCoreReports[reportIdx];
	}
	else
	{
		u32 reportIdx = mNextSimReportIdx + (u32)((i32)kNumSavedFrames - ((i32)idx + 1));
		reportIdx = (reportIdx) % kNumSavedFrames;

		return mSavedSimReports[reportIdx];
	}
}

namespace bs
{
ProfilingManager& GetProfiler()
{
	return ProfilingManager::Instance();
}
} // namespace bs
