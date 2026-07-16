//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DIMetalRenderWindowSurface.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "B3DMetalUtility.h"
#include "Image/B3DPixelData.h"
#include "Debug/B3DLog.h"

namespace b3d::render
{
	TAsyncOp<TShared<PixelData>> IMetalRenderWindowSurface::ReadAsync(GpuCommandBuffer& commandBuffer)
	{
		@autoreleasepool
		{
			TAsyncOp<TShared<PixelData>> op;

			id<MTLTexture> colorTexture = GetCurrentColorTexture();
			if (colorTexture == nil)
			{
				// No back buffer to read; signal "no frame produced" by completing with nullptr. Callers
				// (snapshot tests, capture paths) already treat nullptr as the no-frame code path.
				op.CompleteOperation(nullptr);
				return op;
			}

			if (colorTexture.framebufferOnly == YES)
			{
				// Defense in depth: the windowed surface configures its CAMetalLayer with framebufferOnly=NO so
				// the drawable texture can be used as a blit source. If a future change ever flips this to YES
				// the blit below silently produces undefined bytes on Apple Silicon and fails Metal API
				// validation. Surface the error unambiguously instead.
				B3D_LOG(Error, LogRenderBackend, "ReadAsync requires a blit-sampleable color texture (framebufferOnly must be NO).");
				op.CompleteOperation(nullptr);
				return op;
			}

			auto& metalCommandBuffer = static_cast<MetalGpuCommandBuffer&>(commandBuffer);
			id<MTLBlitCommandEncoder> blit = metalCommandBuffer.GetOrOpenBlitEncoder();
			if (blit == nil)
			{
				op.CompleteOperation(nullptr);
				return op;
			}

			// Derive width/height from the texture itself — it is the authoritative size the blit must use, and
			// reading it avoids racing the surface's render-thread-owned size fields.
			const u32 width = (u32)colorTexture.width;
			const u32 height = (u32)colorTexture.height;
			const PixelFormat pixelFormat = GetColorPixelFormat();

			TShared<PixelData> pixelData = PixelData::Create(width, height, 1, pixelFormat);

			const u32 bytesPerRow = MetalUtility::GetTextureRowPitch(pixelFormat, width);
			const NSUInteger bufferSize = (NSUInteger)bytesPerRow * height;

			auto& metalDevice = static_cast<MetalGpuDevice&>(commandBuffer.GetGpuDevice());
			id<MTLBuffer> stagingBuffer = [metalDevice.GetMetalDevice() newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
			if (stagingBuffer == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "ReadAsync: failed to allocate {0}-byte staging buffer.", (u64)bufferSize);
				op.CompleteOperation(nullptr);
				return op;
			}

			[blit copyFromTexture:colorTexture
				sourceSlice:0
				sourceLevel:0
				sourceOrigin:MTLOriginMake(0, 0, 0)
				sourceSize:MTLSizeMake(width, height, 1)
				toBuffer:stagingBuffer
				destinationOffset:0
				destinationBytesPerRow:bytesPerRow
				destinationBytesPerImage:bufferSize];

			// Completion hooks through the engine-level command buffer signal rather than Metal's raw
			// addCompletedHandler:, so the memcpy runs on the pool-owner engine thread the caller expects —
			// identical to how the Vulkan backend consumes this signal in B3DIVulkanRenderWindowSurface.cpp.
			// The shared-storage staging buffer is CPU-coherent the moment the GPU blit retires, which is
			// strictly before OnDidComplete fires, so no explicit synchronize step is required.
			//
			// In non-ARC builds stagingBuffer carries the +1 from newBufferWithLength:. Exactly one of the two
			// handlers below releases it — OnDidComplete after the copy-out, or OnDestroyed when the command
			// buffer is discarded without ever being submitted (OnDestroyed with isSubmitted=true early-outs,
			// so a completed-then-destroyed sequence cannot double-release). The encoded blit additionally
			// retains the buffer for the GPU's lifetime of the command buffer (default retained-references
			// mode), so releasing on the engine thread is always safe.
			const u32 resultRowPitch = pixelData->GetRowPitch();
			const u32 resultHeight = height;

			auto fnOnCommandBufferCompleted = [stagingBuffer, op, pixelData, bytesPerRow, resultRowPitch, resultHeight]() mutable
			{
				const u8* src = (const u8*)[stagingBuffer contents];
				u8* dst = pixelData->GetData();
				if (src != nullptr && dst != nullptr)
				{
					if (resultRowPitch == bytesPerRow)
						memcpy(dst, src, (size_t)bytesPerRow * resultHeight);
					else
					{
						for (u32 row = 0; row < resultHeight; row++)
							memcpy(dst + row * resultRowPitch, src + row * bytesPerRow, bytesPerRow);
					}
				}

#if !__has_feature(objc_arc)
				[stagingBuffer release];
#endif
				op.CompleteOperation(pixelData);
			};

			auto fnOnCommandBufferDestroyed = [stagingBuffer, op](bool isSubmitted) mutable
			{
				if (isSubmitted)
					return;

#if !__has_feature(objc_arc)
				[stagingBuffer release];
#endif
				op.CompleteOperation(nullptr);
			};

			commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
			commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

			return op;
		}
	}
} // namespace b3d::render
