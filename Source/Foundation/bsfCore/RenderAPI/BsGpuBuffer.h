//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreObject/BsCoreObject.h"
#include "CoreObject/BsRenderProxy.h"

namespace bs
{
	class GpuQueue;
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
		StructuredStorage, /**< Contains arbitrary data, formatted as an array of structures. Structure layout is up to the user. */
		StagingWrite, /**< Special type of CPU writeable buffer type used only as a source of copy operations. Ignores the store flags and is always accessible by CPU only. */
		StagingRead, /**< Special type of CPU readable buffer type used only as a destination of copy operations. Ignores the store flags and is always accessible by CPU only. */
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

		/**
		 * Number of sub-allocated buffers to create. Internally this will allocate memory for this many buffers, which can be bound by providing a dynamic offset when
		 * binding the buffer on GpuCommandBuffer. Binding buffers this way is more efficient than creating separate GpuBuffer for each entry.
		 */
		u32 SuballocationCount = 1;
	};

	/** Descriptor structure used for initialization of a GpuBuffer. */
	struct GpuBufferCreateInformation : GpuBufferInformation
	{
		GpuBufferCreateInformation() = default;
		GpuBufferCreateInformation(const GpuBufferInformation& other)
			:GpuBufferInformation(other)
		{ }

		/**
		 * Builds a structure that can be used for creating a GpuBuffer containing vertex information.
		 *
		 * @param elementSize	Size of a single vertex in the buffer.
		 * @param elementCount	Number of vertices in the buffer.
		 * @param flags			Flags that control how is buffer accessed/used.
		 * @return				Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateVertex(u32 elementSize, u32 elementCount, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::Vertex;
			output.Flags = flags;
			output.Vertex.ElementSize = elementSize;
			output.Vertex.Count = elementCount;

			return output;
		}

		/**
		 * Builds a structure that can be used for creating a GpuBuffer containing triangle indices.
		 *
		 * @param indexType		Type of index contained in the buffer.
		 * @param indexCount	Number of indices in the buffer.
		 * @param flags			Flags that control how is buffer accessed/used.
		 * @return				Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateIndex(IndexType indexType, u32 indexCount, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::Index;
			output.Flags = flags;
			output.Index.Type = indexType;
			output.Index.Count = indexCount;

			return output;
		}

		/**
		 * Builds a structure that can be used for creating a GpuBuffer containing uniform parameters.
		 *
		 * @param size					Size of the uniform buffer.
		 * @param flags					Flags that control how is buffer accessed/used.
		 * @param suballocationCount	Number of buffers of requested size to create. In case you need multiple buffers of the same size this is more efficient
		 *								than creating a separate GpuBuffer for each. Sub-allocated buffers can be bound for rendering by using the dynamic
		 *								offset functionality provided on GpuCommandBuffer.
		 * @return						Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateUniform(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowWriteCachingOnCPU, u32 suballocationCount = 1)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::Uniform;
			output.Flags = flags;
			output.SuballocationCount = suballocationCount;
			output.Uniform.Size = size;

			return output;
		}

		/**
		 * Builds a structure that can be used for creating a GpuBuffer that may be bound for arbitrary reads or writes, containing simple (primitive) elements.
		 *
		 * @param format		Format of elements in the buffer.
		 * @param elementCount	Number of elements in the buffer.
		 * @param flags			Flags that control how is buffer accessed/used.
		 * @return				Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateSimpleStorage(GpuBufferFormat format, u32 elementCount, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::SimpleStorage;
			output.Flags = flags;
			output.SimpleStorage.Format = format;
			output.SimpleStorage.Count = elementCount;

			return output;
		}

		/**
		 * Builds a structure that can be used for creating a GpuBuffer that may be bound for arbitrary reads or writes, containing arbitrary data.
		 *
		 * @param elementSize	Size of a single element in the buffer.
		 * @param elementCount	Number of elements in the buffer.
		 * @param flags			Flags that control how is buffer accessed/used.
		 * @return				Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateStructuredStorage(u32 elementSize, u32 elementCount, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::StructuredStorage;
			output.Flags = flags;
			output.StructuredStorage.ElementSize = elementSize;
			output.StructuredStorage.Count = elementCount;

			return output;
		}
	};

	/** Defines a buffer that can be used for operations on the GPU. */
	class B3D_CORE_EXPORT GpuBuffer : public CoreObject
	{
	public:
		virtual ~GpuBuffer() = default;

		/** Returns the total size of this buffer in bytes. */
		u32 GetTotalSize() const { return mTotalSize; }

		/**
		 * In case this buffer is containing multiple sub-allocated buffers, returns the size of one sub-allocation. Note this size might be different than requested
		 * during creation as platform alignment requirements for suballocation must be respected.
		 *
		 * If the buffer doesn't have any suballocated buffers, this is equivalent to GetTotalSize().
		 */
		u32 GetSuballocationSize() const { return mSuballocationSize; }

		/** Returns information describing the buffer. */
		const GpuBufferInformation& GetInformation() const { return mInformation; }

		/**
		 * Writes the data into the CPU cached buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. Data will be synced
		 * with the render proxy on the next sync call.
		 */
		void WriteCached(u32 offset, u32 length, const void* source);

		/**
		 * Same as WriteCached(), but takes care of respecting the padding/alignment requirements of the provided type. (e.g. a 3x3 matrix will be padded with 4 bytes in each row).
		 * @p source must contain at least as many bytes as the size provided in @p typeInformation. Returns the total number of written bytes, including the padding.
		 */
		u32 WriteCachedType(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source);

		/**
		 * Clears the specified area of the cache. Buffer must have been created with AllowWriteCachingOnCPU flag. Data will be synced
		 * with the render proxy on the next sync call.
		 */
		void ZeroOutCached(u32 offset, u32 length);

		/**
		 * Reads the data from the cached buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. Note the cached data
		 * only includes writes done by WriteData() and ZeroOutData() calls. It will not account for writes done explicitly on the render
		 * thread, or on the GPU.
		 */
		void ReadCached(u32 offset, u32 length, void* destination);

		/** Creates a new buffer. */
		static SPtr<GpuBuffer> Create(const GpuBufferCreateInformation& createInformation);

		/** Returns the size of a single element in the buffer, of the provided format, in bytes. */
		static u32 GetFormatSize(GpuBufferFormat format);

		/** Returns teh size of a single index buffer element of the specified type, in bytes. */
		static u32 GetIndexSize(IndexType type) { return type == IT_32BIT ? 4 : 2; }

		/** Calculates the size of a buffer described by the provided information, in bytes. */
		static u32 CalculateTotalBufferSize(const GpuBufferInformation& information, const SPtr<GpuDevice>& gpuDevice);

		/**
		 * Calculates the distance between two buffers, in case the buffer contains sub-allocated buffers. This is guaranteed to be at
		 * least the request size of a single sub-allocated buffer, but may be larger due to alignment requirements.
		 */
		static u32 CalculateSuballocatedBufferSize(const GpuBufferInformation& information, const SPtr<GpuDevice>& gpuDevice);
		static u32 CalculateSuballocatedBufferSize(const GpuBufferInformation& information, const GpuDevice& gpuDevice);

	protected:
		struct SyncPacket;
		friend class ct::GpuBuffer;

		GpuBuffer(const GpuBufferCreateInformation& createInformation);

		void Initialize() override;
		void Destroy() override;
		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		GpuBufferInformation mInformation;
		u32 mSuballocationSize = 0;
		u32 mTotalSize = 0;
		u8* mCache = nullptr;
	};
}

namespace bs::ct
{
	/** Defines a buffer that can be used for operations on the GPU. */
	class B3D_CORE_EXPORT GpuBuffer : public RenderProxy
	{
	public:
		virtual ~GpuBuffer();

		/** Returns information describing the buffer. */
		const GpuBufferInformation& GetInformation() const { return mInformation; }

		/** Assigns an name to the buffer, primarily used for easier debugging. */
		virtual void SetName(const StringView& name) { mName = name; }

		/**
		 * Locks a portion of the buffer and returns pointer to the locked area. You must call Unlock() when done. This method
		 * is only available on buffers that are accessible by the CPU (StoreOnCPU or StoreOnCPUWithGPUAcess flags).
		 *
		 * @param	offset		Offset in bytes from which to lock the buffer.
		 * @param	length		Length of the area you want to lock, in bytes.
		 * @param	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do
		 *						anything he hasn't requested (for example don't try to read from the buffer unless you
		 *						requested it here).
		 */
		virtual void* Lock(u32 offset, u32 length, GpuLockOptions options)
		{
			B3D_ASSERT(!IsLocked() && "Cannot lock this buffer, it is already locked!");
			void* ret = Map(offset, length, options);
			mIsLocked = true;

			return ret;
		}

		/**
		 * Locks the entire buffer and returns pointer to the locked area. You must call Unlock() when done.
		 *
		 * @param	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do
		 *						anything he hasn't requested (for example don't try to read from the buffer unless you
		 *						requested it here).
		 */
		void* Lock(GpuLockOptions options)
		{
			return this->Lock(0, mTotalSize, options);
		}

		/**	Releases the lock on this buffer. */
		virtual void Unlock()
		{
			B3D_ASSERT(IsLocked() && "Cannot unlock this buffer, it is not locked!");

			Unmap();
			mIsLocked = false;
		}

		/**
		 * Reads data from a portion of the buffer and copies it to the destination buffer. Caller must ensure destination buffer is large enough.
		 *
		 * @note	If the buffer cannot be directly mapped by the CPU (i.e. doesn't have the StoreOnCPU or StoreOnCPUWithGPUAccess flags) this will
		 *			internally create a staging buffer, on which the contents will be copied before being read by the CPU, using an internal command buffer.
		 * @note	If the buffer is currently being used by the GPU, this method will block until the GPU is done executing, so you should call this
		 *			method in very rare circumstances.
		 *
		 * @param	offset			Offset in bytes from which to copy the data.
		 * @param	length			Length of the area you want to copy, in bytes.
		 * @param	destination		Destination buffer large enough to store the read data. Data is written from the start of the buffer (@p offset is only applied to the source).
		 * @param	gpuQueue		GPU queue on which to perform the read. If not specified the default queue will be used.
		 */
		virtual void ReadData(u32 offset, u32 length, void* destination, const SPtr<GpuQueue>& gpuQueue = nullptr) = 0;

		/**
		 * Writes data into a portion of the buffer from the source memory.
		 *
		 * @note	If the buffer cannot be directly mapped by the CPU (i.e. doesn't have the StoreOnCPU or StoreOnCPUWithGPUAccess flags) this will
		 *			internally create a staging buffer, on which the contents will be copied before being written by the GPU, using the provided command buffer).
		 *
		 * @param	offset			Offset in bytes from which to copy the data.
		 * @param	length			Length of the area you want to copy, in bytes.
		 * @param	source			Source buffer containing the data to write. Data is read from the start of the buffer
		 *							(@p offset is only applied to the destination).
		 * @param	writeFlags		Optional write flags that may affect performance.
		 * @param	commandBuffer	Command buffer on which to encode the staging buffer copy, in case the buffer is not directly readable. If not provided
		 *							the operation will be queued on an internal command buffer that will be submitted before any regular command
		 *							buffer submission.
		 */
		virtual void WriteData(u32 offset, u32 length, const void* source, BufferWriteType writeFlags = BWT_NORMAL, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr) = 0;

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
		virtual void CopyData(GpuBuffer& srcBuffer, u32 srcOffset, u32 dstOffset, u32 length, bool discardWholeBuffer = false, const SPtr<ct::GpuCommandBuffer>& commandBuffer = nullptr) = 0;

		/**
		 * Copy data from the provided buffer into this buffer. If buffers are not the same size, smaller size will be used.
		 *
		 * @param	source			Hardware buffer to copy from.
		 * @param	commandBuffer	Command buffer to queue the copy operation on. If null, main command buffer is used.
		 */
		virtual void CopyData(GpuBuffer& source, const SPtr<ct::GpuCommandBuffer>& commandBuffer = nullptr)
		{
			const u32 sizeToCopy = std::min(GetTotalSize(), source.GetTotalSize());
			CopyData(source, 0, 0, sizeToCopy, true, commandBuffer);
		}

		/**
		 * Writes the data into the CPU cached buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. In order
		 * for the data to actually reach the underlying buffer you must call FlushCache().
		 */
		virtual void WriteCached(u32 offset, u32 length, const void* source);

		/**
		 * Same as WriteCached(), but takes care of respecting the padding/alignment requirements of the provided type. (e.g. a 3x3 matrix will be padded with 4 bytes in each row).
		 * @p source must contain at least as many bytes as the size provided in @p typeInformation. Returns the total number of written bytes, including the padding.
		 */
		u32 WriteCachedType(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source);

		/**
		 * Clears the specified area of the cache. Buffer must have been created with AllowWriteCachingOnCPU flag. In order
		 * for the data to actually reach the underlying buffer you must call FlushCache().
		 */
		virtual void ZeroOutCached(u32 offset, u32 length);

		/**
		 * Reads the data from the cached buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. Note the cached data
		 * only includes writes done by WriteToCache() and ZeroOutCache() calls. It will not account for writes done on the GPU or writes
		 * that skip the cache.
		 */
		void ReadCached(u32 offset, u32 length, void* destination);

		/** Flushes the cached to the underlying buffer. Buffer must have been created with AllowWriteCachingOnCPU flag. */
		virtual void FlushCache();

		/** Returns the total size of this buffer in bytes. */
		u32 GetTotalSize() const { return mTotalSize; }

		/**
		 * In case this buffer is containing multiple sub-allocated buffers, returns the size of one sub-allocation. Note this size might be different than requested
		 * during creation as platform alignment requirements for suballocation must be respected.
		 *
		 * If the buffer doesn't have any suballocated buffers, this is equivalent to GetTotalSize().
		 */
		u32 GetSuballocationSize() const { return mSuballocationSize; }

		/**	Returns whether or not this buffer is currently locked. */
		bool IsLocked() const { return mIsLocked; }

	protected:
		friend class bs::GpuDevice;
		friend class bs::GpuBuffer;

		/** Constructs a new GPU buffer. */
		GpuBuffer(const GpuBufferCreateInformation& createInformation, u32 suballocationSize);

		void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

		/** @copydoc Lock */
		virtual void* Map(u32 offset, u32 length, GpuLockOptions options) { return nullptr; }

		/** @copydoc Unlock */
		virtual void Unmap() {}

	protected:
		GpuBufferInformation mInformation;
		String mName;
		u32 mSuballocationSize = 0;
		u32 mTotalSize = 0;
		u8* mCache = nullptr;
		bool mIsCacheDirty = false;

		bool mIsLocked = false;
	};
} // namespace bs::ct

/** @} */
