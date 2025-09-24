//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/B3DProfilingManager.h"
#include "Math/B3DMath.h"

using namespace b3d;

const u32 ProfilingManager::kNumSavedFrames = 200;

ProfilingManager::ProfilingManager()
{
	mSavedMainThreadReports = B3DNewMultiple<ProfilerReport, ProfilerAllocatorTag>(kNumSavedFrames);
	mSavedRenderThreadReports = B3DNewMultiple<ProfilerReport, ProfilerAllocatorTag>(kNumSavedFrames);
}

ProfilingManager::~ProfilingManager()
{
	if(mSavedMainThreadReports != nullptr)
		B3DDeleteMultiple<ProfilerReport, ProfilerAllocatorTag>(mSavedMainThreadReports, kNumSavedFrames);

	if(mSavedRenderThreadReports != nullptr)
		B3DDeleteMultiple<ProfilerReport, ProfilerAllocatorTag>(mSavedRenderThreadReports, kNumSavedFrames);
}

void ProfilingManager::UpdateInternal()
{
#if B3D_PROFILING_ENABLED
	mSavedMainThreadReports[mNextSimulationReportIndex].CpuReport = GetProfilerCPU().GenerateReport();

	GetProfilerCPU().Reset();

	mNextSimulationReportIndex = (mNextSimulationReportIndex + 1) % kNumSavedFrames;
#endif
}

void ProfilingManager::UpdateRenderThreadInternal()
{
#if B3D_PROFILING_ENABLED
	Lock lock(mSync);
	mSavedRenderThreadReports[mNextRenderThreadReportIndex].CpuReport = GetProfilerCPU().GenerateReport();

	GetProfilerCPU().Reset();

	mNextRenderThreadReportIndex = (mNextRenderThreadReportIndex + 1) % kNumSavedFrames;
#endif
}

const ProfilerReport& ProfilingManager::GetReport(ProfiledThread thread, u32 idx) const
{
	idx = Math::Clamp(idx, 0U, (u32)(kNumSavedFrames - 1));

	if(thread == ProfiledThread::Render)
	{
		Lock lock(mSync);

		u32 reportIdx = mNextRenderThreadReportIndex + (u32)((i32)kNumSavedFrames - ((i32)idx + 1));
		reportIdx = (reportIdx) % kNumSavedFrames;

		return mSavedRenderThreadReports[reportIdx];
	}
	else
	{
		u32 reportIdx = mNextSimulationReportIndex + (u32)((i32)kNumSavedFrames - ((i32)idx + 1));
		reportIdx = (reportIdx) % kNumSavedFrames;

		return mSavedMainThreadReports[reportIdx];
	}
}

namespace b3d
{
ProfilingManager& GetProfiler()
{
	return ProfilingManager::Instance();
}
} // namespace b3d
