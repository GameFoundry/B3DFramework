//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsHardwareBufferManager.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles creation of OpenGL specific hardware buffers. */
    class BS_RSGL_EXPORT GLHardwareBufferManager : public HardwareBufferManager
    {
    public:
		/**	Converts engine buffer usage flags into OpenGL specific flags. */
		static GLenum getGLUsage(GpuBufferUsage usage);

		/**	Converts vertex element type into OpenGL specific type. */
        static GLenum getGLType(VertexElementType type);

	protected:
		/** @copydoc HardwareBufferManager::createVertexBufferInternal */
        SPtr<VertexBufferCore> createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
        	GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createIndexBufferInternal */
		SPtr<IndexBufferCore> createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuParamBlockBufferInternal */
		SPtr<GpuParamBlockBufferCore> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc HardwareBufferManager::createGpuBufferInternal */
		SPtr<GpuBufferCore> createGpuBufferInternal(const GPU_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
    };

	/** @} */
}}