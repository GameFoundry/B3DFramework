//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuBarriers.h"
#include "B3DGpuParameterSetPool.h"
#include "B3DGpuQueries.h"
#include "B3DGpuTimelineFence.h"
#include "B3DPrerequisites.h"
#include "B3DSamplerState.h"
#include "B3DGpuCompletionTracker.h"
#include "B3DGpuQueue.h"
#include "B3DGpuSubmitThread.h"
#include "B3DGpuWorkContext.h"

#include "B3DGpuSplitBarrier.h"
#include "B3DGpuBindlessHeap.h"
#include "B3DGpuMemoryDefragmenter.h"

namespace b3d::render
{
	class GpuQueryPool;
	class GpuCommandBufferPoolRing;
	class GpuBuffer;
	class Texture;
}

namespace b3d
{
	class GpuPipelineParameterSetLayout;
	class GpuCommandCapture;
	class GpuTransferQueue;
	class GpuPageMappingQueue;
	class IGpuAllocator;
	class IGpuPagePool;
	class IGpuTransientHeapPool;
	struct GpuPageMapping;
	struct GpuVirtualTextureLayout;
	struct GpuMemoryRequirements;
	struct GpuAllocation;
	struct SamplerStateCreateInformation;
	struct TextureCreateInformation;
	struct TextureCopyInformation;
	struct TextureBlitInformation;

	namespace render
	{
		struct GpuCommandBufferPoolCreateInformation;
		class GpuCommandBufferPool;
		class GpuVertexInput;
		class GpuVertexInputManager;
		struct GpuVertexInputLayout;
		class GpuAccelerationStructure;
		class GpuShaderBindingTable;
	}

	struct GpuPipelineParameterLayoutCreateInformation;
	struct GpuResourceTableLayout;
	struct GpuProgramBytecode;
	struct GpuBufferCreateInformation;
	struct GpuProgramCreateInformation;
	struct GpuComputePipelineStateCreateInformation;
	struct GpuGraphicsPipelineStateCreateInformation;
	struct GpuRayTracingPipelineStateCreateInformation;
	struct GpuShaderBindingTableCreateInformation;
	struct GpuAccelerationStructureSizes;
	struct GpuAccelerationStructureCreateInformation;
	struct GpuAccelerationStructureInputs;
	struct GpuRayTracingInstance;
	struct GpuSerializedAccelerationStructureInformation;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Flags that control how GpuDevice::CreateScratchAllocator() creates an allocator. */
	enum class GpuScratchAllocatorFlag
	{
		None = 0,
		ThreadSafe = 1 << 0 /**< The allocator may be used from multiple threads at once. */
	};

	using GpuScratchAllocatorFlags = Flags<GpuScratchAllocatorFlag>;
	B3D_FLAGS_OPERATORS(GpuScratchAllocatorFlag)

	/**
	 * Provides access to a particular GPU device.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuDevice
	{
	public:
		virtual ~GpuDevice();

		/** Initializes the GpuDevice. Should be called after construction but before any other operations. */
		virtual bool Initialize() = 0;

		/** Returns true if Initialize() has been called. */
		virtual bool IsInitialized() const = 0;

		virtual const GpuDeviceCapabilities& GetCapabilities() const = 0;

		/**
		 * Returns the stages the device has: every stage except the ray tracing shader stages without RSC_RAY_TRACING_PIPELINE, rays
		 * traced in the vertex, fragment and compute shader stages without RSC_RAY_QUERY, and the acceleration structure build stages
		 * without RSC_ACCELERATION_STRUCTURES.
		 */
		render::GpuStageFlags GetSupportedStageFlags() const;

		/** Returns information about available output devices and their video modes. */
		virtual const VideoModeInfo& GetVideoModeInfo() const = 0;

		/** Query if a GPU program language is supported (for example "hlsl", "glsl"). Thread safe. */
		virtual bool IsGpuProgramLanguageSupported(const StringView& language) const = 0;

		/** Returns the number of queues supported for the specific usage. */
		virtual u32 GetQueueCount(GpuQueueType type) const = 0;

		/** Retrieves a queue with the specified usage and index. */
		virtual TShared<GpuQueue> GetQueue(GpuQueueType type, u32 index) const = 0;

		/** Invokes the callback for every queue on the device, across all queue types. */
		void DoForEachQueue(const std::function<void(GpuQueue&)>&& callback) const;

		/**
		 * Presents the back-buffer image from the provided window onto the window, on the first graphics queue. See
		 * GpuQueue::PresentRenderWindow().
		 *
		 * @param	renderWindow		Window whose back-buffer to present.
		 * @param	syncMask			Optional synchronization mask that determines if the present operation
		 *								depends on command buffers submitted on other queues.
		 *
		 * @note	Render thread only.
		 */
		void PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll);

		/** Blocks the calling thread until all operations on the device finish. */
		virtual void WaitUntilIdle() = 0;

		/**
		 * Returns the submit thread responsible for executing queue submit and present operations for this device.
		 * Only valid once the device has started up, and until it shuts down.
		 */
		render::GpuSubmitThread& GetSubmitThread() const
		{
			B3D_ASSERT(mSubmitThread != nullptr);
			return *mSubmitThread;
		}

		/** Returns true if the submit thread is running. It doesn't run during device startup and shutdown. */
		bool HasSubmitThread() const { return mSubmitThread != nullptr; }

		/**
		 * Returns the queue that uploads CPU data to buffers and textures. Like the submit thread, only valid once the device has started
		 * up, and until it shuts down. Thread safe.
		 */
		GpuTransferQueue& GetTransferQueue() const
		{
			B3D_ASSERT(mTransferQueue != nullptr);
			return *mTransferQueue;
		}

		/** Notifies the device the rendering for the current frame will start. See EndFrame(). Render thread only. */
		virtual void BeginFrame() = 0;

		/**
		 * Notifies the device the rendering for the current frame has ended, see BeginFrame(). Queues the pending removals of virtual buffer
		 * and texture pages whose frame has completed (see GpuPageTable), submits the uploads queued on the transfer queue, blocks until the previous frame's resources are safe to
		 * reuse, then reclaims unused memory of every transient heap pool and page pool (see GetPagePool()), and advances the frame index. Render thread only.
		 */
		void EndFrame();

		/**
		 * Submits the page mappings that give virtual buffers and textures the memory committed by GpuPageTable::TryCommit() to the GPU
		 * immediately, instead of with the next submission. Call it right after the commits, before recording the work that uses them, 
		 * so the mappings execute while the GPU is still busy with earlier work. Otherwise the first submission using the new pages 
		 * might need to wait for them to execute. Make all commits first and flush once, since every flush pays a fixed cost for 
		 * its GPU call, regardless of how many pages it maps. Thread safe.
		 */
		void FlushPageMappings();

		/** Returns the subsystem that moves textures and buffers in persistent GPU memory. */
		GpuMemoryDefragmenter& GetMemoryDefragmenter() { return mMemoryDefragmenter; }

		/************************************************************************/
		/* 								CREATION METHODS                   		*/
		/************************************************************************/

		/**
		 * Compiles the GPU program to an intermediate bytecode format. The bytecode can be cached and used for
		 * quicker compilation/creation of GPU programs. 
		 */
		virtual TShared<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const;

		/** Creates a command buffer pool that may be used for allocating command buffers. */
		virtual TShared<render::GpuCommandBufferPool> CreateGpuCommandBufferPool(const render::GpuCommandBufferPoolCreateInformation& createInformation) = 0;

		/**
		 * Creates a new GPU texture whose memory comes from the device's persistent allocator for its memory type.
		 *
		 * @param	createInformation	Object describing the texture to create.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 */
		TShared<render::Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None);

		/**
		 * Creates a new GPU texture at the provided allocation.
		 *
		 * A pending allocation allocates from its allocator, including whenever the texture recreates its native resource.
		 * An allocation with memory fixes the texture to that memory, which must satisfy GetMemoryRequirements() for
		 * @p createInformation.
		 *
		 * @param	createInformation	Object describing the texture to create.
		 * @param	allocation			Memory the texture is created at. Must not be empty.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 * @return						Created texture, or null if @p allocation is not valid for @p createInformation.
		 */
		TShared<render::Texture> CreateTexture(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None);

		/**
		 * Creates a new GPU buffer whose memory comes from the device's persistent allocator for its memory type. Use
		 * GpuWorkContext::CreateScratchGpuBuffer for short-lived buffers backed by a context's scratch allocator.
		 *
		 * @param	createInformation	Object describing the buffer to create.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 */
		TShared<render::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None);

		/**
		 * Creates a new GPU buffer at the provided allocation. Allocation rules match
		 * CreateTexture(const TextureCreateInformation&, const GpuAllocation&, GpuObjectCreateFlags).
		 *
		 * @param	createInformation	Object describing the buffer to create.
		 * @param	allocation			Memory the buffer is created at. Must not be empty.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 * @return						Created buffer, or null if @p allocation is not valid for @p createInformation.
		 */
		TShared<render::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None);

		/** Returns the memory a texture described by @p createInformation needs. Thread safe. */
		virtual GpuMemoryRequirements GetMemoryRequirements(const TextureCreateInformation& createInformation) const = 0;

		/** Returns the memory a buffer described by @p createInformation needs. Thread safe. */
		virtual GpuMemoryRequirements GetMemoryRequirements(const GpuBufferCreateInformation& createInformation) const = 0;

		/**
		 * Returns the page layout of a virtual texture, or false if the device cannot create @p createInformation as virtual. Returns false
		 * by default: backends with RSC_VIRTUAL_TEXTURES override it. Thread safe.
		 */
		virtual bool TryGetVirtualTextureLayout(const TextureCreateInformation& createInformation, GpuVirtualTextureLayout& outLayout) const;

		/**
		 * Creates an acceleration structure whose memory comes from the device's persistent allocator. Build it with
		 * render::GpuCommandBuffer::BuildAccelerationStructures().
		 *
		 * @param	createInformation	Object describing the structure to create.
		 * @return						Created structure, or null, with the reason logged, if the device doesn't support acceleration
		 *								structures or @p createInformation is invalid.
		 */
		TShared<render::GpuAccelerationStructure> CreateGpuAccelerationStructure(const GpuAccelerationStructureCreateInformation& createInformation);

		/**
		 * Returns the memory an acceleration structure with @p inputs needs, and the scratch memory its build needs. Thread safe.
		 *
		 * @param	inputs		Contents of the acceleration structure. Buffers are only inspected, not read.
		 * @param	outSizes	Receives the sizes. Zero if the call fails.
		 * @return				False, with the reason logged, if the device doesn't support acceleration structures or the inputs are invalid.
		 */
		virtual bool GetAccelerationStructureSizes(const GpuAccelerationStructureInputs& inputs, GpuAccelerationStructureSizes& outSizes) const;

		/**
		 * Reads the description of serialized acceleration structure data (see render::GpuCommandBuffer::SerializeAccelerationStructure()),
		 * and checks that this device can deserialize it. Data serialized on another GPU, or with another driver version, usually can't
		 * be deserialized. Thread safe.
		 *
		 * @param	data			Serialized data.
		 * @param	outInformation	Receives the description of the data. Zero if the call fails.
		 * @return					True if this device can deserialize the data. False if it can't, and False, with the reason logged, if the
		 *							device doesn't support serialization or @p data is smaller than its header reports.
		 */
		virtual bool GetSerializedAccelerationStructureInformation(TArrayView<const u8> data, GpuSerializedAccelerationStructureInformation& outInformation) const;

		/**
		 * Converts @p instances to the layout top level acceleration structure builds read, see GpuAccelerationStructureInputs::InstanceBuffer.
		 * Thread safe.
		 *
		 * @param	instances		Instances to convert. Their bottom level structures, if any, must stay alive while the call runs.
		 * @param	destination		Memory that receives the instances, GpuRayTracingCapabilities::InstanceSize bytes each.
		 * @return					False, with the reason logged, if the device doesn't support acceleration structures or an instance is
		 *							invalid. Nothing is written then.
		 */
		virtual bool WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance> instances, void* destination) const;

		/**
		 * Writes the native reference of each structure in @p bottomLevels to @p destination, 8 bytes each, in order. A shader passes a
		 * reference to WriteRayTracingInstance() (RayTracingInstances.bslinc) to place the structure in a top level structure. A null entry
		 * writes the reference of no structure, which makes the instance empty. A reference stays valid for as long as the structure exists.
		 * Thread safe.
		 *
		 * @param	bottomLevels	Bottom level structures. Must stay alive while the call runs.
		 * @param	destination		Memory that receives the references, at least 8 bytes per structure. May be write-combined.
		 * @return					False, with the reason logged, if the device doesn't support acceleration structures or a structure is
		 *							not a bottom level structure. Nothing is written then.
		 */
		virtual bool WriteAccelerationStructureReferences(TArrayView<const render::GpuAccelerationStructure* const> bottomLevels, void* destination) const;

		/**
		 * Returns the device-owned persistent allocator for memory type @p memoryType, as reported by
		 * GetMemoryRequirements(). Thread safe.
		 */
		virtual IGpuAllocator& GetPersistentAllocator(u32 memoryType) = 0;

		/** Appends every persistent allocator of the device created so far to @p outAllocators. Thread safe. */
		virtual void GetPersistentAllocators(TArray<IGpuAllocator*>& outAllocators) = 0;

		/**
		 * Creates a new sampler state, or returns an existing one if one with the same create information was already created.
		 *
		 * @param	createInformation		Object describing the sampler state to create.
		 */
		virtual TShared<SamplerState> FindOrCreateSamplerState(const SamplerStateCreateInformation& createInformation);

		/** Returns the cache of the vertex input objects created on this device. */
		render::GpuVertexInputManager& GetVertexInputManager() const { return *mVertexInputManager; }

		/**
		 *  Creates a sampler state.
		 *
		 * @param	createInformation		Object describing the sampler state to create.
		 * @param	flags					Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Creates a new query pool. Returns null if the device does not support queries of the requested type, or cannot create the pool.
		 *
		 * @param	createInformation		Object describing the query pool to create.
		 */
		virtual TShared<render::GpuQueryPool> CreateQueryPool(const render::GpuQueryPoolCreateInformation& createInformation) = 0;

		/** Create a new event query. */
		virtual TShared<render::EventQuery> CreateEventQuery() = 0;

		/**
		 * Creates a new GPU program using the provided source code. If compilation fails or program is not supported
		 * GpuProgram::IsCompiled() will return false, and you will be able to retrieve the error message via GpuProgram::GetCompileErrorMessage().
		 *
		 * @param	createInformation		Object describing the program to create.
		 * @param	flags					Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Creates a graphics pipeline.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	flags					Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Creates a compute pipeline.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	flags					Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Creates a ray tracing pipeline. Thread safe.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	flags					Creation flags. @see GpuObjectCreateFlag
		 * @return							Created pipeline, or null, with the reason logged, if the device doesn't support ray tracing
		 *									pipelines or @p createInformation is invalid.
		 */
		TShared<GpuRayTracingPipelineState> CreateGpuRayTracingPipelineState(const GpuRayTracingPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None);

		/**
		 * Creates a shader binding table, the records that pick the programs of a ray tracing pipeline that run for each ray. Run it with
		 * render::GpuCommandBuffer::DispatchRays(). Thread safe.
		 *
		 * @param	createInformation		Object describing the table to create.
		 * @return							Created table, or null, with the reason logged, if the device doesn't support ray tracing pipelines
		 *									or @p createInformation is invalid.
		 */
		TShared<render::GpuShaderBindingTable> CreateGpuShaderBindingTable(const GpuShaderBindingTableCreateInformation& createInformation);

		/**
		 * Creates a pipeline layout from a set of GPU program parameter descriptions.
		 *
		 * @param	createInformation		Object describing the layout to create.
		 */
		virtual TShared<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation) = 0;

		/**
		 * Creates a single GPU pipeline parameter set layout from a parameter description.
		 *
		 * @param	parameterDescription	Description of parameters in the set.
		 * @param	resourceTableLayout		Optional reflected resource-table layout from a compiled program that backs
		 *									the set. Backends that pack descriptors at compiler-chosen offsets consume it
		 *									together with @p tableIndex - to place descriptors exactly where the shader 
		 *									reads them; other backends ignore it. Null when no compiled program backs the 
		 *									set (engine-authored layouts).
		 * @param	tableIndex				Index of the child descriptor table backing the set within
		 *									@p resourceTableLayout (an index into GpuResourceTableLayout::Tables). Only
		 *									meaningful when @p resourceTableLayout is non-null; ~0u otherwise.
		 * @return							The created set layout.
		 */
		virtual TShared<GpuPipelineParameterSetLayout> CreateGpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription, const TShared<GpuResourceTableLayout>& resourceTableLayout = nullptr, u32 tableIndex = ~0u) = 0;

		/**
		 * Maps a shader register to the parameter slot value this backend uses in parameter descriptions and set
		 * layouts. Engine-authored parameter descriptions must assign slots through this mapping to remain
		 * slot-compatible with shader-reflected layouts, which may encode more than the register index into the slot
		 * value (e.g. the register class).
		 *
		 * Backends that don't encode the class use the register index as the slot, so register indices within one set
		 * must be unique across all register classes.
		 */
		virtual u32 GetParameterSlot(ShaderRegisterClass registerClass, u32 registerIndex) const { return registerIndex; }

		/**
		 * Creates a parameter set pool for allocating GPU parameter sets.
		 *
		 * @param	createInformation	Pool configuration including mode and capacity limits.
		 * @return						Created parameter set pool.
		 */
		virtual TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) = 0;

		/** Creates a timeline fence that can be signaled when command buffer execution finishes. */
		virtual TShared<GpuTimelineFence> CreateTimelineFence() = 0;

		/**
		 * Creates an object that connects the release of a split barrier with its acquire. See
		 * render::GpuCommandBuffer::ReleaseBarriers(). Thread safe.
		 *
		 * @param	releaseQueue	Type of the queue the release is recorded for. Empty if it matches @p acquireQueue.
		 * @param	acquireQueue	Type of the queue the acquire is recorded for. Empty if it matches @p releaseQueue. If both are
		 *							empty, the halves are recorded for one queue of any type, which is the common case.
		 */
		virtual TShared<render::GpuSplitBarrier> CreateSplitBarrier(TOptional<GpuQueueType> releaseQueue = {}, TOptional<GpuQueueType> acquireQueue = {});

		/**
		 * Backend factory for a context-owned scratch (linear/bump) allocator. Manufactures an
		 * allocator for memory type @p memoryType, drawing pages from the device's shared per-type page
		 * pool and retiring them against @p completionTracker. Ownership transfers to the caller. Without
		 * GpuScratchAllocatorFlag::ThreadSafe in @p flags the allocator must only be used by one thread at a time.
		 *
		 * The base implementation returns nullptr (context scratch allocation unsupported); backends
		 * that support it override this.
		 */
		virtual TUnique<IGpuAllocator> CreateScratchAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker, GpuScratchAllocatorFlags flags = GpuScratchAllocatorFlag::None);

		/**
		 * Returns the transient heap pool that resources of memory type @p memoryType, as reported by GetMemoryRequirements(),
		 * are placed in by GpuTransientResourceAllocator. Several memory types may share one pool. Returns null if the memory
		 * type cannot be aliased. Thread safe.
		 */
		virtual IGpuTransientHeapPool* GetTransientHeapPool(u32 memoryType);

		/** Returns every transient heap pool of the device. Thread safe. */
		TArrayView<IGpuTransientHeapPool* const> GetTransientHeapPools() const;

		/**
		 * Returns the page pool for @p memoryType, as reported by GetMemoryRequirements() for a virtual resource. A page pool owns the heaps
		 * whose memory GpuPageTable commits to the pages of virtual buffers and textures. Null if there is none, which is the default: backends
		 * with RSC_VIRTUAL_BUFFERS or RSC_VIRTUAL_TEXTURES override it. Thread safe.
		 */
		virtual IGpuPagePool* GetPagePool(u32 memoryType);

		/** Returns the index of the current frame, as advanced by EndFrame(). Thread safe. */
		u64 GetFrameIndex() const { return mFrameCompletionTracker.GetCurrentMarker(); }

		/** Returns the bindless heaps of the device, or null if the device doesn't support bindless resources. Thread safe. */
		GpuBindlessHeaps* GetBindlessHeaps() const { return mBindlessHeaps.get(); }

		/**
		 * Keeps @p resource alive until a matching UnpinResource() completes, even if it is destroyed. For native resources that GPU
		 * reads without command buffers tracking them, (e.g. through a bindless heap). Thread safe.
		 */
		void PinResource(IGpuResource& resource);

		/** Releases a pin taken by PinResource(), once the current frame completes on the GPU. Thread safe. */
		void UnpinResource(IGpuResource& resource);

		/************************************************************************/
		/* 								UTILITY METHODS                    		*/
		/************************************************************************/

		/** Contains a default matrix into a matrix suitable for use by this specific render system. */
		virtual void ConvertProjectionMatrix(const Matrix4& input, Matrix4& output) = 0;

		/**
		 * Generates a uniform buffer description and calculates per-uniform offsets for the provided buffer members.
		 * The generated offsets are GPU backend specific.
		 *
		 * @param	name			Name to assign the uniform block.
		 * @param	inOutUniforms	List of members in the uniform buffer. Only name, type and array size fields need to be
		 * 							populated, the rest will be populated when the method returns. If a parameter is a struct
		 * 							then the elementSize field needs to be populated with the size of the struct in bytes.
		 * @return					Descriptor for the uniform buffer holding the provided parameters as laid out by the
		 *							active GPU backend's layout.
		 */
		virtual GpuUniformBufferInformation GenerateUniformBufferInformation(const String& name, TArray<GpuUniformBufferMemberInformation>& inOutUniforms) = 0;

		/**
		 * Converts a GPU timestamp into a time in milliseconds.

		 * @param timestamp		Timestamp as the one retrieved from timestamp GPU query.
		 * @return				Time in milliseconds.
		 */
		virtual float ConvertTimestampToMilliseconds(u64 timestamp) = 0;

		/**
		 * Explicit deleter for objects that derive from RenderProxy. By default render proxy objects provide a custom deleter that
		 * ensure they always get deleted on the render thread. But this behaviour is not always wanted (i.e. if creating a GPU object
		 * on a worker thread), in which case this deleter will be used.
		 */
		template <typename Type, typename MainAllocatorTag = DefaultAllocatorTag, typename PointerDataAllocatorTag = DefaultAllocatorTag>
		static TShared<Type> MakeSharedStandalone(Type* data)
		{
			auto fnStandaloneDeleter = [](render::RenderProxy* object)
			{
				if(!object->IsDestroyed())
					object->Destroy();

				B3DDelete<Type, MainAllocatorTag>((Type*)object);
			};

			return TShared<Type>(data, fnStandaloneDeleter, StdAlloc<Type, PointerDataAllocatorTag>());
		}

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Releases the pins whose UnpinResource() frame completed on the GPU. EndFrame() does this every frame. Thread safe.
		 *
		 * @param	isGpuIdle	If true, releases every pin passed to UnpinResource(), whichever frame that was in. The GPU must be idle.
		 *						Backends pass true during destruction, once the bindless heaps are destroyed and while their resource
		 *						manager is still alive.
		 */
		void ReleaseUnpinnedResources(bool isGpuIdle);

		/**
		 * Executes page mappings of virtual buffers and textures before returning, without ordering them against submitted GPU work (see
		 * IGpuSubmitThreadBackend::ExecutePageMappings()). Every mapping must map memory to unmapped pages. Returns false and executes nothing
		 * if the device doesn't support immediate mapping, which is the default. Thread safe.
		 */
		virtual bool TryExecutePageMappingsImmediately(TArrayView<const GpuPageMapping> mappings);

		/**
		 * Returns the changes of the memory mapped to the pages of the device's virtual buffers and textures that are not executed yet. Thread
		 * safe.
		 */
		GpuPageMappingQueue& GetPageMappingQueue() const { return *mPageMappingQueue; }

		/** @} */

	protected:
		friend class render::GpuVertexInputManager;

		GpuDevice();

		/** Creates the transfer queue. Backends call it right after creating the submit thread. */
		void CreateTransferQueue();

		/**
		 * Destroys the transfer queue, dropping the uploads still queued on it. Backends call it once the device is idle, before destroying
		 * the submit thread.
		 */
		void DestroyTransferQueue();

		/**
		 * Creates a new GPU texture at @p allocation, which is pending or has memory that satisfies the texture's memory
		 * requirements. See CreateTexture(const TextureCreateInformation&, const GpuAllocation&, GpuObjectCreateFlags).
		 */
		virtual TShared<render::Texture> CreateTextureInternal(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) = 0;

		/**
		 * Creates a new GPU buffer at @p allocation, which is pending or has memory that satisfies the buffer's memory
		 * requirements. See CreateGpuBuffer(const GpuBufferCreateInformation&, const GpuAllocation&, GpuObjectCreateFlags).
		 */
		virtual TShared<render::GpuBuffer> CreateGpuBufferInternal(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags) = 0;

		/**
		 * Creates an acceleration structure described by @p createInformation, which is valid for the device. See
		 * CreateGpuAccelerationStructure(). Returns null by default: backends with RSC_ACCELERATION_STRUCTURES override it.
		 */
		virtual TShared<render::GpuAccelerationStructure> CreateGpuAccelerationStructureInternal(const GpuAccelerationStructureCreateInformation& createInformation);

		/**
		 * Creates a ray tracing pipeline described by @p createInformation, which is valid for the device. See
		 * CreateGpuRayTracingPipelineState(). Returns null by default: backends with RSC_RAY_TRACING_PIPELINE override it.
		 */
		virtual TShared<GpuRayTracingPipelineState> CreateGpuRayTracingPipelineStateInternal(const GpuRayTracingPipelineStateCreateInformation& createInformation, GpuObjectCreateFlags flags);

		/**
		 * Creates a shader binding table described by @p createInformation, which is valid for the device. See
		 * CreateGpuShaderBindingTable(). Returns null by default: backends with RSC_RAY_TRACING_PIPELINE override it.
		 */
		virtual TShared<render::GpuShaderBindingTable> CreateGpuShaderBindingTableInternal(const GpuShaderBindingTableCreateInformation& createInformation);

		/**
		 * Creates the object mapping the vertex buffer elements of @p layout onto the inputs of a vertex shader, identified by @p id.
		 * Returns null if the backend cannot express the layout. Called by the vertex input manager, which caches the result.
		 */
		virtual render::GpuVertexInput* CreateVertexInput(u32 id, const render::GpuVertexInputLayout& layout) = 0;

		/**
		 * Registers a transient heap pool created by the backend, takes ownership of it, and returns it. Must only be called during
		 * device initialization.
		 */
		IGpuTransientHeapPool* AddTransientHeapPool(TUnique<IGpuTransientHeapPool> pool);

		/**
		 * Destroys every transient heap pool. Backends call this during destruction, once the GPU is idle and every resource
		 * placed in the pools is destroyed, and before the heap backends the pools create heaps with are destroyed.
		 */
		void DestroyTransientHeapPools();

		/**
		 * Registers a page pool created by the backend, takes ownership of it, and returns it. Every pool reclaims its unused memory at the
		 * end of each frame. Must only be called during device initialization.
		 */
		IGpuPagePool* AddPagePool(TUnique<IGpuPagePool> pool);

		/**
		 * Destroys every page pool. Backends call this during destruction, once the GPU is idle and every virtual resource is destroyed, and
		 * before the heap backends the pools create heaps with are destroyed.
		 */
		void DestroyPagePools();

		/**
		 * Takes ownership of the backend's bindless heaps and initializes them, creating the first native resource heap and the global
		 * samplers. Must be called once during device initialization, after the device can create sampler states. Returns false and
		 * destroys the heaps if initialization fails.
		 */
		bool CreateBindlessHeaps(TUnique<GpuBindlessHeaps> heaps);

		/**
		 * Returns the initial and maximum resource heap capacities the backend should create the bindless heaps with, from the
		 * configuration and @p deviceMaximumCapacity, the most the device supports.
		 */
		static void GetBindlessResourceCapacities(u32 deviceMaximumCapacity, u32& outCapacity, u32& outMaximumCapacity);

		/** Removes cached sampler states that nothing outside the cache references. Returns the number of removed sampler states. */
		u32 EvictUnusedSamplerStates();

		friend class GpuBindlessHeaps;

		mutable UnorderedMap<SamplerStateCreateInformation, TShared<SamplerState>> mCachedSamplerStates;
		mutable Mutex mSamplerStateMutex;

		/** Cache of the vertex input objects created on this device. Backends destroy it before their resource manager. */
		TUnique<render::GpuVertexInputManager> mVertexInputManager;

		/** Thread responsible for executing queue submit and present operations. Constructed by backends that use one. */
		TUnique<render::GpuSubmitThread> mSubmitThread;

		/** See GetTransferQueue(). Created and destroyed through CreateTransferQueue() and DestroyTransferQueue(). */
		TUnique<GpuTransferQueue> mTransferQueue;

		/** See GetPageMappingQueue(). Backends clear it once the device is idle, before their resource manager is destroyed. */
		TUnique<GpuPageMappingQueue> mPageMappingQueue;

		/**
		 * Completion tracker whose marker is the frame index, advanced by EndFrame(). Transient heap pools defer the destruction
		 * of released heaps against it.
		 */
		GpuFrameCompletionTracker mFrameCompletionTracker;

		/**
		 * Bindless heaps, created by backends that support bindless resources. Backends destroy them once the GPU is idle and the sampler
		 * state cache is cleared, before their native heaps and resource manager.
		 */
		TUnique<GpuBindlessHeaps> mBindlessHeaps;

	private:
		/** Pin waiting for the GPU to finish the frame it was released in. */
		struct PendingUnpin
		{
			PendingUnpin() = default;
			PendingUnpin(u64 frameIndex, IGpuResource* resource)
				: FrameIndex(frameIndex), Resource(resource)
			{ }

			u64 FrameIndex = 0; /**< Frame UnpinResource() was called in. */
			IGpuResource* Resource = nullptr;
		};

		TArray<IGpuTransientHeapPool*> mTransientHeapPools; /**< Owned. */
		TArray<IGpuPagePool*> mPagePools; /**< Owned. */
		GpuMemoryDefragmenter mMemoryDefragmenter;
		TArray<PendingUnpin> mPendingUnpins; /**< In release order, so frame indices ascend. */
		Mutex mPendingUnpinMutex;
	};

	/** @} */

} // namespace b3d
