//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles creation of OpenGL specific hardware buffers. */
	class GLHardwareBufferManager : public HardwareBufferManager
	{
	public:
		/**	Converts engine buffer usage flags into OpenGL specific flags. */
		static GLenum getGLUsage(GpuBufferUsage usage);

		/**	Converts vertex element type into OpenGL specific type. */
		static GLenum getGLType(VertexElementType type);

	protected:
		/** @copydoc HardwareBufferManager::createVertexBufferInternal */
		SPtr<VertexBuffer> createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createIndexBufferInternal */
		SPtr<IndexBuffer> createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuParamBlockBufferInternal */
		SPtr<GpuParamBlockBuffer> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuBufferInternal */
		SPtr<GpuBuffer> createGpuBufferInternal(const GPU_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
	};

	/** @} */
}}