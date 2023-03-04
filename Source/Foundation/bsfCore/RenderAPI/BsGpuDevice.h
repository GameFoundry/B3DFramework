//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Provides access to a particular GPU device.
	 *
	 * @note	Thread safe.
	 */
	class B3D_CORE_EXPORT GpuDevice
	{
	public:
		virtual ~GpuDevice() = default;

		// TODO - Doc
		virtual bool IsInitialized() const = 0;
		virtual bool Initialize() = 0;

		virtual const GpuDeviceCapabilities& GetCapabilities() = 0;

		/** Returns information about available output devices and their video modes. */
		virtual const VideoModeInfo& GetVideoModeInfo() const = 0;

		//virtual SPtr<ct::EventQuery> CreateEventQuery() = 0;
		//virtual SPtr<ct::TimerQuery> CreateTimerQuery() = 0;
	};

	/** @} */

} // namespace bs
