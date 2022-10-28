//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "RenderAPI/BsVertexDeclaration.h"

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
		HardwareBufferManager() = default;
		virtual ~HardwareBufferManager() = default;

		/**
		 * Creates a new vertex buffer used for holding number of vertices and other per-vertex data. Buffer can be bound
		 * to the pipeline and its data can be passed to the active vertex GPU program.
		 *
		 * @param[in]	desc	Description of the buffer to create.
		 */
		SPtr<VertexBuffer> CreateVertexBuffer(const VERTEX_BUFFER_DESC& desc);

		/**
		 * Creates a new index buffer that holds indices referencing vertices in a vertex buffer. Indices are interpreted
		 * by the pipeline and vertices are drawn in the order specified in the index buffer.
		 *
		 * @param[in]	desc	Description of the buffer to create.
		 */
		SPtr<IndexBuffer> CreateIndexBuffer(const INDEX_BUFFER_DESC& desc);

		/**
		 * Creates an GPU parameter block that you can use for setting parameters for GPU programs. Parameter blocks may be
		 * used for sharing parameter data between multiple GPU programs, requiring you to update only one buffer for all of
		 * them, potentially improving performance.
		 *
		 * @param[in]	size	Size of the parameter buffer in bytes.
		 * @param[in]	usage	Usage that tells the hardware how will be buffer be used.
		 */
		SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBuffer(u32 size, GpuBufferUsage usage = GBU_DYNAMIC);

		/**
		 * Creates a generic buffer that can be passed as a parameter to a GPU program. This type of buffer can hold various
		 * type of data and can be used for various purposes. See GpuBufferType for explanation of different buffer types.
		 *
		 * @param[in]	desc  	Description of the buffer to create.
		 */
		SPtr<GpuBuffer> CreateGpuBuffer(const GPU_BUFFER_DESC& desc);

		/**
		 * Creates a new vertex declaration from a list of vertex elements.
		 *
		 * @param[in]	desc	Description of the object to create.
		 */
		SPtr<VertexDeclaration> CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc);

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
		class BS_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
		{
		public:
			virtual ~HardwareBufferManager() {}

			/**
			 * @copydoc bs::HardwareBufferManager::CreateVertexBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexBuffer> CreateVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateIndexBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<IndexBuffer> CreateIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateVertexDeclaration
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexDeclaration> CreateVertexDeclaration(const SPtr<VertexDataDesc>& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Creates a new vertex declaration from a list of vertex elements.
			 *
			 * @param[in]	elements		List of elements to initialize the declaration with.
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexDeclaration> CreateVertexDeclaration(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateGpuParamBlockBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBuffer(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateGpuBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<GpuBuffer> CreateGpuBuffer(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuBuffer::Create(const GPU_BUFFER_DESC&, SPtr<HardwareBuffer>) */
			SPtr<GpuBuffer> CreateGpuBuffer(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer);

			/** @copydoc GpuParams::Create(const SPtr<GpuPipelineParamInfo>&, GpuDeviceFlags) */
			SPtr<GpuParams> CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class bs::IndexBuffer;
			friend class IndexBuffer;
			friend class bs::VertexBuffer;
			friend class VertexBuffer;
			friend class bs::VertexDeclaration;
			friend class bs::GpuParamBlockBuffer;
			friend class bs::GpuBuffer;
			friend class GpuBuffer;

			/** Key for use in the vertex declaration map. */
			struct VertexDeclarationKey
			{
				VertexDeclarationKey(const Vector<VertexElement>& elements);

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

				Vector<VertexElement> Elements;
			};

			/** @copydoc CreateVertexBuffer */
			virtual SPtr<VertexBuffer> CreateVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateIndexBuffer */
			virtual SPtr<IndexBuffer> CreateIndexBufferInternal(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateGpuParamBlockBuffer */
			virtual SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage = GBU_DYNAMIC, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateGpuBuffer(const GPU_BUFFER_DESC&, GpuDeviceFlags) */
			virtual SPtr<GpuBuffer> CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateGpuBuffer(const GPU_BUFFER_DESC&, SPtr<HardwareBuffer>) */
			virtual SPtr<GpuBuffer> CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer) = 0;

			/** @copydoc CreateVertexDeclaration(const Vector<VertexElement>&, GpuDeviceFlags) */
			virtual SPtr<VertexDeclaration> CreateVertexDeclarationInternal(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc CreateGpuParams */
			virtual SPtr<GpuParams> CreateGpuParamsInternal(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			typedef UnorderedMap<VertexDeclarationKey, SPtr<VertexDeclaration>, VertexDeclarationKey::HashFunction, VertexDeclarationKey::EqualFunction> DeclarationMap;

			DeclarationMap mCachedDeclarations;
		};
	} // namespace ct

	/** @} */
} // namespace bs
