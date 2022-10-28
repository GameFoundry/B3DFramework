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
		u32 VertexSize; /**< Size of a single vertex in the buffer, in bytes. */
		u32 NumVerts; /**< Number of vertices the buffer can hold. */
		GpuBufferUsage Usage = GBU_STATIC; /**< Usage that tells the hardware how will be buffer be used. */
		bool StreamOut = false; /**< If true the buffer will be usable for streaming out data from the GPU. */
	};

	/** Contains information about a vertex buffer buffer. */
	class BS_CORE_EXPORT VertexBufferProperties
	{
	public:
		VertexBufferProperties(u32 numVertices, u32 vertexSize);

		/**	Gets the size in bytes of a single vertex in this buffer. */
		u32 GetVertexSize() const { return mVertexSize; }

		/**	Get the number of vertices in this buffer. */
		u32 GetNumVertices() const { return mNumVertices; }

	protected:
		friend class VertexBuffer;
		friend class ct::VertexBuffer;

		u32 mNumVertices;
		u32 mVertexSize;
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
		SPtr<ct::VertexBuffer> GetCore() const;

		/** @copydoc HardwareBufferManager::CreateVertexBuffer */
		static SPtr<VertexBuffer> Create(const VERTEX_BUFFER_DESC& desc);

		static const int kMaxSemanticIdx = 8;

	protected:
		friend class HardwareBufferManager;

		VertexBuffer(const VERTEX_BUFFER_DESC& desc);

		SPtr<ct::CoreObject> CreateCore() const override;

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
			const VertexBufferProperties& GetProperties() const { return mProperties; }

			void ReadData(u32 offset, u32 length, void* dest, u32 deviceIdx = 0, u32 queueIdx = 0) override;
			void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) override;
			void CopyData(HardwareBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<CommandBuffer>& commandBuffer = nullptr) override;

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
			SPtr<GpuBuffer> GetLoadStore(GpuBufferType type, GpuBufferFormat format, u32 elementSize = 0);

			/** @copydoc HardwareBufferManager::CreateVertexBuffer */
			static SPtr<VertexBuffer> Create(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class HardwareBufferManager;

			void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) override;
			void Unmap() override;
			void Initialize() override;

			VertexBufferProperties mProperties;

			HardwareBuffer* mBuffer = nullptr;
			SPtr<HardwareBuffer> mSharedBuffer;
			Vector<SPtr<GpuBuffer>> mLoadStoreViews;

			typedef void (*Deleter)(HardwareBuffer*);
			Deleter mBufferDeleter = nullptr;
		};

		/** @} */
	} // namespace ct
} // namespace bs
