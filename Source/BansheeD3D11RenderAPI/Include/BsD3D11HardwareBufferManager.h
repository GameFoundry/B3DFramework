//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsHardwareBufferManager.h"

namespace bs { namespace ct
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
		/** @copydoc HardwareBufferManager::createVertexBufferInternal */
		SPtr<VertexBuffer> createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createIndexBufferInternal */
		SPtr<IndexBuffer> createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuParamBlockBufferInternal  */
		SPtr<GpuParamBlockBuffer> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuBufferInternal */
		SPtr<GpuBuffer> createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		D3D11Device& mDevice;
	};

	/** @} */
}}