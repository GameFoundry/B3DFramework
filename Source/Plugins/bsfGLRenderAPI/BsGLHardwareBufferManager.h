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
			SPtr<VertexBuffer> CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<IndexBuffer> CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GenericGpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GenericGpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
