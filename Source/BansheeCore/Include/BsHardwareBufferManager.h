//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsVertexBuffer.h"
#include "BsIndexBuffer.h"
#include "BsVertexDeclaration.h"

namespace BansheeEngine 
{
	struct GPU_BUFFER_DESC;
	struct GPU_PARAMS_DESC;

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Handles creation of various hardware buffers.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
	{
    public:
        HardwareBufferManager();
        virtual ~HardwareBufferManager();

		/**
		 * Creates a new vertex buffer used for holding number of vertices and other per-vertex data. Buffer can be bound 
		 * to the pipeline and its data can be passed to the active vertex GPU program.
		 *
		 * @param[in]	vertexSize	Size of a single vertex in the buffer, in bytes.
		 * @param[in]	numVerts	Number of vertices the buffer can hold.
		 * @param[in]	usage		Usage that tells the hardware how will be buffer be used. 
		 * @param[in]	streamOut	If true the buffer will be usable for streaming out data from the GPU.
		 */
		SPtr<VertexBuffer> createVertexBuffer(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, 
			bool streamOut = false);

		/**
		 * Creates a new index buffer that holds indices referencing vertices in a vertex buffer. Indices are interpreted 
		 * by the pipeline and vertices are drawn in the order specified in the index buffer.
		 *
		 * @param[in]	itype		Index type, determines size of an index.
		 * @param[in]	numIndices	Number of indexes can buffer can hold.
		 * @param[in]	usage		Usage that tells the hardware how will be buffer be used. 
		 */
		SPtr<IndexBuffer> createIndexBuffer(IndexType itype, UINT32 numIndices, GpuBufferUsage usage);

		/**
		 * Creates an GPU parameter block that you can use for setting parameters for GPU programs. Parameter blocks may be
		 * used for sharing parameter data between multiple GPU programs, requiring you to update only one buffer for all of
		 * them, potentially improving performance.
		 *
		 * @param[in]	size	Size of the parameter buffer in bytes.
		 * @param[in]	usage	Usage that tells the hardware how will be buffer be used. 
		 */
		SPtr<GpuParamBlockBuffer> createGpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage = GPBU_DYNAMIC);

		/**
		 * Creates a generic buffer that can be passed as a parameter to a GPU program. This type of buffer can hold various 
		 * type of data and can be used for various purposes. See GpuBufferType for explanation of different buffer types.
		 *
		 * @param[in]	desc  	Description of the buffer to create.
		 */
		SPtr<GpuBuffer> createGpuBuffer(const GPU_BUFFER_DESC& desc);

		/** Creates a new vertex declaration from a list of vertex elements. */
		SPtr<VertexDeclaration> createVertexDeclaration(const SPtr<VertexDataDesc>& desc);

		/** Creates a new GpuParams object from the provided set of GPU parameter descriptors. */
		SPtr<GpuParams> createGpuParams(const GPU_PARAMS_DESC& desc);
	};

	/**
	 * Handles creation of various hardware buffers.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT HardwareBufferCoreManager : public Module<HardwareBufferCoreManager>
	{
    public:
		virtual ~HardwareBufferCoreManager() { }

		/** @copydoc HardwareBufferManager::createVertexBuffer */
		SPtr<VertexBufferCore> createVertexBuffer(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, 
			bool streamOut = false);

		/** @copydoc HardwareBufferManager::createIndexBuffer */
		SPtr<IndexBufferCore> createIndexBuffer(IndexType itype, UINT32 numIndices, GpuBufferUsage usage);

		/** @copydoc HardwareBufferManager::createVertexDeclaration */
		SPtr<VertexDeclarationCore> createVertexDeclaration(const SPtr<VertexDataDesc>& desc);

		/** @copydoc HardwareBufferManager::createVertexDeclaration */
		SPtr<VertexDeclarationCore> createVertexDeclaration(const List<VertexElement>& elements);

		/** @copydoc HardwareBufferManager::createGpuParamBlockBuffer */
		SPtr<GpuParamBlockBufferCore> createGpuParamBlockBuffer(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC);

		/** @copydoc HardwareBufferManager::createGpuBuffer */
		SPtr<GpuBufferCore> createGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** @copydoc HardwareBufferManager::createGpuParams */
		SPtr<GpuParamsCore> createGpuParams(const GPU_PARAMS_DESC& desc);

	protected:
		friend class IndexBuffer;
		friend class IndexBufferCore;
		friend class VertexBuffer;
		friend class VertexBufferCore;
		friend class VertexDeclaration;
		friend class GpuParamBlockBuffer;
		friend class GpuBuffer;
		friend class GpuBufferCore;

		/** @copydoc createVertexBuffer */
		virtual SPtr<VertexBufferCore> createVertexBufferInternal(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, 
			bool streamOut = false) = 0;

		/** @copydoc createIndexBuffer */
		virtual SPtr<IndexBufferCore> createIndexBufferInternal(IndexType itype, UINT32 numIndices, 
			GpuBufferUsage usage) = 0;

		/** @copydoc createGpuParamBlockBuffer */
		virtual SPtr<GpuParamBlockBufferCore> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC) = 0;

		/** @copydoc createGpuBuffer */
		virtual SPtr<GpuBufferCore> createGpuBufferInternal(const GPU_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc createVertexDeclaration */
		virtual SPtr<VertexDeclarationCore> createVertexDeclarationInternal(const List<VertexElement>& elements);

		/** @copydoc createGpuParams */
		virtual SPtr<GpuParamsCore> createGpuParamsInternal(const GPU_PARAMS_DESC& desc);
	};

	/** @} */
}

