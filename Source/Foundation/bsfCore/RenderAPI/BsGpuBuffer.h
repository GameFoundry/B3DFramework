//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "CoreThread/BsCoreObjectCore.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**	Determines in what way will a GpuBuffer be used in. */
	enum class GpuBufferType
	{
		Vertex, /**< Contains mesh vertices and associated properties. */
		Index, /**< Contains mesh indices that determine which vertices form a triangle. */
		Uniform, /**< Contains GPU program parameters. */
		SimpleStorage, /**< Contains arbitrary data, formatted as an array of primitive types using one of the supported buffer formats. */
		StructuredStorage /**< Contains arbitrary data, formatted as an array of structures. Structure layout is up to the user. */
	};

	/** Flags that determine how a GpuBuffer behaves. */
	enum class GpuBufferFlag
	{
		/**
		 * Ensures the buffer is placed into memory on the GPU device. This allows the GPU to access the buffer quickly, but makes updating the buffer slower.
		 * CPU cannot read or write to this buffer directly, and a staging buffer must be used (it will be created internally for you, or you can use an explicit
		 * buffer via StoreOnCPU).
		 *
		 * Generally you wish to use this if your buffer is immutable or is not updated often from the CPU. If your buffer is updating every frame then
		 * PlaceOnCPUWithGPUAccess could be more efficient. Mutually exclusive with StoreOnCPUWithGPUAccess and StoreOnCPU.
		 */
		StoreOnGPU = 1 << 0,

		/**
		 * Places the buffer into CPU memory accessible to the GPU. This means the buffer is faster to update from the CPU (and may be updated without a
		 * staging buffer), but it's slower to access by the GPU as the memory access happens through the PCI Express bus. One exception is if your GPU is
		 * integrated on the CPU die, then this memory can be accessed directly by the GPU. Mutually exclusive with StoreOnGPU and StoreOnCPU.
		 */
		StoreOnCPUWithGPUAccess = 1 << 1,

		/**
		 * Places the buffer into CPU memory that cannot be accessed by the GPU. You can use this for staging buffers that receive and send data from/to a buffer on the
		 * GPU via the Copy operation. Mutually exclusive with StoreOnGPU and StoreOnCPUWithGPUAccess.
		 */
		StoreOnCPU = 1 << 2,

		/**
		 * Ensures that the GPU can perform write operations in the buffer. Generally this is used for buffers used in compute operations. StoreOnGPU memory
		 * flag must be used.
		 */
		AllowWritesOnTheGPU = 1 << 2,

		/** If set, a buffer will maintain a separate CPU-only buffer into which you may write via WriteCached(). Writes can then be sent to the GPU all at once via a FlushToGPU() call. */
		AllowWriteCachingOnCPU = 1 << 3,
	};

	using GpuBufferFlags = Flags<GpuBufferFlag>;
	B3D_FLAGS_OPERATORS(GpuBufferFlag);

	/** Descriptor structure used for a GpuBuffer. */
	struct GpuBufferInformation
	{
		GpuBufferInformation()
			: Vertex() // Zero the largest member of the union
		{ }

		/** Describes an array of vertices and their properties. */
		struct VertexBufferInformation
		{
			u32 ElementSize = 0; /**< Size of a single vertex in the buffer, in bytes. */
			u32 Count = 0; /**< Number of vertices the buffer can hold. */
		};

		/** Describes an array of indices that let the renderer know how the vertices connect into triangles. */
		struct IndexBufferInformation
		{
			IndexType Type = IT_32BIT; /**< Index type, determines the size of a single index. */
			u32 Count = 0; /**< Number of indices can buffer can hold. */
		};

		/** Describes a buffer containing parameters used for controlling execution of a GPU program. */
		struct UniformBufferInformation
		{
			u32 Size = 0; /**< Total size of the uniform buffer. */
		};

		/** Describes a buffer containing of array of primitive types using a specific format. */
		struct SimpleStorageBufferInformation
		{
			u32 Count = 0; /**< Number of elements in the buffer. */

			/** Format of each element in the buffer. */
			GpuBufferFormat Format = BF_32X4F;
		};

		/** Describes a buffer containing of array of structures. */
		struct StructuredStorageBufferInformation
		{
			u32 Count = 0; /**< Number of elements in the buffer. */

			/**
			 * Size of each individual element in the buffer, in bytes. Only needed if using non-standard buffer. If using
			 * standard buffers element size is calculated from format and this must be zero.
			 */
			u32 ElementSize = 0;
		};

		union
		{
			VertexBufferInformation Vertex;
			IndexBufferInformation Index;
			UniformBufferInformation Uniform;
			SimpleStorageBufferInformation SimpleStorage;
			StructuredStorageBufferInformation StructuredStorage;
		};

		GpuBufferType Type = GpuBufferType::Vertex; /**< Controls at which parts of the GPU pipeline is the buffer intended to be primarily used in. */
		GpuBufferFlags Flags = GpuBufferFlag::StoreOnGPU; /**< Flags that control the behavior of the buffer. */
	};

	/** Descriptor structure used for initialization of a GpuBuffer. */
	struct GpuBufferCreateInformation : GpuBufferInformation
	{
		GpuBufferCreateInformation() = default;
		GpuBufferCreateInformation(const GpuBufferInformation& other)
			:GpuBufferInformation(other)
		{ }

		static GpuBufferCreateInformation CreateVertex(u32 elementSize, u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::Vertex;
			output.Flags = flags;
			output.Vertex.ElementSize = elementSize;
			output.Vertex.Count = count;

			return output;
		}

		static GpuBufferCreateInformation CreateIndex(IndexType indexType, u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::Index;
			output.Flags = flags;
			output.Index.Type = indexType;
			output.Index.Count = count;

			return output;
		}

		static GpuBufferCreateInformation CreateUniform(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::Uniform;
			output.Flags = flags;
			output.Uniform.Size = size;

			return output;
		}

		static GpuBufferCreateInformation CreateSimpleStorage(GpuBufferFormat format, u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::SimpleStorage;
			output.Flags = flags;
			output.SimpleStorage.Format = format;
			output.SimpleStorage.Count = count;

			return output;
		}

		static GpuBufferCreateInformation CreateStructuredStorage(u32 elementSize, u32 count, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::StructuredStorage;
			output.Flags = flags;
			output.StructuredStorage.ElementSize = elementSize;
			output.StructuredStorage.Count = count;

			return output;
		}
	};

	/** Defines a buffer that can be used for operations on the GPU. */
	class B3D_CORE_EXPORT GpuBuffer : public CoreObject
	{
	public:
		virtual ~GpuBuffer() {}

		/** Returns information describing the buffer. */
		const GpuBufferInformation& GetInformation() const { return mInformation; }

		/** Retrieves a core implementation of the buffer usable only from the core thread. */
		SPtr<ct::GpuBuffer> GetCore() const;

		/** Creates a new buffer. */
		static SPtr<GpuBuffer> Create(const GpuBufferCreateInformation& createInformation);

	protected:
		GpuBuffer(const GpuBufferCreateInformation& createInformation);

		SPtr<ct::CoreObject> CreateCore() const override;

		GpuBufferInformation mInformation;
	};
}

namespace bs::ct
{
	/** Defines a buffer that can be used for operations on the GPU. */
	class B3D_CORE_EXPORT GpuBuffer : public CoreObject
	{
	public:
		virtual ~GpuBuffer();

		/** Assigns an name to the buffer, primarily used for easier debugging. */
		virtual void SetName(const StringView& name) { mName = name; }

		/**
		 * Locks a portion of the buffer and returns pointer to the locked area. You must call Unlock() when done.
		 *
		 * @param	offset		Offset in bytes from which to lock the buffer.
		 * @param	length		Length of the area you want to lock, in bytes.
		 * @param	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do
		 *						anything he hasn't requested (for example don't try to read from the buffer unless you
		 *						requested it here).
		 * @param	deviceIdx	Index of the device whose memory to map. If the buffer doesn't exist on this device,
		 *						the method returns null.
		 * @param	queueIdx	Device queue to perform any read/write operations on. See @ref queuesDoc.
		 */
		virtual void* Lock(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx = 0, u32 queueIdx = 0)
		{
			B3D_ASSERT(!IsLocked() && "Cannot lock this buffer, it is already locked!");
			void* ret = Map(offset, length, options, deviceIdx, queueIdx);
			mIsLocked = true;

			return ret;
		}

		/**
		 * Locks the entire buffer and returns pointer to the locked area. You must call Unlock() when done.
		 *
		 * @param	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do
		 *						anything he hasn't requested (for example don't try to read from the buffer unless you
		 *						requested it here).
		 * @param	deviceIdx	Index of the device whose memory to map. If the buffer doesn't exist on this device,
		 *						the method returns null.
		 * @param	queueIdx	Device queue to perform any read/write operations on. See @ref queuesDoc.
		 */
		void* Lock(GpuLockOptions options, u32 deviceIdx = 0, u32 queueIdx = 0)
		{
			return this->Lock(0, mSize, options, deviceIdx, queueIdx);
		}

		/**	Releases the lock on this buffer. */
		virtual void Unlock()
		{
			B3D_ASSERT(IsLocked() && "Cannot unlock this buffer, it is not locked!");

			Unmap();
			mIsLocked = false;
		}

		/**
		 * Reads data from a portion of the buffer and copies it to the destination buffer. Caller must ensure destination
		 * buffer is large enough.
		 *
		 * @param	offset			Offset in bytes from which to copy the data.
		 * @param	length			Length of the area you want to copy, in bytes.
		 * @param	destination		Destination buffer large enough to store the read data. Data is written from the start
		 *							of the buffer (@p offset is only applied to the source).
		 * @param	deviceIdx		Index of the device whose memory to read. If the buffer doesn't exist on this device,
		 *							no data will be read.
		 * @param	queueIdx		Device queue to perform the read operation on. See @ref queuesDoc.
		 */
		virtual void ReadData(u32 offset, u32 length, void* destination, u32 deviceIdx = 0, u32 queueIdx = 0) = 0;

		/**
		 * Writes data into a portion of the buffer from the source memory.
		 *
		 * @param	offset		Offset in bytes from which to copy the data.
		 * @param	length		Length of the area you want to copy, in bytes.
		 * @param	source		Source buffer containing the data to write. Data is read from the start of the buffer
		 *						(@p offset is only applied to the destination).
		 * @param	writeFlags	Optional write flags that may affect performance.
		 * @param	queueIdx	Device queue to perform the write operation on. See @ref queuesDoc.
		 */
		virtual void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, u32 queueIdx = 0) = 0;

		/**
		 * Copies data from a specific portion of the source buffer into a specific portion of this buffer.
		 *
		 * @param[in]	srcBuffer			Buffer to copy from.
		 * @param[in]	srcOffset			Offset into the source buffer to start copying from, in bytes.
		 * @param[in]	dstOffset			Offset into this buffer to start copying to, in bytes.
		 * @param[in]	length				Size of the data to copy, in bytes.
		 * @param[in]	discardWholeBuffer	Specify true if the data in the current buffer can be entirely discarded. This
		 *									may improve performance.
		 * @param[in]	commandBuffer		Command buffer to queue the copy operation on. If null, main command buffer is
		 *									used.
		 */
		virtual void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<ct::CommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Copy data from the provided buffer into this buffer. If buffers are not the same size, smaller size will be used.
		 *
		 * @param	source			Hardware buffer to copy from.
		 * @param	commandBuffer	Command buffer to queue the copy operation on. If null, main command buffer is used.
		 */
		virtual void CopyData(GpuBuffer& source, const SPtr<ct::CommandBuffer>& commandBuffer = nullptr)
		{
			const u32 sizeToCopy = std::min(GetSize(), source.GetSize());
			CopyData(source, 0, 0, sizeToCopy, true, commandBuffer);
		}

		/**
		 * Writes the data into the CPU cached buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. In order
		 * for the data to actually reach the underlying buffer you must call FlushCache().
		 */
		virtual void WriteToCache(u32 offset, u32 length, const void* source);

		/**
		 * Clears the specified area of the cache. Buffer must have been created with AllowWriteCachingOnCPU flag. In order
		 * for the data to actually reach the underlying buffer you must call FlushCache().
		 */
		virtual void ZeroOutCache(u32 offset, u32 length);

		/** Flushes the cached to the underlying buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. */
		virtual void FlushCache();

		/** Returns the size of this buffer in bytes. */
		u32 GetSize() const { return mSize; }

		/**	Returns whether or not this buffer is currently locked. */
		bool IsLocked() const { return mIsLocked; }

	protected:
		friend class bs::GpuDevice;

		/** Constructs a new GPU buffer. */
		GpuBuffer(const GpuBufferCreateInformation& createInformation);

		/** @copydoc Lock */
		virtual void* Map(u32 offset, u32 length, GpuLockOptions options, u32 deviceIdx, u32 queueIdx) { return nullptr; }

		/** @copydoc Unlock */
		virtual void Unmap() {}

	protected:
		GpuBufferType mType = GpuBufferType::SimpleStorage;
		String mName;
		u32 mSize;
		GpuBufferFlags mBufferFlags;
		u8* mCache = nullptr;
		bool mIsCacheDirty = false;

		bool mIsLocked = false;
	};
} // namespace bs::ct

/** @} */
