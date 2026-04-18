//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuDevice.h"
#include "CoreObject/B3DCoreObject.h"
#include "CoreObject/B3DRenderProxy.h"

namespace b3d
{
	class GpuQueue;

	/** @addtogroup GpuBackend
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
		AllowUnorderedAccessOnTheGPU = 1 << 2
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

		/** Describes a buffer used for copying from or to a buffer stores in non-CPU accessible memory. */
		struct StagingBufferInformation
		{
			u32 Size = 0; /**< Total size of the staging buffer, in bytes. */
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
			StagingBufferInformation Staging;
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
		static GpuBufferCreateInformation CreateUniform(u32 size, GpuBufferFlags flags = GpuBufferFlag::StoreOnCPUWithGPUAccess, u32 suballocationCount = 1)
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

		/**
		 * Builds a structure that can be used for creating a GpuBuffer that can be written to by the CPU and used as a source in copy operations.
		 *
		 * @param size		Size of the buffer.
		 * @return			Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateStagingWrite(u32 size)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::StagingWrite;
			output.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
			output.Staging.Size = size;

			return output;
		}

		/**
		 * Builds a structure that can be used for creating a GpuBuffer that can be read by the CPU and used as a destination in copy operations.
		 *
		 * @param size		Size of the buffer.
		 * @return			Structure that can be used for creating the buffer.
		 */
		static GpuBufferCreateInformation CreateStagingRead(u32 size)
		{
			GpuBufferCreateInformation output;
			output.Type = GpuBufferType::StagingWrite;
			output.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
			output.Staging.Size = size;

			return output;
		}
	};

	/** Options for GPU buffer mapping operations. */
	enum class GpuMapOption
	{
		Read = 1 << 0,      /**< Map for reading (will invalidate before mapping). */
		Write = 1 << 1,     /**< Map for writing (will flush after unmapping). */
		NoOverwrite = 1 << 2, /**< Suppresses validation warnings if the caller writes to a buffer region already bound to a command buffer. Indicates that the caller knows what he is doing. */
		ReadWrite = Read | Write  /**< Map for both reading and writing. */
	};

	using GpuMapOptions = Flags<GpuMapOption>;
	B3D_FLAGS_OPERATORS(GpuMapOption);

	// Forward declaration for TGpuBufferSuballocation::Map return type
	template <bool IsRenderProxy>
	class TGpuBufferMappedScope;

	/** Represents a single sub-allocation within a specific GpuBuffer. Templated to support both main thread and render thread variants. */
	template <bool IsRenderProxy>
	class TGpuBufferSuballocation
	{
	public:
		using GpuBufferType = CoreVariantType<b3d::GpuBuffer, IsRenderProxy>;

		TGpuBufferSuballocation() = default;
		explicit TGpuBufferSuballocation(const SPtr<GpuBufferType>& buffer, u32 suballocationOffset = 0)
			: mBuffer(buffer), mSuballocationOffset(suballocationOffset) {}

		/** Gets the underlying GPU buffer. */
		const SPtr<GpuBufferType>& GetBuffer() const { return mBuffer; }

		/** Gets the zero-based suballocation index within the buffer. Computed lazily from offset. */
		u32 GetSuballocationIndex() const { return mBuffer ? (mSuballocationOffset / mBuffer->GetSuballocationSize()) : 0; }

		/** Gets the byte offset from the start of the buffer for this suballocation. */
		u32 GetSuballocationOffset() const { return mSuballocationOffset; }

		/**
		 * Gets the size of this suballocation in bytes (aligned).
		 * May be larger than requested size during buffer creation due to alignment requirements.
		 */
		u32 GetSize() const;

		/** Checks if this is a valid suballocation. */
		bool IsValid() const { return mBuffer != nullptr; }

		/** Clears the suballocation, making it invalid. */
		void Reset() { mBuffer = nullptr; }

		/**
		 * Maps this suballocation for CPU access.
		 *
		 * @param options	Map options (typically GpuMapOption::Write).
		 * @return			RAII mapped region that auto-flushes on destruction.
		 */
		TGpuBufferMappedScope<IsRenderProxy> Map(GpuMapOptions options = GpuMapOption::Write) const;

	private:
		SPtr<GpuBufferType> mBuffer;
		u32 mSuballocationOffset = 0;
	};

	/**
	 * RAII wrapper returned by GpuBuffer::Map operation. Allows you to access the mapped memory, and automatically flushes the memory on destruction, if needed.
	 * Prevents the mapped buffer from being destructed while the scope is active. As a helper can also be constructed back into a TGpuBufferSuballocation referencing the mapped range.
	 * Templated to support both main thread and render thread variants.
	 */
	template <bool IsRenderProxy>
	class TGpuBufferMappedScope
	{
	public:
		using GpuSuballocationType = TGpuBufferSuballocation<IsRenderProxy>;
		using GpuBufferType = typename GpuSuballocationType::GpuBufferType;

		TGpuBufferMappedScope() = default;

		TGpuBufferMappedScope(void* mappedMemory, GpuSuballocationType suballocation, u32 size, GpuMapOptions options)
			: mMappedMemory(mappedMemory) , mSuballocation(std::move(suballocation)) , mSize(size) , mOptions(options)
		{}

		TGpuBufferMappedScope(TGpuBufferMappedScope&& other) noexcept
			: mMappedMemory(other.mMappedMemory), mSuballocation(std::move(other.mSuballocation)), mSize(other.mSize) , mOptions(other.mOptions)
		{
			other.mMappedMemory = nullptr;
			other.mSuballocation = GpuSuballocationType();
		}

		TGpuBufferMappedScope& operator=(TGpuBufferMappedScope&& other) noexcept
		{
			if(this != &other)
			{
				Unmap();
				mMappedMemory = std::exchange(other.mMappedMemory, nullptr);
				mSuballocation = std::move(other.mSuballocation);
				mSize = other.mSize;
				mOptions = other.mOptions;
				other.mSuballocation.Reset();
			}
			return *this;
		}

		TGpuBufferMappedScope(const TGpuBufferMappedScope&) = delete;
		TGpuBufferMappedScope& operator=(const TGpuBufferMappedScope&) = delete;

		~TGpuBufferMappedScope() { Unmap(); }

		/** Explicitly releases the mapping. Safe to call multiple times. Flushes if write mapping (render proxy) or marks dirty (main thread). */
		void Unmap();

		void* GetMappedMemory() const { return mMappedMemory; }
		const GpuSuballocationType& GetSuballocation() const { return mSuballocation; }
		const SPtr<GpuBufferType>& GetBuffer() const { return mSuballocation.GetBuffer(); }
		bool IsValid() const { return mMappedMemory != nullptr; }
		explicit operator bool() const { return IsValid(); }

		/** Implicit conversion to TGpuBufferSuballocation. */
		operator const GpuSuballocationType&() const { return mSuballocation; }

	private:
		void* mMappedMemory = nullptr;
		GpuSuballocationType mSuballocation;
		u32 mSize = 0;
		GpuMapOptions mOptions;
	};

	using GpuBufferSuballocation = TGpuBufferSuballocation<false>;
	using GpuBufferMappedScope = TGpuBufferMappedScope<false>;

	/** Defines a buffer that can be used for operations on the GPU. */
	class B3D_EXPORT GpuBuffer : public CoreObject
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

		/** Writes the data into the CPU cached buffer. Data will be synced with the render proxy on the next sync call. */
		void Write(u32 offset, u32 length, const void* source);

		/**
		 * Same as Write(), but takes care of respecting the padding/alignment requirements of the provided type. (e.g. a 3x3 matrix will be padded with 4 bytes in each row).
		 * @p source must contain at least as many bytes as the size provided in @p typeInformation. Returns the total number of written bytes, including the padding.
		 */
		u32 WriteTyped(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source);

		/** Clears the specified area of the cache. Data will be synced with the render proxy on the next sync call. */
		void ZeroOut(u32 offset, u32 length);

		/**
		 * Reads the data from the cached buffer. Note the cached data only includes writes done by the CPU.
		 * It will not account for writes done explicitly on the render thread or on the GPU.
		 */
		void Read(u32 offset, u32 length, void* destination);

		/**
		 * Maps the cache buffer and returns an RAII mapped region that automatically handles flush on destruction.
		 *
		 * @param offset   Offset in bytes from which to map.
		 * @param size     Size of the region to map, in bytes.
		 * @param options  Specifies read/write intent for the mapping.
		 * @return         RAII mapped region containing the mapped memory pointer.
		 */
		GpuBufferMappedScope Map(u32 offset, u32 size, GpuMapOptions options)
		{
			void* mappedMemory = mCache + offset;
			return GpuBufferMappedScope(mappedMemory, GpuBufferSuballocation(std::static_pointer_cast<GpuBuffer>(GetShared()), offset), size, options);
		}

		/**
		 * Maps the entire buffer and returns an RAII mapped region.
		 *
		 * @param options  Specifies read/write intent for the mapping.
		 * @return         RAII mapped region containing the mapped memory pointer.
		 */
		GpuBufferMappedScope Map(GpuMapOptions options)
		{
			return Map(0, mTotalSize, options);
		}

		/** Creates a new buffer. */
		static SPtr<GpuBuffer> Create(const GpuBufferCreateInformation& createInformation);

		/** Returns the size of a single element in the buffer, of the provided format, in bytes. */
		static u32 GetFormatSize(GpuBufferFormat format);

		/** Returns the size of a single index buffer element of the specified type, in bytes. */
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
		friend class render::GpuBuffer;
		template <bool IsRenderProxy> friend class TGpuBufferMappedScope;

		GpuBuffer(const GpuBufferCreateInformation& createInformation);

		void Initialize() override;
		void Destroy() override;
		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		GpuBufferInformation mInformation;
		u32 mSuballocationSize = 0;
		u32 mTotalSize = 0;
		u8* mCache = nullptr;
	};

	template <bool IsRenderProxy>
	u32 TGpuBufferSuballocation<IsRenderProxy>::GetSize() const
	{
		return mBuffer ? mBuffer->GetSuballocationSize() : 0;
	}

	template <bool IsRenderProxy>
	TGpuBufferMappedScope<IsRenderProxy> TGpuBufferSuballocation<IsRenderProxy>::Map(GpuMapOptions options) const
	{
		B3D_ASSERT(IsValid());

		return mBuffer->Map(mSuballocationOffset, GetSize(), options);
	}

	template <bool IsRenderProxy>
	void TGpuBufferMappedScope<IsRenderProxy>::Unmap()
	{
		if(mMappedMemory != nullptr && mSuballocation.IsValid())
		{
			if(mOptions.IsSet(GpuMapOption::Write))
			{
				if constexpr(IsRenderProxy)
					mSuballocation.GetBuffer()->Flush(mSuballocation.GetSuballocationOffset(), mSize);
				else
					mSuballocation.GetBuffer()->MarkRenderProxyDataDirty();
			}

			mMappedMemory = nullptr;
		}
	}
}

namespace b3d::render
{
	using GpuBufferSuballocation = TGpuBufferSuballocation<true>;
	using GpuBufferMappedScope = TGpuBufferMappedScope<true>;

	/** Defines a buffer that can be used for operations on the GPU. */
	class B3D_EXPORT GpuBuffer : public RenderProxy
	{
	public:
		virtual ~GpuBuffer();

		/** Returns information describing the buffer. */
		const GpuBufferInformation& GetInformation() const { return mInformation; }

		/** Assigns an name to the buffer, primarily used for easier debugging. */
		virtual void SetName(const StringView& name) { mName = name; }

		/** Returns the name of the buffer. Primarily used for debugging purposes. */
		const String& GetName() const { return mName; }

		/**
		 * Maps the buffer and returns an RAII mapped region that automatically handles flush on destruction.
		 *
		 * @param offset   Offset in bytes from which to map.
		 * @param size     Size of the region to map, in bytes.
		 * @param options  Specifies read/write intent for the mapping.
		 * @return         RAII mapped region containing the mapped memory pointer.
		 */
		GpuBufferMappedScope Map(u32 offset, u32 size, GpuMapOptions options)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			ValidateMap(offset, size, options);
#endif

			if(options.IsSet(GpuMapOption::Read))
				Invalidate(offset, size);

			void* mappedMemory = static_cast<u8*>(GetMappedMemory()) + offset;
			return GpuBufferMappedScope(mappedMemory, GpuBufferSuballocation(std::static_pointer_cast<GpuBuffer>(GetShared()), offset), size, options);
		}

		/**
		 * Maps the entire buffer and returns an RAII mapped region.
		 *
		 * @param options  Specifies read/write intent for the mapping.
		 * @return         RAII mapped region containing the mapped memory pointer.
		 */
		GpuBufferMappedScope Map(GpuMapOptions options)
		{
			return Map(0, mTotalSize, options);
		}

		/**
		 * Writes the data of the specified length into the buffer at the provided offset. @p source must contain at least @p length bytes.
		 *
		 * Buffer must support CPU writes. This mean it's either explicitly created with StoreOnCPUWithGPUAccess flag, or running on a GPU that supports CPU access.
		 * The latter usually means running on an integrated GPU with shared memory.
		 *
		 * After all writes are finished make sure to call Flush() to make the writes visible to the GPU.
		 */
		void Write(u32 offset, u32 length, const void* source);

		/**
		 * Writes the data into the buffer at the provided offset. Takes care of respecting the padding/alignment requirements of the provided type. (e.g. a 3x3 matrix will be padded with 4 bytes in each row).
		 * @p source must contain at least as many bytes as the size provided in @p typeInformation. Returns the total number of written bytes, including the padding.
		 *
		 * Buffer must support CPU writes. This mean it's either explicitly created with StoreOnCPUWithGPUAccess flag, or running on a GPU that supports CPU access. The latter usually means running on an
		 * integrated GPU with shared memory.
		 *
		 * After all writes are finished make sure to call Flush() to make the writes visible to the GPU.
		 */
		u32 WriteTyped(u32 offset, const GpuDataParameterTypeInformation& typeInformation, const void* source);

		/**
		 * Clears the specified area of the buffer.
		 *
		 * Buffer must support CPU writes. This mean it's either explicitly created with StoreOnCPUWithGPUAccess flag, or running on a GPU that supports CPU access.
		 * The latter usually means running on an integrated GPU with shared memory.
		 *
		 * After all writes are finished make sure to call Flush() to make the writes visible to the GPU.
		 */
		void ZeroOut(u32 offset, u32 length);

		/**
		 * Reads the data from the buffer at the provided offset. 
		 *
		 * Buffer must support CPU reads. This mean it's either explicitly created with StoreOnCPUWithGPUAccess flag, or running on a GPU that supports CPU access. The latter usually means running on an
		 * integrated GPU with shared memory.
		 *
		 * If GPU wrote to this buffer you must ensure to issue an execution barrier which ensures all GPU units finish writing to the buffer, a GPU memory barrier that makes sure it flushes it caches into memory, and then finally call
		 * Invalidate(), which forces CPU to fetch the data from the memory rather than its caches. All of this must be done before reading the data.
		 */
		void Read(u32 offset, u32 length, void* destination);

		/** Returns the total size of this buffer in bytes. */
		u32 GetTotalSize() const { return mTotalSize; }

		/**
		 * In case this buffer is containing multiple sub-allocated buffers, returns the size of one sub-allocation. Note this size might be different than requested
		 * during creation as platform alignment requirements for suballocation must be respected.
		 *
		 * If the buffer doesn't have any suballocated buffers, this is equivalent to GetTotalSize().
		 */
		u32 GetSuballocationSize() const { return mSuballocationSize; }

		/** Returns a pointer to persistently mapped memory, or nullptr if not mappable. */
		void* GetMappedMemory() const { return mMappedMemory; }

		/**
		 * Flushes CPU writes to make them visible to the GPU.
		 * Only relevant for non-coherent memory.
		 *
		 * @param offset  Offset from buffer start, in bytes.
		 * @param size    Size of region to flush, in bytes.
		 */
		virtual void Flush(u32 offset, u32 size) {}

		/**
		 * Invalidates GPU writes to make them visible to the CPU.
		 * Only relevant for non-coherent memory.
		 *
		 * @param offset  Offset from buffer start, in bytes.
		 * @param size    Size of region to invalidate, in bytes.
		 */
		virtual void Invalidate(u32 offset, u32 size) {}

		/** Gets the GPU device the buffer is created on. */
		GpuDevice& GetDevice() const { return static_cast<GpuDevice&>(mDevice); }

		/**
		 * Returns if the buffer is currently being used on the GPU, and if so on which queues is it scheduled. Allows the caller so synchronize command buffer
		 * execution after buffer is done being used.
		 */
		virtual GpuQueueMask GetUseMask(GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) = 0;

		/** Returns the amount of command buffers that the buffer is currently bound to. Note that this does not specify if the command buffer has been submitted for execution or not. */
		virtual u32 GetBoundCount() const = 0;

		/** Returns the amount of submitted command buffers that the buffer is bound to. */
		virtual u32 GetUseCount() const = 0;

#if B3D_BUILD_TYPE_DEVELOPMENT
		/** Checks if any suballocation overlapping the given byte range is bound. */
		virtual bool IsRangeBound(u32 offset, u32 size) const = 0;

		/** Checks if any suballocation overlapping the given byte range is in use. */
		virtual bool IsRangeInUse(u32 offset, u32 size) const = 0;
#endif

	protected:
		friend class b3d::GpuDevice;
		friend class b3d::GpuBuffer;
		friend struct GpuBufferUtility;

		/** Constructs a new GPU buffer. */
		GpuBuffer(GpuDevice& device, const GpuBufferCreateInformation& createInformation, u32 suballocationSize);

		void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

		/** Recreates the underlying buffer. Note this will clear all currently written data. Old buffer will be released once its done being used. */
		virtual void RecreateInternalBuffer() = 0;

#if B3D_BUILD_TYPE_DEVELOPMENT
		/** Logs a warning if the offset we're trying to map is currently being used on GPU, or is bound to any command buffer. */
		void ValidateMap(u32 offset, u32 size, GpuMapOptions options);
#endif

	protected:
		GpuBufferInformation mInformation;
		GpuDevice& mDevice;
		String mName;
		u32 mSuballocationSize = 0;
		u32 mTotalSize = 0;
		void* mMappedMemory = nullptr;
	};

	/** Flags used to control the GPU buffer writes. */
	enum class GpuBufferWriteFlag
	{
		/**
		 * Default flag. Performs the write assuming the buffer is not currently bound to a command buffer, or used by the GPU.
		 * If the buffer is in use by the GPU or bound to a command buffer, it's expected the caller will provide a command buffer
		 * on which to queue the write operation on, to ensure previous uses are not disturbed. Otherwise write fails.
		 */
		Normal = 0,

		/**
		 * If the buffer is currently being used on the GPU the system will internally allocate new memory for the buffer
		 * and write to the new memory. Old buffer memory will remain for whatever purpose it was used for until
		 * execution finishes, at which point it will be freed. Caller must ensure to either fully write in the
		 * buffer range, as anything not written by the caller will be undefined. Useful if you don't care about
		 * previous buffer contents.
		 */
		Discard = 1 << 0,

		/**
		 * If the buffer is currently being used on the GPU the system will still let you update it. It's up to the
		 * caller not to update the same memory region as the GPU is operating on, while respecting any other rules
		 * required by the low-level render API when doing such an operation (such as issuing memory barriers, flushing
		 * memory and respecting granularity). Use only when you know what you are doing.
		 */
		NoOverwrite = 1 << 1
	};

	using GpuBufferWriteFlags = Flags<GpuBufferWriteFlag>;
	B3D_FLAGS_OPERATORS(GpuBufferWriteFlag);

	/** Provides various utility operations on GpuBuffer. */
	struct B3D_EXPORT GpuBufferUtility
	{
		/**
		 * Creates a staging buffer that can be used for as copy source or destination for the provided buffer.
		 *
		 * @param	buffer		Buffer to create the the staging buffer for. The staging buffer will have enough size to fit the contents of this buffer.
		 * @param	readable	True if the buffer needs to be CPU-readable, false if the buffer needs to be CPU-writeable.
		 * @return				Newly created buffer.
		 */
		static SPtr<GpuBuffer> CreateStaging(const SPtr<GpuBuffer>& buffer, bool readable);

		/**
		 * Writes data into a buffer while accounting for the fact that the buffer might not be directly CPU-writable. Only buffers with
		 * StoreOnCPUWithGPUAccess flag, or staging write buffers are directly writable by the CPU. And only in the case they are not currently being
		 * used by the GPU.
		 *
		 * If a buffer is being used by the GPU, or is not directly CPU-writable, this method will internally create a staging buffer, write the data into it,
		 * and then copy the staging buffer into the destination buffer using the provided command buffer. If no command buffer is provided, it will use
		 * a transfer buffer which will be submitted automatically before the next regular command buffer submission.
		 *
		 * @param	offset			Offset in bytes into the destination buffer at which to copy the data to.
		 * @param	length			Length of the area you want to copy, in bytes.
		 * @param	source			Source buffer containing the data to write. Data is read from the start of the buffer (@p offset is only applied to the destination).
		 * @param	writeFlags		Optional write flags that may you can use to control behavior of the write operation if the buffer is used by the GPU.
		 * @param	commandBuffer	Command buffer on which to encode the staging buffer copy, in case the buffer is not directly writeable. If not provided
		 *							the operation will be queued on a transfer command buffer that will be submitted just before next regular command
		 *							buffer submission (or at the latest, at the end of the current frame).
		 */
		static void Write(const SPtr<GpuBuffer>& buffer, u32 offset, u32 length, const void* source, GpuBufferWriteFlags writeFlags = GpuBufferWriteFlag::Normal, SPtr<GpuCommandBuffer> commandBuffer = nullptr);

		/**
		 * Reads data from a buffer while accounting for the fact that the buffer might not be directly CPU-readable. Only buffers with
		 * StoreOnCPUWithGPUAccess flag, or staging read buffers are directly readable by the CPU. And only in the case they are not currently being
		 * used by the GPU.
		 *
		 * If a buffer is being used by the GPU, or is not directly CPU-readable, this method will internally create a staging buffer, copy the source
		 * buffer into the staging buffer using the provided GPU queue, and then read the data from the staging buffer. If no GPU queue is provided,
		 * it will use the default graphics queue.
		 *
		 * Note if the buffer is currently used on the GPU, this method will block until the GPU is done executing, stalling the pipeline.
		 *
		 * @param	buffer			Buffer to read from.
		 * @param	offset			Offset in bytes from which to copy the data.
		 * @param	length			Length of the area you want to copy, in bytes.
		 * @param	destination		Destination buffer large enough to store the read data. Data is written from the start of the buffer (@p offset is only applied to the source).
		 * @param	gpuQueue		GPU queue on which to perform the read. If not specified the default transfer queue will be used.
		 */
		static void Read(const SPtr<GpuBuffer>& buffer, u32 offset, u32 length, void* destination, const SPtr<GpuQueue>& gpuQueue = nullptr);

		/**
		 * Performs a non-blocking read operation. The GPU will execute the read when the command buffer reaches the execution point
		 * and the asynchronous operation will be signaled with the return value.
		 *
		 * @param	commandBuffer	Command buffer to queue the operation on.
		 * @param	offset			Offset in bytes from which to read the data.
		 * @param	length			Length of the area you want to read, in bytes.
		 * @return					Operation that will be signaled when the data is ready to be read.
		 */
		static TAsyncOp<SPtr<MemoryDataStream>> ReadAsync(const SPtr<GpuBuffer>& buffer, u32 offset, u32 length, GpuCommandBuffer& commandBuffer);
	};

} // namespace b3d::render

/** @} */
