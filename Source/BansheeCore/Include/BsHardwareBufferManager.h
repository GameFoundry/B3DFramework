//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsVertexBuffer.h"
#include "BsIndexBuffer.h"
#include "BsVertexDeclaration.h"

namespace bs 
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
		 * @param[in]	desc	Description of the buffer to create.
		 */
		SPtr<VertexBuffer> createVertexBuffer(const VERTEX_BUFFER_DESC& desc);

		/**
		 * Creates a new index buffer that holds indices referencing vertices in a vertex buffer. Indices are interpreted 
		 * by the pipeline and vertices are drawn in the order specified in the index buffer.
		 *
		 * @param[in]	desc	Description of the buffer to create.
		 */
		SPtr<IndexBuffer> createIndexBuffer(const INDEX_BUFFER_DESC& desc);

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

		/** 
		 * Creates a new vertex declaration from a list of vertex elements. 
		 * 
		 * @param[in]	desc	Description of the object to create.
		 */
		SPtr<VertexDeclaration> createVertexDeclaration(const SPtr<VertexDataDesc>& desc);

		/** @copydoc GpuParams::create(const SPtr<GpuPipelineParamInfo>&) */
		SPtr<GpuParams> createGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo);
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

		/** 
		 * @copydoc HardwareBufferManager::createVertexBuffer 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<VertexBufferCore> createVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc HardwareBufferManager::createIndexBuffer 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<IndexBufferCore> createIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc HardwareBufferManager::createVertexDeclaration 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<VertexDeclarationCore> createVertexDeclaration(const SPtr<VertexDataDesc>& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/**
		 * Creates a new vertex declaration from a list of vertex elements.
		 *
		 * @param[in]	elements		List of elements to initialize the declaration with.
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<VertexDeclarationCore> createVertexDeclaration(const List<VertexElement>& elements, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc HardwareBufferManager::createGpuParamBlockBuffer 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<GpuParamBlockBufferCore> createGpuParamBlockBuffer(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc HardwareBufferManager::createGpuBuffer
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		SPtr<GpuBufferCore> createGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** @copydoc GpuParamsCore::create(const SPtr<GpuPipelineParamInfoCore>&, GpuDeviceFlags) */
		SPtr<GpuParamsCore> createGpuParams(const SPtr<GpuPipelineParamInfoCore>& paramInfo,
											GpuDeviceFlags deviceMask = GDF_DEFAULT);
	protected:
		friend class IndexBuffer;
		friend class IndexBufferCore;
		friend class VertexBuffer;
		friend class VertexBufferCore;
		friend class VertexDeclaration;
		friend class GpuParamBlockBuffer;
		friend class GpuBuffer;
		friend class GpuBufferCore;

		/** Key for use in the vertex declaration map. */
		struct VertexDeclarationKey
		{
			VertexDeclarationKey(const List<VertexElement>& elements);

			class HashFunction
			{
			public:
				size_t operator()(const VertexDeclarationKey& key) const;
			};

			class EqualFunction
			{
			public:
				bool operator()(const VertexDeclarationKey& lhs, const VertexDeclarationKey& rhs) const;
			};

			List<VertexElement> elements;
		};

		/** @copydoc createVertexBuffer */
		virtual SPtr<VertexBufferCore> createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc createIndexBuffer */
		virtual SPtr<IndexBufferCore> createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc createGpuParamBlockBuffer */
		virtual SPtr<GpuParamBlockBufferCore> createGpuParamBlockBufferInternal(UINT32 size, 
			GpuParamBlockUsage usage = GPBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc createGpuBuffer */
		virtual SPtr<GpuBufferCore> createGpuBufferInternal(const GPU_BUFFER_DESC& desc, 
			GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

		/** @copydoc createVertexDeclaration(const List<VertexElement>&, GpuDeviceFlags) */
		virtual SPtr<VertexDeclarationCore> createVertexDeclarationInternal(const List<VertexElement>& elements,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** @copydoc createGpuParams */
		virtual SPtr<GpuParamsCore> createGpuParamsInternal(const SPtr<GpuPipelineParamInfoCore>& paramInfo,
															GpuDeviceFlags deviceMask = GDF_DEFAULT);

		typedef UnorderedMap<VertexDeclarationKey, SPtr<VertexDeclarationCore>, 
			VertexDeclarationKey::HashFunction, VertexDeclarationKey::EqualFunction> DeclarationMap;

		DeclarationMap mCachedDeclarations;
	};

	/** @} */
}