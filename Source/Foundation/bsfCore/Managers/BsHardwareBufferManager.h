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
	struct GenericGpuBufferCreateInformation;
	struct GPU_PARAMS_DESC;

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Handles creation of various hardware buffers.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
	{
	public:
		HardwareBufferManager() = default;
		virtual ~HardwareBufferManager() = default;

		/**
		 * Creates a new vertex buffer used for holding number of vertices and other per-vertex data. Buffer can be bound
		 * to the pipeline and its data can be passed to the active vertex GPU program.
		 *
		 * @param[in]	createInformation	Description of the buffer to create.
		 */
		SPtr<VertexBuffer> CreateVertexBuffer(const VertexBufferCreateInformation& createInformation);

		/**
		 * Creates a new index buffer that holds indices referencing vertices in a vertex buffer. Indices are interpreted
		 * by the pipeline and vertices are drawn in the order specified in the index buffer.
		 *
		 * @param[in]	createInformation	Description of the buffer to create.
		 */
		SPtr<IndexBuffer> CreateIndexBuffer(const IndexBufferCreateInformation& createInformation);

		/**
		 * Creates an GPU parameter block that you can use for setting parameters for GPU programs. Parameter blocks may be
		 * used for sharing parameter data between multiple GPU programs, requiring you to update only one buffer for all of
		 * them, potentially improving performance.
		 *
		 * @param[in]	size	Size of the parameter buffer in bytes.
		 * @param[in]	flags	Flags that control the behavior of the buffer.
		 */
		SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBuffer(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU);

		/**
		 * Creates a generic buffer that can be passed as a parameter to a GPU program. This type of buffer can hold various
		 * type of data and can be used for various purposes. See GenericGpuBufferType for explanation of different buffer types.
		 *
		 * @param[in]	createInformation  	Description of the buffer to create.
		 */
		SPtr<GenericGpuBuffer> CreateGpuBuffer(const GenericGpuBufferCreateInformation& createInformation);

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
		class B3D_CORE_EXPORT HardwareBufferManager : public Module<HardwareBufferManager>
		{
		public:
			virtual ~HardwareBufferManager() {}

			/**
			 * Creates a generic buffer allocated on the GPU. This buffer can be used as a backing memory for specific buffer types.
			 *
			 * @param	type			Determines what buffers can this buffer be used as a backing memory for.
			 * @param	size			Size of the buffer in bytes.
			 * @param	usage			Determines how is the buffer intended to be used.
			 * @param	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			virtual SPtr<GpuBuffer> CreateHardwareBuffer(GpuBufferType type, u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/**
			 * @copydoc bs::HardwareBufferManager::CreateVertexBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexBuffer> CreateVertexBuffer(const VertexBufferCreateInformation& createInformation, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateIndexBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<IndexBuffer> CreateIndexBuffer(const IndexBufferCreateInformation& createInformation, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateVertexDeclaration
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexDeclaration> CreateVertexDeclaration(const SPtr<VertexDataDesc>& createInformation, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Creates a new vertex declaration from a list of vertex elements.
			 *
			 * @param[in]	elements		List of elements to initialize the declaration with.
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<VertexDeclaration> CreateVertexDeclaration(const Vector<VertexElement>& elements, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuParamBlockBuffer::Create(u32, GpuBufferUsage, GpuDeviceFlags) */
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBuffer(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GpuParamBlockBuffer::Create(const SPtr<GpuBuffer>&, u32, u32) */
			SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBuffer(const SPtr<GpuBuffer>& owner, u32 offset, u32 size);

			/**
			 * @copydoc bs::HardwareBufferManager::CreateGpuBuffer
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			SPtr<GenericGpuBuffer> CreateGpuBuffer(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/** @copydoc GenericGpuBuffer::Create(const GPU_BUFFER_DESC&, SPtr<GpuBuffer>) */
			SPtr<GenericGpuBuffer> CreateGpuBuffer(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer);

			/** @copydoc GpuParams::Create(const SPtr<GpuPipelineParamInfo>&, GpuDeviceFlags) */
			SPtr<GpuParams> CreateGpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class bs::IndexBuffer;
			friend class IndexBuffer;
			friend class bs::VertexBuffer;
			friend class VertexBuffer;
			friend class bs::VertexDeclaration;
			friend class bs::GpuParamBlockBuffer;
			friend class bs::GenericGpuBuffer;
			friend class GenericGpuBuffer;

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
			virtual SPtr<VertexBuffer> CreateVertexBufferInternal(const VertexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateIndexBuffer */
			virtual SPtr<IndexBuffer> CreateIndexBufferInternal(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateGpuParamBlockBuffer(u32, GpuBufferUsage, GpuDeviceFlags) */
			virtual SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateGpuParamBlockBuffer(const SPtr<GpuBuffer>&, u32, u32) */
			virtual SPtr<GpuParamBlockBuffer> CreateGpuParamBlockBufferInternal(const SPtr<GpuBuffer>& owner, u32 offset, u32 size) = 0;

			/** @copydoc CreateGpuBuffer(const GPU_BUFFER_DESC&, GpuDeviceFlags) */
			virtual SPtr<GenericGpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) = 0;

			/** @copydoc CreateGpuBuffer(const GPU_BUFFER_DESC&, SPtr<GpuBuffer>) */
			virtual SPtr<GenericGpuBuffer> CreateGpuBufferInternal(const GenericGpuBufferCreateInformation& desc, SPtr<GpuBuffer> underlyingBuffer) = 0;

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
