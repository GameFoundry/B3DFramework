//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

namespace b3d
{
	struct RenderTargetClearValues;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Collection of static utility methods shared between GPU backend implementations. */
	class B3D_EXPORT GpuBackendUtility
	{
	public:
		/**
		 * Calculates the size and alignment of a single element within a shader interface block using the std140 layout.
		 *
		 * @param[in]		type		Type of the element. Structs are not supported.
		 * @param[in]		arraySize	Number of array elements of the element (1 if it's not an array).
		 * @param[in, out]	offset		Current location in some parent buffer at which the element should be placed at. If the
		 *								location doesn't match the element's alignment, the value will be modified to a valid
		 *								alignment. In multiples of 4 bytes.
		 * @return						Size of the element, in multiples of 4 bytes.
		 */	
		static u32 CalcStd140MemberSizeAndOffset(GpuDataParameterType type, u32 arraySize, u32& offset)
		{
			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[type];
			u32 size = (typeInfo.BaseTypeSize * typeInfo.NumColumns * typeInfo.NumRows) / 4;
			const u32 alignment = typeInfo.Alignment / 4;

			if (alignment > 0)
			{
				const u32 alignOffset = offset % alignment;
				if (alignOffset != 0)
					offset += (alignment - alignOffset);
			}

			if (arraySize > 1)
			{
				// Array elements are always padded and aligned to vec4.
				const u32 sizeRemainder = size % 4;
				if (sizeRemainder != 0)
					size += (4 - sizeRemainder);

				const u32 offsetRemainder = offset % 4;
				if (offsetRemainder != 0)
					offset += (4 - offsetRemainder);
			}

			return size;
		}

		/** Maps resource usage flags to the stages describing on which pipeline stage and how the resource is accessed. */
		static render::GpuStageFlags GetStageFlags(render::GpuResourceUseFlags usage);

		/**
		 * Maps the program stages using a shader binding to the shader stage flags used by resource tracking. Returns
		 * GpuResourceUseFlag::Undefined if @p stages contains no shader stages.
		 */
		static render::GpuResourceUseFlags GetShaderResourceUseFlags(GpuProgramStageBits stages);

		/** Returns the stages that queues of @p queueType can execute. */
		static render::GpuStageFlags GetQueueStageFlags(GpuQueueType queueType);

		/**
		 * Returns true if textures created with @p usage are sampleable only: not render targets, depth-stencil targets, UAVs or
		 * CPU-writable. Backends share such textures between queues.
		 */
		static bool IsSampleableOnly(TextureUsageFlags usage);

		/**
		 * Returns true if textures created with @p usage can be read on several queues at once: they allow concurrent queue reads, or are
		 * sampleable only. Backends create such textures so that no queue owns them. Other textures rest on the graphics queue, and must
		 * be transferred to be accessed on another queue.
		 */
		static bool IsConcurrentlyReadable(TextureUsageFlags usage);

		/** Converts a single GpuStageFlag into a readable string representing the access and stage. */
		static const char* GetAccessStageName(render::GpuStageFlag flag);

		/** Appends every bit set in @p flags to @p output as readable stage names, using "|" as separator. */
		static void GetAccessStageNames(render::GpuStageFlags flags, StringStream& output);

		/** Checks if the two image subresource ranges have any overlapping subresources. */
		static bool RangeOverlaps(const GpuTextureSubresourceRange& a, const GpuTextureSubresourceRange& b);

		/** Checks if the two image subresource ranges cover the same aspects, mip levels and array layers. */
		static bool RangeEquals(const GpuTextureSubresourceRange& a, const GpuTextureSubresourceRange& b);

		/** Returns the smallest range that contains both @p a and @p b, including the aspects of both. */
		static GpuTextureSubresourceRange GetBoundingRange(const GpuTextureSubresourceRange& a, const GpuTextureSubresourceRange& b);

		/** Returns true if @p outer contains every subresource of @p inner. */
		static bool RangeContains(const GpuTextureSubresourceRange& outer, const GpuTextureSubresourceRange& inner);

		/** Returns the aspects of an image with the engine pixel @p format: depth, depth and stencil, or color. */
		static GpuTextureAspectFlags GetFormatAspects(PixelFormat format);

		/** Returns @p range with "remaining" counts resolved and aspects limited to those of @p imageRange, the full range of the image. */
		static GpuTextureSubresourceRange ClampRange(GpuTextureSubresourceRange range, const GpuTextureSubresourceRange& imageRange);

		/**
		 * Returns the subresource range covering the faces and mip levels described by @p surface, over every aspect of
		 * @p imageRange, the full range of the image. A zero face or mip count covers everything from the first face or mip
		 * onward, and explicit counts are clamped to the subresources that remain.
		 */
		static GpuTextureSubresourceRange GetSurfaceRange(const GpuTextureSubresourceRange& imageRange, const TextureSurface& surface);

		/**
		 * Subdivides an image subresource range by cutting it with another range. If the ranges don't overlap, or the
		 * @p cutWith range completely covers the @p toCut range, the original @p toCut range is output.
		 *
		 * @param[in]	toCut		Range to cut.
		 * @param[in]	cutWith		Range to cut with.
		 * @param[out]	output		Pieces of the range that was cut.
		 * @param[out]	numAreas	Number of pieces in the @p output array.
		 */
		static void CutRange(const GpuTextureSubresourceRange& toCut, const GpuTextureSubresourceRange& cutWith, std::array<GpuTextureSubresourceRange, 5>& output, u32& numAreas);

		/** Converts a GpuImageLayout enum value to a readable string representing the layout name. */
		static const char* GetImageLayoutName(render::GpuImageLayout layout);

		/**
		 * Records a copy of a subresource of @p texture into @p stagingBuffer, which the host can read once @p commandBuffer completes. The
		 * subresource and @p stagingBuffer leave their resting states for the copy and return to them.
		 */
		static void CopyToStagingBuffer(render::GpuCommandBuffer& commandBuffer, const TShared<render::Texture>& texture, u32 mipLevel, u32 arrayLayer, const TShared<render::GpuBuffer>& stagingBuffer);

		/**
		 * Records a copy of @p length bytes of @p buffer, starting at @p offset, to the start of @p stagingBuffer, which the host can read
		 * once @p commandBuffer completes. @p buffer and @p stagingBuffer leave their resting states for the copy and return to them.
		 */
		static void CopyToStagingBuffer(render::GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& buffer, u32 offset, u32 length, const TShared<render::GpuBuffer>& stagingBuffer);

		/**
		 * Records a copy of the back-buffer image a window's surface holds into @p stagingBuffer (see GpuCommandBuffer::CopyWindowToBuffer()),
		 * which the host can read once @p commandBuffer completes. The image and @p stagingBuffer leave their resting states for the copy
		 * and return to them.
		 */
		static void CopyToStagingBuffer(render::GpuCommandBuffer& commandBuffer, const TShared<render::RenderWindow>& window, const TShared<render::GpuBuffer>& stagingBuffer);

		/**
		 * Returns the resting state of a buffer on queues of @p queueType. A resource outside of the work that declares its state is at
		 * rest: it may be read in the resting state without a barrier, and whoever writes it brings it back to
		 * rest. Buffers rest readable as vertex, index and indirect arguments, and from every shader stage. Readback buffers
		 * (GpuBufferType::StagingRead) rest readable by the host, so a barrier that returns one to rest makes the GPU's writes visible
		 * to the host. Upload buffers (GpuBufferType::StagingWrite) rest readable by copies, the only GPU work that reads them.
		 */
		static render::GpuAccessState GetRestingState(const render::GpuBuffer& buffer, GpuQueueType queueType);

		/**
		 * Returns the resting state of a texture on queues of @p queueType: sampled from every shader stage in
		 * GpuImageLayout::ShaderReadOnly. Textures that can be render targets, depth-stencil targets or storage textures, which may have
		 * compression metadata, rest with GpuCompressionHint::Compressed: compressed in an encoding sampling can read, without fast-clear
		 * markers. Other textures have no compression metadata, and rest with GpuCompressionHint::Unknown. See
		 * GetRestingState(const render::GpuBuffer&, GpuQueueType).
		 */
		static render::GpuAccessState GetRestingState(const render::Texture& texture, GpuQueueType queueType);

		/**
		 * Returns the resting state of the single-bit @p surface of @p target, on queues of @p queueType. A render texture surface rests
		 * like its texture. A window surface can only be accessed as an attachment: its color surface rests as returned by
		 * GetWindowColorRestingState(), and its depth and stencil surfaces in GpuImageLayout::DepthStencilAttachment.
		 */
		static render::GpuAccessState GetRestingState(const render::RenderTarget& target, RenderSurfaceMaskBits surface, GpuQueueType queueType);

		/**
		 * Returns the resting state of the color surface of a window on queues of @p queueType: read and written by color attachments, in
		 * GpuImageLayout::ColorAttachment. Presentation takes the surface out of rest into GpuImageLayout::Present, and the next
		 * submission that uses it brings it back (see IGpuImageResource::RequestRestingTransition()).
		 */
		static render::GpuAccessState GetWindowColorRestingState(GpuQueueType queueType);

		/**
		 * Returns the layout of an attachment during a render pass, given the surfaces of the render pass that are read-only. Explicit
		 * barriers transition attachments to it before the render pass:
		 *  - Color: GpuImageLayout::ColorAttachment, or GpuImageLayout::General if read-only.
		 *  - Depth and stencil: GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthReadOnlyStencilAttachment,
		 *    GpuImageLayout::DepthAttachmentStencilReadOnly or GpuImageLayout::DepthStencilReadOnly, depending on which aspects are read-only.
		 */
		static render::GpuImageLayout GetRenderPassAttachmentLayout(RenderSurfaceMaskBits surface, RenderSurfaceMask readOnlyMask);

		/** Finds the attachment among @p attachments overlapping @p range of @p image, or returns null. */
		static const render::GpuRenderPassAttachmentUsage* FindRenderPassAttachment(TArrayView<const render::GpuRenderPassAttachmentUsage> attachments,
			IGpuImageResource* image, const GpuTextureSubresourceRange& range);

		/**
		 * Returns the layout a shader read of the specified image range is performed in, in a render pass with @p attachments. Reads of an
		 * attachment use the attachment's layout, all other reads use @p requestedLayout.
		 */
		static render::GpuImageLayout ResolveShaderImageLayout(TArrayView<const render::GpuRenderPassAttachmentUsage> attachments, IGpuImageResource* image,
			const GpuTextureSubresourceRange& subresourceRange, render::GpuImageLayout requestedLayout);

		/**
		 * Returns the surfaces that a render pass clears when it begins, and writes the values to clear them to into @p clearValues. These
		 * are the surfaces in RenderPassCreateInformation::ClearMask, with the clear values of the render target. If
		 * gGpuPoisonUnloadedAttachments is enabled, development builds also clear every surface the render pass neither loads nor clears:
		 * color to magenta, depth and stencil to 0. Backends load depth and stencil together, so neither is cleared if the pass loads either.
		 * Read-only surfaces are never cleared, and must be loaded.
		 */
		static RenderSurfaceMask GetRenderPassClears(const render::RenderPassCreateInformation& createInformation, RenderTargetClearValues& clearValues);
	};

	/** @} */
} // namespace b3d
