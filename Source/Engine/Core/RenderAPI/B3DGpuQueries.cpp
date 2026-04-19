//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGPUQueries.h"

using namespace b3d::render;

GpuQueryPool::GpuQueryPool(const GpuQueryPoolCreateInformation& createInformation)
	: mQueryType(createInformation.Type), mPoolSize(createInformation.PoolSize)
{
	B3D_ENSURE(mQueryType != GpuQueryType::PipelineStatistics || Bitwise::CountSetBits(createInformation.PipelineStatisticsQueryBits) > 0);
}

