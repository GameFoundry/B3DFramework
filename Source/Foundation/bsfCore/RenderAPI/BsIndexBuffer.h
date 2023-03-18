//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "CoreThread/BsCoreObject.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Descriptor structure used for initialization of an IndexBuffer. */
	struct IndexBufferCreateInformation
	{
		IndexType IndexType = IT_32BIT; /**< Index type, determines the size of a single index. */
		u32 IndexCount = 0; /**< Number of indices can buffer can hold. */
		GpuBufferFlags Flags = GpuBufferFlag::StoreOnGPU; /**< Usage that tells the hardware how will be buffer be used. */
	};

	/**	Contains information about an index buffer. */
	class B3D_CORE_EXPORT IndexBufferProperties
	{
	public:
		IndexBufferProperties(IndexType idxType, u32 numIndexes);

		/**	Returns the type of indices stored. */
		IndexType GetType() const { return mIndexType; }

		/**	Returns the number of indices this buffer can hold. */
		u32 GetNumIndices() const { return mNumIndices; }

		/**	Returns the size of a single index in bytes. */
		u32 GetIndexSize() const { return mIndexSize; }

	protected:
		friend class IndexBuffer;
		friend class ct::IndexBuffer;

		IndexType mIndexType;
		u32 mNumIndices;
		u32 mIndexSize;
	};

	/** Hardware buffer that hold indices that reference vertices in a vertex buffer. */
	class B3D_CORE_EXPORT IndexBuffer : public CoreObject
	{
	public:
		virtual ~IndexBuffer() {}

		/** Returns information about the index buffer. */
		const IndexBufferProperties& GetProperties() const { return mProperties; }

		/**
		 * Retrieves a core implementation of an index buffer usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::IndexBuffer> GetCore() const;

		/** @copydoc HardwareBufferManager::CreateIndexBuffer */
		static SPtr<IndexBuffer> Create(const IndexBufferCreateInformation& desc);

	protected:
		friend class HardwareBufferManager;

		IndexBuffer(const IndexBufferCreateInformation& desc);

		SPtr<ct::CoreObject> CreateCore() const override;

		IndexBufferProperties mProperties;
		GpuBufferFlags mBufferFlags;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/** Core thread specific implementation of an bs::IndexBuffer. */
		class B3D_CORE_EXPORT IndexBuffer : public GpuBuffer
	{
		public:
			IndexBuffer(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);
			virtual ~IndexBuffer();

			/**	Returns information about the index buffer. */
			const IndexBufferProperties& GetProperties() const { return mProperties; }

			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override;
			void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

			/**
			 * Returns a view of this buffer that can be used for load-store operations. Buffer must have been created with
			 * the GBU_LOADSTORE usage flag.
			 *
			 * @param[in]	type			Type of buffer to view the contents as. Only supported values are GBT_STANDARD and
			 *								GBT_STRUCTURED.
			 * @param[in]	format			Format of the data in the buffer. Size of the underlying buffer must be divisible by
			 *								the	size of an individual element of this format. Must be BF_UNKNOWN if buffer type
			 *								is GBT_STRUCTURED.
			 * @param[in]	elementSize		Size of the individual element in the buffer. Size of the underlying buffer must be
			 *								divisible by this size. Must be 0 if buffer type is GBT_STANDARD (element size gets
			 *								deduced from format).
			 * @return						Buffer usable for load store operations or null if the operation fails. Failure
			 *								can happen if the buffer hasn't been created with GBU_LOADSTORE usage or if the
			 *								element size doesn't divide the current buffer size.
			 */
			SPtr<GenericGpuBuffer> GetLoadStore(GenericGpuBufferType type, GpuBufferFormat format, u32 elementSize = 0);

			/** @copydoc HardwareBufferManager::CreateIndexBuffer */
			static SPtr<IndexBuffer> Create(const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class HardwareBufferManager;

			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) override;
			void Unmap() override;
			void Initialize() override;

			IndexBufferProperties mProperties;

			GpuBuffer* mBuffer = nullptr;
			SPtr<GpuBuffer> mSharedBuffer;
			Vector<SPtr<GenericGpuBuffer>> mLoadStoreViews;

			typedef void (*Deleter)(GpuBuffer*);
			Deleter mBufferDeleter = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
