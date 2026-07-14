//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuHazardTracking.h"
#include "B3DGpuParameterSetPool.h"
#include "B3DGpuQueries.h"
#include "B3DGpuTimelineFence.h"
#include "B3DPrerequisites.h"
#include "B3DSamplerState.h"
#include "B3DGpuCompletionTracker.h"
#include "B3DGpuQueue.h"
#include "B3DGpuSubmitThread.h"
#include "B3DGpuWorkContext.h"

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
	class GpuFrameCapture;
	class IGpuAllocator;
	struct SamplerStateCreateInformation;
	struct TextureCreateInformation;
	struct TextureCopyInformation;
	struct TextureBlitInformation;

	namespace render
	{
		struct GpuCommandBufferPoolCreateInformation;
		class GpuCommandBufferPool;
	}

	struct GpuPipelineParameterLayoutCreateInformation;
	struct GpuResourceTableLayout;
	struct GpuProgramBytecode;
	struct GpuBufferCreateInformation;
	struct GpuProgramCreateInformation;
	struct GpuComputePipelineStateCreateInformation;
	struct GpuGraphicsPipelineStateCreateInformation;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Flags that control creation of GPU objects via GpuDevice factory methods. */
	enum class GpuObjectCreateFlag
	{
		None = 0,
		DeferredInitialize = 1 << 0, /**< Don't call Initialize() automatically. Caller must ensure it gets called after creation. */
		RenderThreadDestroy = 1 << 1 /**< Ensures the object will always get destroyed on the render thread. Only relevant for render proxy objects. */
	};

	using GpuObjectCreateFlags = Flags<GpuObjectCreateFlag>;
	B3D_FLAGS_OPERATORS(GpuObjectCreateFlag)

	/**
	 * Provides access to a particular GPU device.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT GpuDevice
	{
	public:
		virtual ~GpuDevice() = default;

		/** Initializes the GpuDevice. Should be called after construction but before any other operations. */
		virtual bool Initialize() = 0;

		/** Returns true if Initialize() has been called. */
		virtual bool IsInitialized() const = 0;

		virtual const GpuDeviceCapabilities& GetCapabilities() const = 0;

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
		 * Presents the back-buffer image from the provided window onto the window, using the appropriate queue that supports present operations.
		 *
		 * @param	renderWindow		Window whose back-buffer to present.
		 * @param	syncMask			Optional synchronization mask that determines if the present operation
		 *								depends on command buffers submitted on other queues.
		 */
		virtual void PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) = 0;

		/** Blocks the calling thread until all operations on the device finish. */
		virtual void WaitUntilIdle() = 0;

		/**
		 * Returns the submit thread responsible for executing queue submit and present operations for this device.
		 * Only valid on backends that construct one during startup.
		 */
		render::GpuSubmitThread& GetSubmitThread() const
		{
			B3D_ASSERT(mSubmitThread != nullptr);
			return *mSubmitThread;
		}

		/** Notifies the device the rendering for the current frame will start. See EndFrame(). Render thread only. */
		virtual void BeginFrame() = 0;

		/** Notifies the device the rendering for the current frame has ended, see BeginFrame(). Render thread only. */
		virtual void EndFrame() {}

		/**
		 * Runs an incremental defragmentation pass over the device's persistent GPU memory allocators,
		 * recording the relocation copies into @p gpuContext's transfer command buffer. 
		 * No-op on backends without defragmentation support. Render thread only.
		 *
		 * @param	gpuContext	Work context whose transfer command buffer receives the relocation copies.
		 */
		virtual void RunDefragPass(GpuWorkContext& gpuContext) {}

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
		 * Creates a new GPU texture.
		 *
		 * @param	createInformation	Object describing the texture to create.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<render::Texture> CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Creates a new GPU buffer. The buffer's backing memory comes from the device's persistent
		 * per-memory-type allocator; use GpuWorkContext::CreateTransientGpuBuffer for short-lived buffers
		 * backed by a context's transient allocator.
		 *
		 * @param	createInformation	Object describing the buffer to create.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<render::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Determines the backend memory type a buffer described by @p createInformation allocates from.
		 * A buffer's memory type is a pure function of its create information, fixed for its lifetime, so
		 * the per-type allocator backing the buffer can be resolved once, up front. The base
		 * implementation returns 0 (no meaningful memory types); backends with per-type allocators
		 * override this.
		 *
		 * Thread safe.
		 */
		virtual u32 PickBufferMemoryType(const GpuBufferCreateInformation& /*createInformation*/) const { return 0; }

		/**
		 * Creates a new sampler state, or returns an existing one if one with the same create information was already created.
		 *
		 * @param	createInformation		Object describing the sampler state to create.
		 */
		virtual TShared<SamplerState> FindOrCreateSamplerState(const SamplerStateCreateInformation& createInformation);

		/**
		 *  Creates a sampler state.
		 *
		 * @param	createInformation		Object describing the sampler state to create.
		 * @param	flags					Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<SamplerState> CreateSamplerState(const SamplerStateCreateInformation& createInformation, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None) = 0;

		/**
		 * Creates a new query pool.
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
		 * Creates a parameter set pool for allocating GPU parameter sets.
		 *
		 * @param	createInformation	Pool configuration including mode and capacity limits.
		 * @return						Created parameter set pool.
		 */
		virtual TUnique<GpuParameterSetPool> CreateParameterSetPool(const GpuParameterSetPoolCreateInformation& createInformation) = 0;

		/** Creates a timeline fence that can be signaled when command buffer execution finishes. */
		virtual TShared<GpuTimelineFence> CreateTimelineFence() = 0;

		/**
		 * Backend factory for a context-owned transient (linear/bump) allocator. Manufactures an
		 * allocator for memory type @p memoryType, drawing pages from the device's shared per-type page
		 * pool and retiring them against @p completionTracker. Ownership transfers to the caller.
		 *
		 * The base implementation returns nullptr (context transient allocation unsupported); backends
		 * that support it override this.
		 */
		virtual TUnique<IGpuAllocator> CreateTransientAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker);

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

	protected:
		friend class GpuWorkContext;

		GpuDevice() = default;

		/**
		 * Creates a new GPU buffer whose backing memory is suballocated from an explicitly provided
		 * @p allocator instead of the device's persistent allocator. The allocator must be compatible
		 * with the buffer's memory type, i.e. resolved for the type returned by PickBufferMemoryType()
		 * (GpuWorkContext::CreateTransientGpuBuffer is the canonical caller). @p allocator must outlive the
		 * returned buffer.
		 *
		 * @param	createInformation	Object describing the buffer to create.
		 * @param	allocator			Allocator the buffer's backing memory is suballocated from.
		 * @param	flags				Creation flags. @see GpuObjectCreateFlag
		 */
		virtual TShared<render::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, IGpuAllocator& allocator, GpuObjectCreateFlags flags = GpuObjectCreateFlag::None);

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

		mutable UnorderedMap<SamplerStateCreateInformation, TShared<SamplerState>> mCachedSamplerStates;
		mutable Mutex mSamplerStateMutex;

		/** Thread responsible for executing queue submit and present operations. Constructed by backends that use one. */
		TUnique<render::GpuSubmitThread> mSubmitThread;
	};

	/** @} */

} // namespace b3d
