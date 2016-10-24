//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanOcclusionQuery.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	VulkanOcclusionQuery::VulkanOcclusionQuery(bool binary, UINT32 deviceIdx)
		:OcclusionQuery(binary)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanOcclusionQuery::~VulkanOcclusionQuery()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanOcclusionQuery::begin()
	{
		setActive(true);
	}

	void VulkanOcclusionQuery::end()
	{

	}

	bool VulkanOcclusionQuery::isReady() const
	{
		return false;
	}

	UINT32 VulkanOcclusionQuery::getNumSamples()
	{
		return 0;
	}
}