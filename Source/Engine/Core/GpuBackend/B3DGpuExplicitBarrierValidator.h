//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

#if B3D_GPU_EXPLICIT_BARRIERS && B3D_BUILD_TYPE_DEVELOPMENT

#include "Utility/B3DDenseMap.h"

namespace b3d::render
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Development-only shadow of the resource states that explicit barriers declare on one command buffer. Checks the source of each
	 * explicit barrier against the destination of the previous barrier and the accesses since, and checks each access against the
	 * destination of the last barrier. Accesses before the first barrier of a resource on the command buffer are not checked, because
	 * an earlier command buffer declared their state, but the source of that first barrier is checked against them.
	 *
	 * GpuCommandBuffer feeds it from its public methods, so it checks the contract the caller sees and gives the same result on every
	 * backend.
	 *  - Copies and blits access in GpuStageFlag::Transfer, and a resolving GpuCommandBuffer::CopyTexture() in GpuStageFlag::Resolve.
	 *    Their layout is not checked.
	 *  - Draws and dispatches access the bindings of the bound pipeline in the shader stages that use them. Sampled textures are in
	 *    GpuImageLayout::ShaderReadOnly, unless they are attachments of the render pass, and storage textures in GpuImageLayout::General.
	 *  - Render pass attachments access in GpuStageFlag::ColorAttachment, or the early and late fragment tests. See GetAttachmentLayout()
	 *    for their layouts.
	 */
	class B3D_EXPORT GpuExplicitBarrierValidator
	{
	public:
		/**
		 * Checks explicit barriers, and declares their destination.
		 *
		 * @param	barriers	Barriers to check.
		 * @param	phase		Part of the barriers the command buffer records.
		 * @param	split		Split barrier connecting the halves. Null for GpuBarrierPhase::Full.
		 */
		void ValidateBarriers(const GpuExplicitBarriers& barriers, GpuBarrierPhase phase, const GpuSplitBarrier* split);

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

		/** Forgets every state and binding. Call when the command buffer is reset. */
		void Clear();

		/**
		 * Returns the layout explicit barriers must declare for an attachment of a render pass, given its surfaces that are read-only:
		 *  - Color: GpuImageLayout::ColorAttachment, or GpuImageLayout::General if read-only.
		 *  - Depth and stencil: GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthReadOnlyStencilAttachment,
		 *    GpuImageLayout::DepthAttachmentStencilReadOnly or GpuImageLayout::DepthStencilReadOnly, depending on which aspects are read-only.
		 */
		static GpuImageLayout GetAttachmentLayout(RenderSurfaceMaskBits surface, RenderSurfaceMask readOnlyMask);

	private:
		/** State of a buffer or an image subresource on the command buffer. */
		struct ResourceState
		{
			GpuAccessState Declared; /**< Destination of the last barrier. Valid if IsDeclared is true. */
			GpuStageFlags UsedStages; /**< Stages that accessed the resource since the last barrier. */
			GpuAccessFlags UsedAccess; /**< How the resource was accessed since the last barrier. */
			const GpuSplitBarrier* PendingAcquire = nullptr; /**< Split barrier this command buffer released, but did not acquire yet. */
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

		/** Subresources of an image accessed by an operation, along with the identity, name and full range of the image. */
		struct ImageSubresources
		{
			ImageKey Key;
			GpuTextureSubresourceRange FullRange; /**< Every subresource of the image. */
			GpuTextureSubresourceRange Range; /**< Subresources accessed by the operation, within FullRange. */
			StringView Name;
		};

		/** Returns the subresources of a texture in @p range, clamped to the texture. */
		static ImageSubresources GetTextureSubresources(const Texture& texture, const GpuTextureSubresourceRange& range);

		/** Returns the subresources of a single-bit @p surface of @p target, over the whole surface. Returns a null key if the target has no such surface. */
		static ImageSubresources GetSurfaceSubresources(const RenderTarget& target, RenderSurfaceMaskBits surface);

		/** Checks the accesses of the bindings of @p layout. Draws pass @p isDraw, so sampled attachments of the render pass skip the layout check. */
		void ValidateBindings(const GpuPipelineParameterLayout& pipelineLayout, bool isDraw);

		/** Checks an explicit barrier of an image. */
		void ValidateImageBarrier(const ImageSubresources& image, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split);

		/** Checks an access of an image. */
		void ValidateImageAccess(const ImageSubresources& image, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/** Returns true if @p image overlaps an attachment of the last render pass. */
		bool IsRenderPassAttachment(const ImageSubresources& image) const;

		/** Checks @p barrier against @p state and applies it. Returns an error message, or null if the barrier is valid. */
		static const char* ApplyBarrier(ResourceState& state, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split, bool isImage);

		/** Checks an access against @p state and records it. Returns an error message, or null if the access is valid. */
		static const char* ApplyAccess(ResourceState& state, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/** Calls @p function with the state of each subresource of @p image. Returns the first error message it returns. */
		template<class TFunction>
		const char* ForEachSubresourceState(const ImageSubresources& image, TFunction&& function);

		TDenseMap<const GpuBuffer*, ResourceState> mBuffers;
		UnorderedMap<ImageKey, Vector<ResourceState>, ImageKeyHash> mImages; /**< One state per face, mip level and aspect. */

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
