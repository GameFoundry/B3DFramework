//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsHardwareBufferManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Handles creation of OpenGL specific hardware buffers. */
		class GLHardwareBufferManager : public HardwareBufferManager
		{
		public:
			/**	Converts engine buffer usage flags into OpenGL specific flags. */
			static GLenum GetGlUsage(GpuBufferUsage usage);

			/**	Converts vertex element type into OpenGL specific type. */
			static GLenum GetGlType(VertexElementType type);

		protected:
			/** @copydoc HardwareBufferManager::createVertexBufferInternal */
			SPtr<VertexBuffer> CreateVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createIndexBufferInternal */
			SPtr<IndexBuffer> CreateIndexBufferInternal(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createGpuParamBlockBufferInternal */
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC&, GpuDeviceFlags) */
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc HardwareBufferManager::createGpuBufferInternal(const GPU_BUFFER_DESC&, SPtr<HardwareBuffer>) */
			SPtr<GpuBuffer> CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);
		};

		/** @} */
	} // namespace ct
} // namespace bs
