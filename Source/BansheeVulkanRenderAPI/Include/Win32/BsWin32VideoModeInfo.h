//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVideoModeInfo.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc VideoMode */
	class Win32VideoMode : public VideoMode
	{
	public:
		Win32VideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx);

	private:
		friend class VulkanVideoOutputInfo;
	};

	/** @copydoc VideoOutputInfo */
	class Win32VideoOutputInfo : public VideoOutputInfo
	{
	public:
		Win32VideoOutputInfo(UINT32 outputIdx);
		~Win32VideoOutputInfo();
	};

	/** @copydoc VideoModeInfo */
	class Win32VideoModeInfo : public VideoModeInfo
	{
	public:
		Win32VideoModeInfo();
	};

	/** @} */
}