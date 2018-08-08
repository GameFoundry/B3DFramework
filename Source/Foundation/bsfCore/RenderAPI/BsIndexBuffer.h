//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "CoreThread/BsCoreObject.h"

namespace bs 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Descriptor structure used for initialization of an IndexBuffer. */
	struct INDEX_BUFFER_DESC
	{
		IndexType indexType; /**< Index type, determines the size of a single index. */
		UINT32 numIndices; /**< Number of indices can buffer can hold. */
		GpuBufferUsage usage = GBU_STATIC; /**< Usage that tells the hardware how will be buffer be used. */
	};

	/**	Contains information about an index buffer. */
	class BS_CORE_EXPORT IndexBufferProperties
	{
	public:
		IndexBufferProperties(IndexType idxType, UINT32 numIndexes);

		/**	Returns the type of indices stored. */
		IndexType getType() const { return mIndexType; }

		/**	Returns the number of indices this buffer can hold. */
		UINT32 getNumIndices() const { return mNumIndices; }

		/**	Returns the size of a single index in bytes. */
		UINT32 getIndexSize() const { return mIndexSize; }

	protected:
		friend class IndexBuffer;
		friend class ct::IndexBuffer;

		IndexType mIndexType;
		UINT32 mNumIndices;
		UINT32 mIndexSize;
	};

	/** Hardware buffer that hold indices that reference vertices in a vertex buffer. */
	class BS_CORE_EXPORT IndexBuffer : public CoreObject
	{
	public:
		virtual ~IndexBuffer() { }

		/** Returns information about the index buffer. */
		const IndexBufferProperties& getProperties() const { return mProperties; }

		/**
		 * Retrieves a core implementation of an index buffer usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::IndexBuffer> getCore() const;

		/** @copydoc HardwareBufferManager::createIndexBuffer */
		static SPtr<IndexBuffer> create(const INDEX_BUFFER_DESC& desc);

	protected:
		friend class HardwareBufferManager;

		IndexBuffer(const INDEX_BUFFER_DESC& desc);

		/** @copydoc CoreObject::createCore */
		virtual SPtr<ct::CoreObject> createCore() const;

		IndexBufferProperties mProperties;
		GpuBufferUsage mUsage;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Core thread specific implementation of an bs::IndexBuffer. */
	class BS_CORE_EXPORT IndexBuffer : public CoreObject, public HardwareBuffer
	{
	public:
		IndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);
		virtual ~IndexBuffer();

		/**	Returns information about the index buffer. */
		const IndexBufferProperties& getProperties() const { return mProperties; }

		/** @copydoc HardwareBuffer::readData */
		void readData(UINT32 offset, UINT32 length, void* dest, UINT32 deviceIdx = 0, UINT32 queueIdx = 0) override;

		/** @copydoc HardwareBuffer::writeData */
		void writeData(UINT32 offset, UINT32 length, const void* source, 
			BufferWriteType writeFlags = BWT_NORMAL, UINT32 queueIdx = 0) override;

		/** @copydoc HardwareBuffer::copyData */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, 
			bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

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
		SPtr<GpuBuffer> getLoadStore(GpuBufferType type, GpuBufferFormat format, UINT32 elementSize = 0);

		/** @copydoc HardwareBufferManager::createIndexBuffer */
		static SPtr<IndexBuffer> create(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		friend class HardwareBufferManager;

		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		IndexBufferProperties mProperties;

		HardwareBuffer* mBuffer = nullptr;
		SPtr<HardwareBuffer> mSharedBuffer;
		Vector<SPtr<GpuBuffer>> mLoadStoreViews;

		typedef void(*Deleter)(HardwareBuffer*);
		Deleter mBufferDeleter = nullptr;
	};

	/** @} */
		}
}