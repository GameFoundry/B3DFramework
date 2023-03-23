//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Utility/BsModule.h"

namespace bs
{
	struct GenericGpuBufferCreateInformation;
	struct GPU_PARAMS_DESC;

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Handles creation of various hardware buffers.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
	{
	public:
		HardwareBufferManager() = default;
		virtual ~HardwareBufferManager() = default;

		/** @copydoc GpuParams::Create(const SPtr<GpuPipelineParamInfo>&) */
		SPtr<GpuParams> CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo);
	};

	namespace ct
	{

		/**
		 * Handles creation of various hardware buffers.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
		{
		public:
			virtual ~HardwareBufferManager() {}

			/** @copydoc GpuParams::Create(const SPtr<GpuPipelineParamInfo>&, GpuDeviceFlags) */
			SPtr<GpuParams> CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class bs::GpuBuffer;

			/** @copydoc CreateGpuParams */
			virtual SPtr<GpuParams> CreateGpuParamsInternal(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);
		};
	} // namespace ct

	/** @} */
} // namespace bs
