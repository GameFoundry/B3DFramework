//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanEventQuery.h"
#include "BsVulkanRenderAPI.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine
{
	VulkanEventQuery::VulkanEventQuery(UINT32 deviceIdx)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanEventQuery::~VulkanEventQuery()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanEventQuery::begin()
	{
		setActive(true);
	}

	bool VulkanEventQuery::isReady() const
	{
		return false;
	}
}