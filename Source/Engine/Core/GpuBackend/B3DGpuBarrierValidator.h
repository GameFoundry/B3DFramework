//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

#if B3D_BUILD_TYPE_DEVELOPMENT

#include "Utility/B3DDenseMap.h"

namespace b3d::render
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Development-only shadow of the resource states that explicit barriers declare. Checks the source of each explicit barrier against
	 * the destination of the previous barrier and the accesses since, and checks each access against the destination of the last barrier.
	 *
	 * A command buffer validates only with a validator set by GpuCommandBuffer::SetValidator(). Command buffers that share one carry the
	 * resource states over from one to the next. The owner of the validator registers alias acquires with DeclareAliasAcquire().
	 * Accesses of a resource before its first barrier are checked against the resource's resting state if gGpuValidateRestingStates is
	 * enabled, and not checked otherwise, but the source of that first barrier is checked against them. With it enabled, the first
	 * barrier of a texture outside a frame graph must also declare a source: the texture is at rest, so it starts from its resting state.
	 *
	 * GpuCommandBuffer feeds it from its public methods, so it checks the contract the caller sees and gives the same result on every
	 * backend.
	 *  - Copies and blits access in GpuStageFlag::Transfer, and a resolving GpuCommandBuffer::CopyTexture() in GpuStageFlag::Resolve.
	 *    Their layout is not checked.
	 *  - Draws and dispatches access the bindings of the bound pipeline in the shader stages that use them. Sampled textures are in
	 *    GpuImageLayout::ShaderReadOnly, unless they are attachments of the render pass, and storage textures in GpuImageLayout::General.
	 *    A draw or dispatch must not access a subresource that an earlier draw or dispatch wrote since its last barrier.
	 *  - Render pass attachments access in GpuStageFlag::ColorAttachment, or the early and late fragment tests. See
	 *    GpuBackendUtility::GetRenderPassAttachmentLayout() for their layouts.
	 *  - Resources must not be accessed or barriered while they are unplaced (see render::Texture::SetUnplaced()). A resource with a
	 *    declared alias acquire must not be accessed or barriered before it, and the resources it supersedes not after it.
	 */
	class B3D_EXPORT GpuBarrierValidator
	{
	public:
		/**
		 * Starts validating a command buffer of queues of @p queueType. Forgets the bindings of the previous command buffer, and keeps
		 * the resource states. The validator keeps the bound objects alive until then, or until Clear().
		 */
		void BeginCommandBuffer(GpuQueueType queueType);

		/**
		 * Checks explicit barriers, and declares their destination.
		 *
		 * @param	barriers	Barriers to check.
		 * @param	phase		Part of the barriers the command buffer records.
		 * @param	split		Split barrier connecting the halves. Null for GpuBarrierPhase::Full.
		 */
		void ValidateBarriers(const GpuBarriers& barriers, GpuBarrierPhase phase, const GpuSplitBarrier* split);

		/**
		 * Declares that @p buffer starts a new lifetime on memory that earlier resources used, as @p acquire describes. The first barrier
		 * of the buffer must be an alias acquire whose source covers GpuAliasAcquire::Source, and must precede every access. Once it is
		 * recorded, the resources in GpuAliasAcquire::Predecessors must not be accessed or barriered, nor barriered together with it,
		 * until their own alias acquire. Their GpuBarrierFlag::AliasRelease barriers are the exception: they may be recorded together
		 * with the alias acquire, or after it as the acquire half of a split barrier. The buffer must be placed.
		 */
		void DeclareAliasAcquire(const GpuBuffer& buffer, const GpuAliasAcquire& acquire);

		/** @copydoc DeclareAliasAcquire(const GpuBuffer&, const GpuAliasAcquire&) */
		void DeclareAliasAcquire(const Texture& texture, const GpuAliasAcquire& acquire);

		/** Remembers a parameter set bound by GpuCommandBuffer::SetGpuParameterSet(). */
		void SetParameterSet(const TShared<GpuParameterSet>& parameters);

		/** Remembers the pipeline bound by GpuCommandBuffer::SetGpuGraphicsPipelineState(). */
		void SetGraphicsPipeline(const TShared<GpuGraphicsPipelineState>& pipeline) { mGraphicsPipeline = pipeline; }

		/** Remembers the pipeline bound by GpuCommandBuffer::SetGpuComputePipelineState(). */
		void SetComputePipeline(const TShared<GpuComputePipelineState>& pipeline) { mComputePipeline = pipeline; }

		/** Remembers vertex buffers bound by GpuCommandBuffer::SetVertexBuffers(). */
		void SetVertexBuffers(u32 index, const TShared<GpuBuffer>* buffers, u32 bufferCount);

		/** Remembers the index buffer bound by GpuCommandBuffer::SetIndexBuffer(). */
		void SetIndexBuffer(const TShared<GpuBuffer>& buffer) { mIndexBuffer = buffer; }

		/** Checks the attachment accesses of a render pass, and remembers its attachments for the draws in it. */
		void ValidateRenderPass(const RenderPassCreateInformation& createInformation);

		/** Checks the accesses of a draw: the bindings of the graphics pipeline, the vertex buffers and, if @p indexed, the index buffer. */
		void ValidateDraw(bool indexed);

		/** Checks the accesses of a dispatch: the bindings of the compute pipeline. */
		void ValidateDispatch();

		/** Checks an access of a buffer in @p stages against the declared state, and records it for the next barrier. */
		void ValidateBufferAccess(const GpuBuffer& buffer, GpuStageFlags stages, GpuAccessFlags access);

		/**
		 * Checks an access of a range of texture subresources in @p stages against the declared state, and records it for the next
		 * barrier. @p layout is only checked if it is not GpuImageLayout::Undefined.
		 */
		void ValidateTextureAccess(const Texture& texture, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/**
		 * Checks an access of a render target surface in @p stages against the declared state, and records it for the next barrier. @p layout
		 * is only checked if it is not GpuImageLayout::Undefined.
		 */
		void ValidateRenderTargetAccess(const RenderTarget& target, RenderSurfaceMaskBits surface, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/** Forgets every state, declaration and binding. */
		void Clear();

	private:
		/** State of a buffer or an image subresource. */
		struct ResourceState
		{
			GpuAccessState Declared; /**< Destination of the last barrier. Valid if IsDeclared is true. */
			GpuStageFlags UsedStages; /**< Stages that accessed the resource since the last barrier. */
			GpuAccessFlags UsedAccess; /**< How the resource was accessed since the last barrier. */
			const GpuSplitBarrier* PendingAcquire = nullptr; /**< Split barrier released, but not acquired yet. */
			u32 WriterOperation = 0; /**< Draw or dispatch that wrote the resource since the last barrier, or 0 if none did. */
			bool IsDeclared = false;
		};

		/** Identifies an image: a texture, or a surface of a window, which has no texture. */
		struct ImageKey
		{
			bool operator==(const ImageKey& other) const { return Object == other.Object && Surface == other.Surface; }

			const void* Object = nullptr; /**< Texture or window render target. */
			u32 Surface = 0; /**< For a window, 0 for the color surface and 1 for depth and stencil. */
		};

		/** Hashes an ImageKey. */
		struct ImageKeyHash
		{
			size_t operator()(const ImageKey& key) const
			{
				size_t hash = 0;
				B3DCombineHash(hash, key.Object);
				B3DCombineHash(hash, key.Surface);

				return hash;
			}
		};

		/** Subresources of an image accessed by an operation, along with the identity, name, full range and resting state of the image. */
		struct ImageSubresources
		{
			ImageKey Key;
			GpuTextureSubresourceRange FullRange; /**< Every subresource of the image. */
			GpuTextureSubresourceRange Range; /**< Subresources accessed by the operation, within FullRange. */
			StringView Name;
			GpuAccessState RestingState;
			const Texture* Texture = nullptr; /**< Null for a window surface. */
		};

		/** Alias acquire declared with DeclareAliasAcquire(), and not recorded yet. */
		struct DeclaredAliasAcquire
		{
			GpuAccessScope Source;
			TInlineArray<const IGpuResource*, 4> Predecessors;
		};

		/** Forgets the bindings and the render pass of the command buffer. */
		void ClearBindings();

		/** Registers @p acquire for the resource @p gpuResource. See DeclareAliasAcquire(). */
		void DeclareAliasAcquire(const IGpuResource* gpuResource, const GpuAliasAcquire& acquire, StringView name);

		/** Starts the lifetimes of the resources that @p barriers alias acquire, which supersedes their predecessors. */
		void StartAliasLifetimes(const GpuBarriers& barriers);

		/** Returns the subresources of a texture in @p range, clamped to the texture. */
		ImageSubresources GetTextureSubresources(const Texture& texture, const GpuTextureSubresourceRange& range) const;

		/** Returns the subresources of a single-bit @p surface of @p target, over the whole surface. Returns a null key if the target has no such surface. */
		ImageSubresources GetSurfaceSubresources(const RenderTarget& target, RenderSurfaceMaskBits surface) const;

		/** Checks the accesses of the bindings of @p layout. Draws pass @p isDraw, so sampled attachments of the render pass skip the layout check. */
		void ValidateBindings(const GpuPipelineParameterLayout& pipelineLayout, bool isDraw);

		/** Checks an explicit barrier of an image. */
		void ValidateImageBarrier(const ImageSubresources& image, const GpuBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split);

		/** Checks an explicit barrier of a buffer. */
		void ValidateBufferBarrier(const GpuBuffer& buffer, const GpuBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split);

		/** Checks an access of an image. */
		void ValidateImageAccess(const ImageSubresources& image, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/** Returns true if @p image overlaps an attachment of the last render pass. */
		bool IsRenderPassAttachment(const ImageSubresources& image) const;

		/** Returns the state accesses of a resource that no barrier declared are checked against, or null if they are not checked. */
		static const GpuAccessState* GetUndeclaredState(const GpuAccessState& restingState);

		/**
		 * Returns an error message if a resource must not be used on the GPU at all, or null if it may. @p isUnplaced is true if a
		 * transient resource allocator returned the resource without memory, and @p gpuResource is its native resource, if any.
		 * @p barrier is the barrier that uses the resource, or null for an access.
		 */
		const char* GetLifetimeError(bool isUnplaced, const IGpuResource* gpuResource, const GpuBarrier* barrier, GpuBarrierPhase phase) const;

		/** Checks @p barrier against @p state and applies it. Returns an error message, or null if the barrier is valid. */
		static const char* ApplyBarrier(ResourceState& state, const GpuBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split, bool isImage);

		/**
		 * Checks an access against @p state and records it. Returns an error message, or null if the access is valid.
		 *
		 * @param	state				State of the accessed resource.
		 * @param	layout				Layout of the access, or GpuImageLayout::Undefined if it is not checked.
		 * @param	stages				Stages of the access.
		 * @param	access				Reads and writes of the access.
		 * @param	undeclaredState		State to check the access against if no barrier declared the resource's state. Null to skip the check.
		 * @param	operation			Draw or dispatch performing the access, or 0 for other accesses.
		 */
		static const char* ApplyAccess(ResourceState& state, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access, const GpuAccessState* undeclaredState, u32 operation);

		/** Calls @p function with the state of each subresource of @p image. Returns the first error message it returns. */
		template<class TFunction>
		const char* ForEachSubresourceState(const ImageSubresources& image, TFunction&& function);

		GpuQueueType mQueueType = GQT_GRAPHICS; /**< Queue type of the command buffer being validated. */
		u32 mOperationCount = 0; /**< Number of draws and dispatches validated so far. */
		u32 mOperation = 0; /**< Draw or dispatch whose accesses are being validated, or 0 outside of one. */

		TDenseMap<const GpuBuffer*, ResourceState> mBuffers;
		UnorderedMap<ImageKey, Vector<ResourceState>, ImageKeyHash> mImages; /**< One state per face, mip level and aspect. */
		UnorderedMap<const IGpuResource*, DeclaredAliasAcquire> mAliasAcquires; /**< By the resource that records the alias acquire. */
		UnorderedSet<const IGpuResource*> mSupersededResources; /**< Resources whose memory an alias acquire of another resource took over. */

		Vector<TShared<GpuParameterSet>> mParameterSets; /**< Bound parameter sets, by set index. */
		TShared<GpuGraphicsPipelineState> mGraphicsPipeline;
		TShared<GpuComputePipelineState> mComputePipeline;
		Vector<TShared<GpuBuffer>> mVertexBuffers;
		TShared<GpuBuffer> mIndexBuffer;
		TInlineArray<ImageSubresources, B3D_MAXIMUM_RENDER_TARGET_COUNT + 2> mRenderPassAttachments; /**< Attachments of the last render pass. */
	};

	/** @} */
} // namespace b3d::render

#endif
