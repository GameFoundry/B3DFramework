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

		/** Create a new event query. */
		virtual SPtr<ct::EventQuery> CreateEventQuery() = 0;

		/** Creates a new timer query. */
		virtual SPtr<ct::TimerQuery> CreateTimerQuery() = 0;

		/**
		 * Creates a new occlusion query.
		 *
		 * @param isBinary		If query is binary it will not give you an exact count of samples rendered, but will
		 *						instead just return 0 (no samples were rendered) or 1 (one or more samples were
		 *						rendered). Binary queries can return sooner as they potentially do not need to wait
		 *						until all of the geometry is rendered.
		 */
		virtual SPtr<ct::OcclusionQuery> CreateOcclusionQuery(bool isBinary) = 0;
	};

	/** @} */

} // namespace bs
