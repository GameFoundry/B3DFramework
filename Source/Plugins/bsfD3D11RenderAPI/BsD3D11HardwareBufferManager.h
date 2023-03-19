//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsHardwareBufferManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Handles creation of DirectX 11 hardware buffers. */
		class D3D11HardwareBufferManager : public HardwareBufferManager
		{
		public:
			D3D11HardwareBufferManager(D3D11Device& device);

		protected:
			SPtr<VertexBuffer> CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<IndexBuffer> CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer) override;

			D3D11Device& mDevice;
		};

		/** @} */
	} // namespace ct
} // namespace bs
