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

	/** Descriptor structure used for initialization of a VertexBuffer. */
	struct VERTEX_BUFFER_DESC
	{
		UINT32 vertexSize; /**< Size of a single vertex in the buffer, in bytes. */
		UINT32 numVerts; /**< Number of vertices the buffer can hold. */
		GpuBufferUsage usage = GBU_STATIC; /**< Usage that tells the hardware how will be buffer be used. */
		bool streamOut = false; /**< If true the buffer will be usable for streaming out data from the GPU. */
	};

	/** Contains information about a vertex buffer buffer. */
	class BS_CORE_EXPORT VertexBufferProperties
	{
	public:
		VertexBufferProperties(UINT32 numVertices, UINT32 vertexSize);

		/**	Gets the size in bytes of a single vertex in this buffer. */
		UINT32 getVertexSize() const { return mVertexSize; }

		/**	Get the number of vertices in this buffer. */
		UINT32 getNumVertices() const { return mNumVertices; }

	protected:
		friend class VertexBuffer;
		friend class ct::VertexBuffer;

		UINT32 mNumVertices;
		UINT32 mVertexSize;
	};

	/**	Specialization of a hardware buffer used for holding vertex data. */
	class BS_CORE_EXPORT VertexBuffer : public CoreObject
	{
	public:
		virtual ~VertexBuffer() = default;

		/**
		 * Retrieves a core implementation of a vertex buffer usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::VertexBuffer> getCore() const;

		/** @copydoc HardwareBufferManager::createVertexBuffer */
		static SPtr<VertexBuffer> create(const VERTEX_BUFFER_DESC& desc);

		static const int MAX_SEMANTIC_IDX = 8;
	protected:
		friend class HardwareBufferManager;

		VertexBuffer(const VERTEX_BUFFER_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		VertexBufferProperties mProperties;
		GpuBufferUsage mUsage;
		bool mStreamOut;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Core thread specific implementation of a bs::VertexBuffer. */
	class BS_CORE_EXPORT VertexBuffer : public CoreObject, public HardwareBuffer
	{
	public:
		VertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);
		virtual ~VertexBuffer();

		/**	Returns information about the vertex buffer. */
		const VertexBufferProperties& getProperties() const { return mProperties; }

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

		/** @copydoc HardwareBufferManager::createVertexBuffer */
		static SPtr<VertexBuffer> create(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected: 
		friend class HardwareBufferManager;

		/** @copydoc HardwareBuffer::map */
		void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 deviceIdx, UINT32 queueIdx) override;

		/** @copydoc HardwareBuffer::unmap */
		void unmap() override;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		VertexBufferProperties mProperties;

		HardwareBuffer* mBuffer = nullptr;
		SPtr<HardwareBuffer> mSharedBuffer;
		Vector<SPtr<GpuBuffer>> mLoadStoreViews;

		typedef void(*Deleter)(HardwareBuffer*);
		Deleter mBufferDeleter = nullptr;
	};

	/** @} */
		}
}