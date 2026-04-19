//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DVulkanQueries.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanUtility.h"

using namespace b3d;
using namespace b3d::render;

VulkanGpuQueryPool::VulkanGpuQueryPool(VulkanResourceManager& vulkanResourceManager, const GpuQueryPoolCreateInformation& createInformation)
	: GpuQueryPool(createInformation), VulkanResource(&vulkanResourceManager, false, "QueryPool")
{
	VkQueryPoolCreateInfo queryPoolCreateInfo;
	queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolCreateInfo.pNext = nullptr;
	queryPoolCreateInfo.flags = 0;
	queryPoolCreateInfo.pipelineStatistics = createInformation.Type == GpuQueryType::PipelineStatistics ? VulkanUtility::GetPipelineStatisticQueryBits(createInformation.PipelineStatisticsQueryBits) : 0;
	queryPoolCreateInfo.queryCount = mPoolSize;
	queryPoolCreateInfo.queryType = VulkanUtility::GetQueryType(createInformation.Type);

	VulkanGpuDevice& device = mOwner->GetDevice();
	VkResult result = vkCreateQueryPool(device.GetLogical(), &queryPoolCreateInfo, gVulkanAllocator, &mPool);
	B3D_ASSERT(result == VK_SUCCESS);

	if(createInformation.Type == GpuQueryType::PipelineStatistics)
		mElementsPerQuery = Bitwise::CountSetBits(queryPoolCreateInfo.pipelineStatistics);
	else
		mElementsPerQuery = 1;

	B3D_ENSURE(mElementsPerQuery > 0);

	mResultBuffer.Resize(createInformation.PoolSize * mElementsPerQuery);
}

VulkanGpuQueryPool::~VulkanGpuQueryPool()
{
	VulkanGpuDevice& device = mOwner->GetDevice();
	vkDestroyQueryPool(device.GetLogical(), mPool, gVulkanAllocator);
}

GpuQueryId VulkanGpuQueryPool::AllocateQuery()
{
	if(mNextFreeQueryId < mPoolSize)
		return GpuQueryId(mNextFreeQueryId++);

	return ~0u;
}

bool VulkanGpuQueryPool::TryResolve(bool wait)
{
	VulkanGpuDevice& device = mOwner->GetDevice();

	if(mNextFreeQueryId == 0)
		return true;

	if(IsBound())
	{
		if(!wait)
			return false;

		device.WaitUntilIdle();
	}

	VkDevice vkDevice = device.GetLogical();
	VkResult vkResult = vkGetQueryPoolResults(vkDevice, mPool, 0, mNextFreeQueryId, mResultBuffer.Size() * sizeof(u64), mResultBuffer.Data(), sizeof(u64) * mElementsPerQuery, VK_QUERY_RESULT_64_BIT | (wait ? VK_QUERY_RESULT_WAIT_BIT : 0));
	B3D_ASSERT(vkResult == VK_SUCCESS || vkResult == VK_NOT_READY);

	return vkResult == VK_SUCCESS;
}

u64 VulkanGpuQueryPool::GetQueryResult(GpuQueryId queryId, u32 elementIndex)
{
	if(!B3D_ENSURE(elementIndex < mElementsPerQuery))
		return 0;

	return mResultBuffer[queryId.Id * mElementsPerQuery];
}
