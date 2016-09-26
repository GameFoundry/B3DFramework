//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsHardwareBufferManager.h"

namespace BansheeEngine 
{
	/** @addtogroup D3D9
	 *  @{
	 */

	/**	Manages creation of DX9 hardware buffers. */
    class BS_D3D9_EXPORT D3D9HardwareBufferCoreManager : public HardwareBufferCoreManager
    {
	protected:     
		/** @copydoc HardwareBufferCoreManager::createVertexBufferInternal */
		SPtr<VertexBufferCore> createVertexBufferInternal(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut = false) override;

		/** @copydoc HardwareBufferCoreManager::createIndexBufferInternal */
		SPtr<IndexBufferCore> createIndexBufferInternal(IndexType itype, UINT32 numIndices, GpuBufferUsage usage) override;

		/** @copydoc	HardwareBufferCoreManager::createVertexDeclarationInternal */
		SPtr<VertexDeclarationCore> createVertexDeclarationInternal(const List<VertexElement>& elements) override;

		/** @copydoc HardwareBufferCoreManager::createGpuParamBlockBufferInternal */
		SPtr<GpuParamBlockBufferCore> createGpuParamBlockBufferInternal(UINT32 size, GpuParamBlockUsage usage = GPBU_DYNAMIC) override;

		/** @copydoc HardwareBufferCoreManager::createGpuBufferInternal */
		SPtr<GpuBufferCore> createGpuBufferInternal(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, 
			GpuBufferFormat format, GpuBufferUsage usage, bool randomGpuWrite = false, bool useCounter = false) override;
    };

	/** @} */
}