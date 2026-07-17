//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuDevice.h"
#include "B3DGpuHazardTracking.h"
#include "B3DGpuQueries.h"
#include "Image/B3DColor.h"
#include "Image/B3DTexture.h"
#include "Math/B3DArea2.h"
#include "Threading/B3DSingleConsumerQueue.h"

namespace b3d
{
	class GpuCommandBufferProfiler;
}

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Possible states that a command buffer can be in. The transitions generally happen in the order they are listed. */
		enum class GpuCommandBufferState
		{
			/** Command buffer is ready to start recording new commands. */
			Ready,

			/** Commands can be recorded to the buffer. This state is entered after calling Begin() on the command buffer. */
			Recording,

			/** Command buffer is currently recording and is within render pass. This state is entered when calling BeginRenderPass(), and exited after calling EndRenderPass(). */
			RecordingRenderPass,

			/** Command buffer is done recording but hasn't been submitted. This state is entered after calling End(). */
			RecordingDone,

			/** Command buffer has been submitted for execution on the GPU, but still hasn't finished executing. */
			Executing,

			/** Command buffer has finished executing on the GPU, but has not yet been reset. After reset the command buffer will go back to ready state. */
			Done
		};

		/** Bits that represent different ways a GPU resource can be used. */
		enum class GpuResourceUseFlag
		{
			Undefined = 0,
			ShaderAccess = 1 << 0, /**< Sampled or unordered access in any shader stage. Shader stage must be provided explicitly via one or multiple Stage* values. */
			IndexBuffer = 1 << 1, /**< Index buffer (read only). */
			VertexBuffer = 1 << 2, /**< Vertex buffer (read only). */
			UniformBuffer = 1 << 3, /**< Uniform buffer (read only) used in any shader stage. Shader stage must be provided explicitly via one or multiple Stage* values. */
			Transfer = 1 << 4, /**< Transfer source or destination. */
			ColorAttachment = 1 << 5, /**< Color attachment. */
			DepthStencilAttachment = 1 << 6, /**< Depth/stencil attachment. */
			Host = 1 << 7, /**< Access by the host (CPU). */

			// Stage flags can be combined with ShaderAccess or UniformBuffer usages, to clearly define at which stage the use is happening.
			// If not provided system usually assumes potential use in all shader stages.
			StageVertexShader = 1 << 8, /**< Access in vertex shader. */
			StageFragmentShader = 1 << 9, /**< Access in fragment shader. */
			StageComputeShader = 1 << 10, /**< Access in compute shader. */

			AnyStage = StageVertexShader | StageFragmentShader | StageComputeShader
		};

		typedef Flags<GpuResourceUseFlag> GpuResourceUseFlags;
		B3D_FLAGS_OPERATORS(GpuResourceUseFlag)

		/** Image layout - determines how an image is accessed in GPU operations. */
		enum class GpuImageLayout
		{
			/**
			 * Undefined layout - initial state or don't care.
			 * Transitioning from Undefined may discard image contents.
			 */
			Undefined,

			/**
			 * General purpose layout - supports all operations but may not be optimal.
			 * Required for storage images (UAV) and some read-write operations.
			 */
			General,

			/**
			 * Optimal for color attachment writes (render targets).
			 * Used during rendering to color attachments.
			 */
			ColorAttachment,

			/**
			 * Optimal for depth/stencil attachment writes.
			 * Used during rendering to depth/stencil attachments.
			 */
			DepthStencilAttachment,

			/**
			 * Optimal for depth/stencil attachment reads (read-only depth).
			 * Used when depth/stencil is bound but not written to, and potentially sampled.
			 */
			DepthStencilReadOnly,

			/** Optimal when the depth aspect is read-only (sampleable) while the stencil aspect is written. */
			DepthReadOnlyStencilAttachment,

			/** Optimal when the depth aspect is written while the stencil aspect is read-only (sampleable). */
			DepthAttachmentStencilReadOnly,

			/**
			 * Optimal for shader reads (sampling, texelFetch).
			 * Most common layout for texture sampling.
			 */
			ShaderReadOnly,

			/** Optimal for transfer source operations (copies, blits). */
			TransferSource,

			/** Optimal for transfer destination operations (copies, blits). */
			TransferDestination,

			/** Layout for presenting to swap chain. Only used for swap chain images. */
			Present
		};

		/** Object describing a GpuCommandBufferPool. */
		struct GpuCommandBufferPoolInformation
		{
			GpuQueueType Type = GQT_GRAPHICS; /**< Determines which commands may be executed on the command buffer. Queue on which the command buffer is submitted must match this usage. */
			ThreadId Thread; /**< Thread on which the command buffer pool is allowed to be used on. Any created command buffers are also bound to this thread. */
			bool UsePoolReset = false; /**< When true, command buffers are reset as a group via pool-level reset instead of individually. */
		};

		/** Descriptor structure used for initialization of a GpuCommandBufferPool. */
		struct GpuCommandBufferPoolCreateInformation : GpuCommandBufferPoolInformation
		{
			GpuCommandBufferPoolCreateInformation() = default;
			GpuCommandBufferPoolCreateInformation(const GpuCommandBufferPoolInformation& other)
				:GpuCommandBufferPoolInformation(other)
			{ }

			/** Allocates a create information for a command buffer pool owned by the calling thread. */
			static GpuCommandBufferPoolCreateInformation CreateForThisThread(GpuQueueType type = GQT_GRAPHICS)
			{
				GpuCommandBufferPoolCreateInformation createInformation;
				createInformation.Thread = B3D_CURRENT_THREAD_ID;
				createInformation.Type = type;

				return createInformation;
			}
		};

		/** Object describing a GpuCommandBuffer. */
		struct GpuCommandBufferInformation
		{
			String Name; /**< Name of the command buffer */
		};

		/** Descriptor structure used for initialization of a GpuCommandBufferPool. */
		struct GpuCommandBufferCreateInformation : GpuCommandBufferInformation 
		{
			GpuCommandBufferCreateInformation() = default;
			GpuCommandBufferCreateInformation(const GpuCommandBufferInformation& other)
				:GpuCommandBufferInformation(other)
			{ }

			/** Allocates a create information for a command buffer with the specified name. */
			static GpuCommandBufferCreateInformation Create(const StringView& name = "")
			{
				GpuCommandBufferCreateInformation createInformation;
				createInformation.Name = name;

				return createInformation;
			}
		};

		/**
		 * Allows creation of command buffers.
		 *
		 * All allocated command buffers may only be used on the GPU queues that have the subset of usage flags provided by this pool.
		 * Command buffer and all command buffers allocated from the command buffer may only be used on a single thread. Command buffers may only be used on another thread as part of command buffer submission.
		 */
		class B3D_EXPORT GpuCommandBufferPool
		{
		public:
			virtual ~GpuCommandBufferPool() = default;

			/** Returns queue that may be used for posting messages to the command buffer pool (e.g. command buffer completion notifies). */
			SingleConsumerQueue& GetMessageQueue() { return mMessageQueue; }

			/** Returns true if the pool uses pool-level reset instead of individual command buffer reset. */
			bool GetUsePoolReset() const { return mInformation.UsePoolReset; }

			/** Creates a new command buffer. */
			virtual TShared<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) = 0;

			/** Attempts to find a free command buffer from the pool, or creates a new one if it cannot be found. */
			virtual TShared<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) = 0;

			/** Resets the command buffer pool, allowing all previously allocated command buffers to be re-used. Must be called only after all previously allocated command buffers have completed executing. */
			virtual void Reset() = 0;

			/** Destroys the pool. Will be called automatically on destruction, but may be called earlier if desired. */
			virtual void Destroy();

		protected:
			friend class b3d::GpuDevice;

			GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation);

			/** Reports an error if the current thread is not the thread associated with the object. */
			void EnsureValidThread() const { B3D_DEBUG_ONLY(B3D_ENSURE(B3D_CURRENT_THREAD_ID == mInformation.Thread)); }

			GpuDevice& mGpuDevice;
			const GpuCommandBufferPoolInformation mInformation;
			SingleConsumerQueue mMessageQueue;
			bool mIsDestroyed = false;
		};

		/** Describes common fields for both buffer and texture barriers. See GpuCommandBuffer::IssueBarrier. */
		struct GpuBarrier
		{
			GpuBarrier(GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
				: SourceUsage(GpuResourceUseFlag::Undefined), SourceAccess(GpuAccessFlag::None), DestinationUsage(destinationUsage), DestinationAccess(destinationAccess)
			{ }

			GpuBarrier(GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
				: SourceUsage(sourceUsage), SourceAccess(sourceAccess), DestinationUsage(destinationUsage), DestinationAccess(destinationAccess)
			{ }

			GpuResourceUseFlags SourceUsage; /**< Determines how was resource used before the barrier. Barrier will only work for provided accesses. If undefined, source usage, access and layout is automatically deduced based on current buffer use. */
			GpuAccessFlags SourceAccess; /**< Determines if the resource was read or written before the barrier. */
			GpuResourceUseFlags DestinationUsage; /**< Determines how was resource will be used after the barrier. Barrier will only work for provided accesses. Images will transition to a layout compatible for this usage - incompatible usages are not allowed. */
			GpuAccessFlags DestinationAccess; /**< Determines if the resource will be read or written after the barrier. */
		};

		/** Describes a barrier for a GpuBuffer. */
		struct GpuBufferBarrier : GpuBarrier
		{
			GpuBufferBarrier(const TShared<GpuBuffer>& object, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
				: GpuBarrier(destinationUsage, destinationAccess), Object(object)
			{ }

			GpuBufferBarrier(const TShared<GpuBuffer>& object, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
				: GpuBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess), Object(object)
			{ }

			TShared<GpuBuffer> Object;
		};

		/** Describes a common set of barrier information used both by Texture and RenderTarget barrier. */
		struct GpuSurfaceBarrier : GpuBarrier
		{
			GpuSurfaceBarrier(GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuBarrier(destinationUsage, destinationAccess), SubresourceRange(subResourceRange)
			{ }

			GpuSurfaceBarrier(GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess,
				GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuBarrier(destinationUsage, destinationAccess), SubresourceRange(subResourceRange), SourceLayout(sourceLayout), DestinationLayout(destinationLayout)
			{ }

			GpuSurfaceBarrier(GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess,
				GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess), SubresourceRange(subResourceRange)
			{ }

			GpuSurfaceBarrier(GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess,
				GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess), SubresourceRange(subResourceRange), SourceLayout(sourceLayout), DestinationLayout(destinationLayout)
			{ }

			GpuTextureSubresourceRange SubresourceRange; /**< Subresources (mips, array levels) of the textures to apply the barrier to. */
			GpuImageLayout SourceLayout = GpuImageLayout::Undefined; /**< Source image layout. If usage is set to undefined, layout is determined from the current usage of the texture object, otherwise it must be provided. */
			GpuImageLayout DestinationLayout = GpuImageLayout::Undefined; /**< Destination image layout. If set to Undefined, no layout transition will be performed. */
		};

		/** Describes a barrier for a Texture. */
		struct GpuTextureBarrier : GpuSurfaceBarrier
		{
			GpuTextureBarrier(const TShared<Texture>& object, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(destinationUsage, destinationAccess, subResourceRange), Object(object)
			{ }

			GpuTextureBarrier(const TShared<Texture>& object, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess,
				GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(destinationUsage, destinationAccess, sourceLayout, destinationLayout, subResourceRange), Object(object)
			{ }

			GpuTextureBarrier(const TShared<Texture>& object, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess,
				GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess, subResourceRange), Object(object)
			{ }

			GpuTextureBarrier(const TShared<Texture>& object, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess,
				GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& subResourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess, sourceLayout, destinationLayout, subResourceRange), Object(object)
			{ }

			TShared<Texture> Object;
		};
/**
		 * Describes a barrier for a RenderTarget.
		 *
		 * This is functionally equivalent to specifying GpuTextureBarrier on the color or depth/stencil textures of the
		 * render target. However, it is necessary for issuing barriers on swap chain images, which cannot be accessed
		 * as standalone textures.
		 *
		 * @note SurfaceMask must specify only a single surface bit (e.g., RT_COLOR0, RT_DEPTH). Combinations of multiple bits are invalid.
		 */
		struct GpuRenderTargetBarrier : GpuSurfaceBarrier
		{
			GpuRenderTargetBarrier(const TShared<RenderTarget>& object, RenderSurfaceMaskBits surfaceMask, 
				GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, const GpuTextureSubresourceRange& subresourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(destinationUsage, destinationAccess, subresourceRange), Object(object), SurfaceMask(surfaceMask)
			{ }

			GpuRenderTargetBarrier(const TShared<RenderTarget>& object, RenderSurfaceMaskBits surfaceMask,
				GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& subresourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(destinationUsage, destinationAccess, sourceLayout, destinationLayout, subresourceRange), Object(object), SurfaceMask(surfaceMask)
			{ }

			GpuRenderTargetBarrier(const TShared<RenderTarget>& object, RenderSurfaceMaskBits surfaceMask, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess,
				GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, const GpuTextureSubresourceRange& subresourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess, subresourceRange), Object(object), SurfaceMask(surfaceMask)
			{ }

			GpuRenderTargetBarrier(const TShared<RenderTarget>& object, RenderSurfaceMaskBits surfaceMask, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess,
				GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout sourceLayout, GpuImageLayout destinationLayout, const GpuTextureSubresourceRange& subresourceRange = GpuTextureSubresourceRange::AllSubresources())
				: GpuSurfaceBarrier(sourceUsage, sourceAccess, destinationUsage, destinationAccess, sourceLayout, destinationLayout, subresourceRange), Object(object), SurfaceMask(surfaceMask)
			{ }

			TShared<RenderTarget> Object;
			RenderSurfaceMaskBits SurfaceMask; /**< Specifies which surface of the render target the barrier applies to. Must be a single bit. */
		};

		/** A list of buffer, texture, and render target barriers. */
		struct GpuBarriers
		{
			GpuBarriers(TArrayView<GpuBufferBarrier> bufferBarriers = TArrayView<GpuBufferBarrier>(), TArrayView<GpuTextureBarrier> textureBarriers = TArrayView<GpuTextureBarrier>(), TArrayView<GpuRenderTargetBarrier> renderTargetBarriers = TArrayView<GpuRenderTargetBarrier>())
			{
				BufferBarriers.Reserve(bufferBarriers.Size());

				for(const auto& entry : bufferBarriers)
					BufferBarriers.Add(entry);

				TextureBarriers.Reserve(textureBarriers.Size());

				for(const auto& entry : textureBarriers)
					TextureBarriers.Add(entry);

				RenderTargetBarriers.Reserve(renderTargetBarriers.Size());

				for(const auto& entry : renderTargetBarriers)
					RenderTargetBarriers.Add(entry);
			}

			GpuBarriers(TArrayView<GpuTextureBarrier> textureBarriers)
			{
				TextureBarriers.Reserve(textureBarriers.Size());

				for(const auto& entry : textureBarriers)
					TextureBarriers.Add(entry);
			}

			GpuBarriers(TArrayView<GpuRenderTargetBarrier> renderTargetBarriers)
			{
				RenderTargetBarriers.Reserve(renderTargetBarriers.Size());

				for(const auto& entry : renderTargetBarriers)
					RenderTargetBarriers.Add(entry);
			}

			GpuBarriers(const GpuTextureBarrier& textureBarrier)
			{
				TextureBarriers.Add(textureBarrier);
			}

			GpuBarriers(const GpuBufferBarrier& bufferBarrier)
			{
				BufferBarriers.Add(bufferBarrier);
			}

			GpuBarriers(const GpuRenderTargetBarrier& renderTargetBarrier)
			{
				RenderTargetBarriers.Add(renderTargetBarrier);
			}

			TInlineArray<GpuBufferBarrier, 2> BufferBarriers;
			TInlineArray<GpuTextureBarrier, 2> TextureBarriers;
			TInlineArray<GpuRenderTargetBarrier, 2> RenderTargetBarriers;
		};

		/**
		 * Descriptor structure used for initialization of a render pass. Render pass will bind the provided
		 * render target and allow draw calls to be executed until it ends.
		 */
		struct RenderPassCreateInformation
		{
			/** Render target to render to. */
			TShared<RenderTarget> Target;

			/**
			 * Which surfaces of the render target are read-only.
			 * Read-only surfaces cannot be written to during the render pass.
			 */
			RenderSurfaceMask ReadOnlyMask = RT_NONE;

			/**
			 * Which surfaces of the render target should preserve their existing contents.
			 * Use this when you need to blend, read, or perform other operations with
			 * the existing contents of the render target.
			 */
			RenderSurfaceMask LoadMask = RT_NONE;

			/**
			 * Determines which surfaces to clear when the render pass starts. Use @p ClearColor, @p ClearDepth and @p ClearStencil
			 * to determine which values to clear them to.
			 */
			RenderSurfaceMask ClearMask = RT_NONE;

			/** Determines the color to which to clear all color attachments, for attachments that clear is enabled in @p ClearMask. */
			Color ClearColor = Color::kBlack;

			/** Determines the depth to which to clear the depth attachment, if clear is enabled in @p ClearMask. */
			float ClearDepth = 1.0f;

			/** Determines the stencil to which to clear the stencil attachment, if clear is enabled in @p ClearStencil. */
			u32 ClearStencil = 0;

			/**
			 * Set of all GPU parameters that will be bound during this render pass. The command buffer will pre-register all resources
			 * from these parameters, allowing barriers and layout transitions to be issued before the render pass begins.
			 */
			TInlineArray<TShared<GpuParameterSet>, 4> Parameters;

			RenderPassCreateInformation() = default;

			RenderPassCreateInformation(const TShared<RenderTarget>& target, RenderSurfaceMask readOnlyMask = RT_NONE, RenderSurfaceMask loadMask = RT_NONE)
				: Target(target), ReadOnlyMask(readOnlyMask), LoadMask(loadMask)
			{ }

			RenderPassCreateInformation(const TShared<RenderTarget>& target, const TShared<GpuParameterSet>& parameters, RenderSurfaceMask readOnlyMask = RT_NONE, RenderSurfaceMask loadMask = RT_NONE)
				: Target(target), ReadOnlyMask(readOnlyMask), LoadMask(loadMask)
			{
				if(parameters != nullptr)
					Parameters.Add(parameters);
			}
		};

		/**
		 * Contains a list of render API commands that can be queued for execution on the GPU. User is allowed to populate the
		 * command buffer from any thread, ensuring render API command generation can be multi-threaded. Command buffers
		 * must always be created on the render thread. Same command buffer cannot be used on multiple threads simulateously
		 * without external synchronization.
		 */
		class B3D_EXPORT GpuCommandBuffer
		{
		public:
			virtual ~GpuCommandBuffer();

			/** Returns the GPU device the command buffer is created on. */
			GpuDevice& GetGpuDevice() const { return mGpuDevice; }

			/** Returns the queue type that determines on which queue is the command buffer allowed to be submitted on, and which commands may be recorded. */
			GpuQueueType GetQueueType() const { return mQueueType; }

			/** Returns the current state of the command buffer. */
			GpuCommandBufferState GetState() const { return mState; }

			/** Assigns an name to the command buffer, primarily used for easier debugging. */
			virtual void SetName(const StringView& name) { mName = name; }

			/**
			 * Binds the parameters so that the following draw or dispatch call uses the provided parameters
			 * in their GPU programs. The caller must ensure the provided parameters match the bound graphics/compute pipeline
			 * at the time of the draw/dispatch call.
			 */
			virtual void SetGpuParameterSet(const TShared<GpuParameterSet>& parameters) = 0;

			/**
			 * Applies an offset from which reads in a buffer should start in a GPU program. This allows caller to quickly change
			 * buffer contents as seen by the shader, without having to rebind GPU program parameters. You should only call this
			 * after binding all parameter sets, as the offsets will be reset when parameters are changed.
			 *
			 * @param set				Descriptor set index.
			 * @param bufferIndex		Dynamic buffer index within the set, as retrieved from GpuPipelineParameterLayout::GetDynamicOffsetIndex.
			 * @param offset			Offset to apply. Must be within the range of the currently bound buffer size and respect hardware alignment requirements.
			 */
			virtual void SetDynamicBufferOffset(u32 set, u32 bufferIndex, u32 offset) = 0;

			/** Sets a pipeline state that controls how will subsequent draw commands render primitives. */
			virtual void SetGpuGraphicsPipelineState(const TShared<GpuGraphicsPipelineState>& pipelineState) = 0;

			/** Sets a pipeline state that controls how will subsequent dispatch commands execute. */
			virtual void SetGpuComputePipelineState(const TShared<GpuComputePipelineState>& pipelineState) = 0;

			/**
			 * Sets the provided vertex buffers starting at the specified source index.	Set buffer to nullptr to clear the
			 * buffer at the specified index.
			 *
			 * @param	index			Index at which to start binding the vertex buffers.
			 * @param	buffers			A list of buffers to bind to the pipeline.
			 * @param	bufferCount		Number of buffers in the @p buffers list.
			 */
			virtual void SetVertexBuffers(u32 index, TShared<GpuBuffer>* buffers, u32 bufferCount) = 0;

			/**
			 * Sets an index buffer to use when drawing. Indices in an index buffer reference vertices in the vertex buffer,
			 * which increases cache coherency and reduces the size of vertex buffers by eliminating duplicate data.
			 *
			 * @param	buffer			Index buffer to bind, null to unbind.
			 */
			virtual void SetIndexBuffer(const TShared<GpuBuffer>& buffer) = 0;

			/**
			 * Sets the description of vertex elements in the vertex buffers that will be bound when executing the vertex GPU program.
			 *
			 * @param	vertexDescription	Vertex description to bind.
			 */
			virtual void SetVertexDescription(const TShared<VertexDescription>& vertexDescription) = 0;

			/**
			 * Sets the draw operation that determines how to interpret the elements of the index or vertex buffers.
			 *
			 * @param	operation			Draw operation to enable.
			 */
			virtual void SetDrawOperation(DrawOperationType operation) = 0;

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration and vertex buffers. Draws directly from
			 * the vertex buffer without using indices.
			 *
			 * @param	vertexOffset	Offset into the currently bound vertex buffer to start drawing from.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally) separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 */
			virtual void Draw(u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0) = 0;

			/**
			 * Draw an object based on currently bound GPU programs, vertex declaration, vertex and index buffers.
			 *
			 * @param	startIndex		Offset into the currently bound index buffer to start drawing from.
			 * @param	indexCount		Number of indices to draw.
			 * @param	vertexOffset	Offset to apply to each vertex index.
			 * @param	vertexCount		Number of vertices to draw.
			 * @param	instanceCount	Number of times to draw the provided geometry, each time with an (optionally) separate per-instance data.
			 * @param	firstInstance	ID of the first instance to draw.
			 */
			virtual void DrawIndexed(u32 startIndex, u32 indexCount, u32 vertexOffset, u32 vertexCount, u32 instanceCount = 0, u32 firstInstance = 0) = 0;

			/**
			 * Executes the currently bound compute shader.
			 *
			 * @param	groupCountX		Number of groups to start in the X direction. Must be in range [1, 65535].
			 * @param	groupCountY		Number of groups to start in the Y direction. Must be in range [1, 65535].
			 * @param	groupCountZ		Number of groups to start in the Z direction. Must be in range [1, 64].
			 */
			virtual void DispatchCompute(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) = 0;

			/**
			 * Begins a new render pass, allowing rendering commands to be recorded.
			 * All resources referenced by the provided GPU parameters will be registered with the
			 * command buffer and necessary barriers/layout transitions will be issued before the
			 * render pass begins.
			 *
			 * @param createInformation    Structure containing render target, masks, and all parameters
			 *                            that will be used during the render pass.
			 */
			virtual void BeginRenderPass(const RenderPassCreateInformation& createInformation) = 0;

			/** Ends the current render pass. */
			virtual void EndRenderPass() = 0;

			/** Returns true if the command buffer is currently recording a render pass. */
			virtual bool IsInRenderPass() const = 0;

			/**
			 * Issues a memory and/or execution barrier that guarantees that the contents of GPU buffers will be correctly visible for the provided destination stages.
			 * Additionally, transitions the images to the correct layout to be used in the destination.
			 *
			 * Note that system automatically issues barriers when needed, you do not need to call this method manually in almost all cases. Currently the only case
			 * you need to call this manually is when you read from a non-staging buffer that was written by the GPU, and you want to read the data on the CPU right
			 * after GPU execution completes.
			 */
			virtual void IssueBarriers(const GpuBarriers& barriers) = 0;

			/**
			 * Sets the active viewport that will be used for all following render operations.
			 *
			 * @param	area			Area of the viewport, in normalized ([0,1] range) coordinates.
			 */
			virtual void SetViewport(const Area2& area) = 0;

			/**
			 * Clears the currently active render target.
			 *
			 * @param	mask			Mask determining which surfaces of the render target to clear.
			 * @param	color			The color to clear the color buffer with, if enabled.
			 * @param	depth			The value to initialize the depth buffer with, if enabled.
			 * @param	stencil			The value to initialize the stencil buffer with, if enabled.
			 */
			virtual void ClearRenderTarget(RenderSurfaceMask mask, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0) = 0;

			/**
			 * Clears the currently active viewport (meaning it clears just a sub-area of a render-target that is covered by the
			 * viewport, as opposed to ClearRenderTarget() which always clears the entire render target).
			 *
			 * @param	mask			Mask determining which surfaces of the render target to clear.
			 * @param	color			The color to clear the color buffer with, if enabled.
			 * @param	depth			The value to initialize the depth buffer with, if enabled.
			 * @param	stencil			The value to initialize the stencil buffer with, if enabled.
			 */
			virtual void ClearViewport(RenderSurfaceMask mask, const Color& color = Color::kBlack, float depth = 1.0f, u16 stencil = 0) = 0;

			/**
			 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
			 * done to render target pixels outside of the provided region.
			 *
			 * @param	left			Left border of the scissor rectangle, in pixels.
			 * @param	top				Top border of the scissor rectangle, in pixels.
			 * @param	right			Right border of the scissor rectangle, in pixels.
			 * @param	bottom			Bottom border of the scissor rectangle, in pixels.
			 */
			virtual void EnableScissorTest(u32 left, u32 top, u32 right, u32 bottom) = 0;

			/**
			 * Allows you to set up a region in which rendering can take place. Coordinates are in pixels. No rendering will be
			 * done to render target pixels outside of the provided region.
			 */
			virtual void EnableScissorTest(const Area2I& area) { EnableScissorTest(area.X, area.Y, area.X + area.Width, area.Y + area.Height); }

			/** Disables scissor test set via EnableScissorTest(). */
			virtual void DisableScissorTest() = 0;

			/**
			 * Sets a reference value that will be used for stencil compare operations.
			 *
			 * @param	value			Reference value to set.
			 */
			virtual void SetStencilReferenceValue(u32 value) = 0;

			/**
			 * Copies the contents of the source buffer to the destination buffer. Caller must ensure the provided offsets and length are within valid bounds of both buffers.
			 * Command buffer must not currently be in a render pass.
			 *
			 * @param	source				Source buffer to copy from.
			 * @param	destination			Destination buffer to copy to.
			 * @param	sourceOffset		Offset into the source buffer, from which to start copying, in bytes.
			 * @param	destinationOffset	Offset into the destination buffer, at which to place the copied data, in bytes.
			 * @param	length				Size of the data to copy, in bytes.
			 */
			virtual void CopyBufferToBuffer(const TShared<GpuBuffer>& source, const TShared<GpuBuffer>& destination, u32 sourceOffset, u32 destinationOffset, u32 length) = 0;

			/**
			 * Copies data from a buffer to a texture subresource. The buffer must contain pixel data in the same format as the texture, accounting for the required
			 * row and depth pitch. Command buffer must not currently be in a render pass.
			 *
			 * @param	source			Source buffer containing pixel data.
			 * @param	destination		Destination texture to copy into.
			 * @param	bufferOffset	Offset into the source buffer, in bytes.
			 * @param	mipLevel		Destination mipmap level.
			 * @param	arrayLayer		Destination texture face (array slice or cubemap face).
			 */
			virtual void CopyBufferToTexture(const TShared<GpuBuffer>& source, const TShared<Texture>& destination, u32 bufferOffset, u32 mipLevel, u32 arrayLayer) = 0;

			/**
			 * Copies data from a texture subresource to a buffer. The buffer must have enough space to receive all pixel data in the same format as the texture,
			 * accounting for the row and depth pitch. Command buffer must not currently be in a render pass.
			 *
			 * @param	source			Source texture to copy from.
			 * @param	destination		Destination buffer to receive pixel data.
			 * @param	mipLevel		Source mipmap level.
			 * @param	arrayLayer		Source texture face (array slice or cubemap face).
			 * @param	bufferOffset	Offset into the destination buffer, in bytes.
			 */
			virtual void CopyTextureToBuffer(const TShared<Texture>& source, const TShared<GpuBuffer>& destination, u32 mipLevel, u32 arrayLayer, u32 bufferOffset = 0) = 0;

			/**
			 * Copies data between texture subresources without format conversion or scaling. Both textures must have matching formats.
			 * For multisampled source textures copying to a non-multisampled destination, a resolve operation is performed.
			 * Command buffer must not currently be in a render pass.
			 *
			 * @param	source			Source texture to copy from.
			 * @param	destination		Destination texture to copy into.
			 * @param	copyInformation	Describes which subresources to copy and where.
			 */
			virtual bool CopyTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureCopyInformation& copyInformation = TextureCopyInformation::kDefault);

			/**
			 * Copies data between texture subresources with optional format conversion and scaling. Uses filtering when scaling.
			 * Does not support multisampled textures. Command buffer must not currently be in a render pass.
			 *
			 * @param	source			Source texture to copy from.
			 * @param	destination		Destination texture to copy into.
			 * @param	blitInformation	Describes which subresources to copy and where, including source/destination regions for scaling.
			 */
			virtual bool BlitTexture(const TShared<Texture>& source, const TShared<Texture>& destination, const TextureBlitInformation& blitInformation = TextureBlitInformation::kDefault);

			/**
			 * Schedules the timestamp to be recorded in the command buffer. The timestamp will record the
			 * time at which the command has been executed by the GPU. The timestamp will be written to the associated
			 * query pool, which should only be accessed when the query pool has resolved the query.
			 * 
			 * @param query			Query to use for referencing the recorded timestamp.
			 * @param queryPool		Query pool that @p query was created from.
			 */
			virtual void WriteTimestamp(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) = 0;

			/**
			 * Schedules the query start in the command buffer. The query will capture information about GPU execution
			 * depending on the query type. Query start operation must be followed by EndQuery(). If a query is started
			 * within a render pass, it must be ended within the same render pass. Queries can also be started outside
			 * of a render pass, in which case they should end outside of a render pass. The query results will be
			 * written to the associated query pool, which should only be accessed when the query pool has resolved the query.
			 *
			 * @param query			Query to use for referencing the recorded data.
			 * @param queryPool		Query pool that @p query was created from.
			 * @param flags			Flags used to control the query.
			 */
			virtual void BeginQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool, GpuQueryFlags flags = GpuQueryFlag::None) = 0;

			/**
			 * Records the timestamp when this particular command executes on the GPU.
			 * 
			 * @param query			Query to use for referencing the recorded data.
			 * @param queryPool		Query pool that @p query was created from.
			 */
			virtual void EndQuery(GpuQueryId query, const TShared<GpuQueryPool>& queryPool) = 0;

			/**
			 * Resets the pool when the command buffer execution reaches this point. After resetting the pool previously allocated queries are no
			 * longer valid, and new AllocateQuery() calls return queries from the start of the pool. Must be done outside of a render pass.
			 */
			virtual void ResetQueries(const TShared<GpuQueryPool>& queryPool) = 0;

			/**
			 * Surrounds all following commands with the provided label, until EndLabel() is called. This may be used by external
			 * tools for easier debugging.
			 */
			virtual void BeginLabel(const StringView& name) = 0;

			/** Closes the label scope as provided by the previous call to BeginLabel(). */
			virtual void EndLabel() = 0;

			/** Inserts a label at the specified location in the command buffer. This may be used by external tools for easier debugging. */
			virtual void InsertLabel(const StringView& name) = 0;

			/** Ends command recording on the command buffer and makes it ready for submission. */
			virtual void End() = 0;

			/**
			 * Ensures that this command buffer will wait for work to finish on all queues specified in the provided mask, before the
			 * command buffer starts executing. This value is utilized at the time the command buffer is submitted to a queue.
			 * Submissions on the same queue are automatically synchronized. The provided mask is OR-ed with existing mask.
			 *
			 * For example if you are performing GPU buffer updates on the transfer (copy) queue, you usually need to wait
			 * for those transfers to finish before you start executing a command buffer that uses that buffer.
			 */
			void AddQueueSyncMask(GpuQueueMask queueMask) { mQueueSyncMask |= queueMask; }

			/** Returns a mask containing all the queues the command buffer needs to wait on before executing. See AddQueueSync. */
			GpuQueueMask GetQueueSyncMask() const { return mQueueSyncMask; }

#if B3D_PROFILING_ENABLED
			/**
			 * Returns a profiler that can be used for profiling calls on this command buffer.
			 *
			 * @name	profilingScopeName		Name of the profiling scope that you may use to identify it when retrieving
			 *									results from GpuProfiler.
			 */
			TShared<GpuCommandBufferProfiler> BeginProfiling(const ProfilerString& profilingScopeName);

			/** Finishes profiling the command buffer. Requested samples will be sent for resolve to GpuProfiler. */
			void EndProfiling();	

			/** Returns the currently active GPU profiler. Only valid in-between Begin/EndProfiling calls. */
			const TShared<GpuCommandBufferProfiler>& GetProfiler() { return mProfiler; }
#endif

			/** Returns the shared pointer to the current object. */
			TShared<GpuCommandBuffer> GetShared() const { return mSelf.lock(); }

			/** Triggers when the command buffer finishes execution on the GPU. triggers on the thread that owns the command buffer (the thread the command buffer pool was created on). */
			Event<void()> OnDidComplete;

			/** Triggered just before a command buffer is about to be destroyed. Provided parameter determines if the command buffer was ever submitted or not. */
			Event<void(bool)> OnDestroyed;

			/**
			 * @name Internal
			 * @{
			 */

			/** Sets a pointer to itself. */
			void SetShared(const TShared<GpuCommandBuffer>& value) { mSelf = value; }

			/** @} */

		protected:
			friend class GpuCommandBufferPool;

			GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueType queueType, const GpuCommandBufferCreateInformation& createInformation);

			/**
			 * Performs internal cleanup of command buffer state without resetting the underlying API command buffer.
			 * This is called by the owning pool during pool-level reset, which resets all command buffers together.
			 * Can also be called internally by individual command buffer Reset() implementations.
			 *
			 * Cleanup includes:
			 * - Resource tracker notifications and clearing
			 * - Queue sync mask reset
			 * - Event clearing (OnDidComplete, OnDestroyed)
			 */
			virtual void Cleanup() {}

			/** Destroys command buffer. Command buffer must not be used after this is called. */
			virtual void Destroy() { mIsDestroyed = true; }

			/** Returns true if the command buffer has been destroyed. */
			bool IsDestroyed() const { return mIsDestroyed; }

			/** Reports an error if the current thread is not the thread associated with the object. */
			void EnsureValidThread() const { B3D_DEBUG_ONLY(B3D_ENSURE(B3D_CURRENT_THREAD_ID == mOwnerThread)); }
	
			GpuDevice& mGpuDevice;
			const GpuCommandBufferCreateInformation mInformation;
			const GpuQueueType mQueueType;
			const ThreadId mOwnerThread;
			String mName;
			GpuCommandBufferState mState = GpuCommandBufferState::Ready;
			GpuQueueMask mQueueSyncMask;
			bool mIsDestroyed = false;

#if B3D_PROFILING_ENABLED
			TShared<GpuCommandBufferProfiler> mProfiler;
			ProfilerString mProfilingScopeName;
#endif

			WeakSPtr<GpuCommandBuffer> mSelf;

		};

		/** @} */
	} // namespace render
} // namespace b3d
