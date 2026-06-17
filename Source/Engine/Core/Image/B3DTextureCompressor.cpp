//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DTextureCompressor.h"

#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DTexture.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DAsyncOp.h"
#include "FileSystem/B3DDataStream.h"
#include "B3DApplication.h"

#include <cstring>
#include <cfloat>

using namespace b3d;

namespace b3d::render
{
	namespace
	{
		/** Per-dispatch constants for the block-compression kernel (maps to the BSL `cbuffer Parameters`). */
		B3D_UNIFORM_BUFFER_BEGIN(TextureCompressParameterDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gTextureSize)  // Texture size in pixels
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gBlockCount)   // Number of 4x4 blocks along each axis (whole texture)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gBlockOffset)  // Block-grid offset of the dispatched band (X is always 0)
		B3D_UNIFORM_BUFFER_END

		TextureCompressParameterDefinition gTextureCompressParameters;
	}
}

namespace b3d
{
	namespace
	{
		/**
		 * Maps a block-compressed pixel format to the TextureCompress.bsl FORMAT variation index, the output texture format
		 * (one packed block per texel: RG32U for 64-bit blocks, RGBA32U for 128-bit), whether the source must be fed as HDR
		 * floating-point (BC6H) rather than normalized RGBA8, and a per-format band-height multiplier.
		 *
		 * @p tileSizeMultiplier scales the caller's maxTileSize band budget by how much GPU work this format does per block,
		 * so each band's dispatch stays well under the OS GPU watchdog (TDR) regardless of format.
		 */
		bool GetFormatInfo(PixelFormat format, i32& outVariation, PixelFormat& outPixelFormat, bool& outIsHDR, float& outTileSizeMultiplier)
		{
			outIsHDR = false;
			switch(format)
			{
			case PF_BC1:
			case PF_BC1a:
				outVariation = 0;
				outPixelFormat = PF_RG32U; // 64-bit block
				outTileSizeMultiplier = 1.0f;
				return true;
			case PF_BC3:
				outVariation = 1;
				outPixelFormat = PF_RGBA32U; // 128-bit block
				outTileSizeMultiplier = 1.0f;
				return true;
			case PF_BC4:
				outVariation = 2;
				outPixelFormat = PF_RG32U; // 64-bit block
				outTileSizeMultiplier = 4.0f; // Trivial min/max encode, allow larger tiles
				return true;
			case PF_BC5:
				outVariation = 3;
				outPixelFormat = PF_RGBA32U; // 128-bit block
				outTileSizeMultiplier = 4.0f; // Trivial min/max encode, allow larger tiles
				return true;
			case PF_BC6H:
				outVariation = 4; // base of the BC6H mode range (variations 4..17, encoder modes 1..14)
				outPixelFormat = PF_RGBA32U; // 128-bit block
				outIsHDR = true; // HDR source: fed as RGBA32F, not normalized RGBA8
				outTileSizeMultiplier = 0.25f; // Multiple modes per tile, expensive, use smaller tile
				return true;
			case PF_BC7:
				outVariation = 18; // base of the BC7 mode range (variations 18..25, encoder modes 0..7)
				outPixelFormat = PF_RGBA32U; // 128-bit block
				outTileSizeMultiplier = 0.25f; // Multiple modes per tile, expensive, use smaller tile
				return true;
			default:
				return false;
			}
		}

		/** RendererMaterial wrapper around the block-compression compute shader. */
		class TextureCompressMaterial : public render::RendererMaterial<TextureCompressMaterial>
		{
			RMAT_DEF("TextureCompress.bsl")

		public:
			TextureCompressMaterial() = default;

			void Initialize() override
			{
				mGpuParameterSet->TryGetUniformBufferParameter("Parameters", mParametersBuffer);
			}

			/**
			 * Records the dispatch that compresses @p input into @p output. Must run on the render thread. @p bestErr is the
			 * per-block running-best-error buffer shared by the BC7 / BC6H per-mode dispatches; pass null for single-dispatch
			 * formats (the variation's shader will not declare gBestErr in that case).
			 */
			void Execute(render::GpuCommandBuffer& commandBuffer, const TShared<render::Texture>& input, const TShared<render::Texture>& output, const render::GpuBufferMappedScope& parameters, const TShared<render::Texture>& bestError, const Vector2I& blockCount, i32 variation)
			{
				mGpuParameterSet->SetSampledTexture("gInput", input);
				mGpuParameterSet->SetStorageTexture("gOutput", output, TextureSurface::kComplete);
				mParametersBuffer.Set(parameters);

				if(bestError != nullptr && mGpuParameterSet->HasStorageTexture("gBestErr"))
					mGpuParameterSet->SetStorageTexture("gBestErr", bestError, TextureSurface::kComplete);

				Bind(commandBuffer);

				// BC6H two-region modes (variations 4..13) dispatch one thread-group per 4x4 block, with 32 threads (one per
				// partition) cooperating via groupshared (kernel uses [numthreads(32, 1, 1)]). Every other variation runs one
				// thread per block ([numthreads(8, 8, 1)], 8x8 groups).
				if(variation >= 4 && variation <= 13)
					commandBuffer.DispatchCompute((u32)blockCount.X, (u32)blockCount.Y);
				else
					commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(blockCount.X, 8), (u32)Math::DivideAndRoundUp(blockCount.Y, 8));
			}

			template <int FORMAT>
			static const ShaderVariationParameters& GetVariationParams()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters({ ShaderVariationParameter("FORMAT", FORMAT) });

				return variation;
			}

			/** Returns the material for a single FORMAT variation value (0..25; see TextureCompress.bsl FORMAT note). */
			static TextureCompressMaterial* GetVariation(i32 variation)
			{
				switch(variation)
				{
				case 0: return Get(GetVariationParams<0>());
				case 1: return Get(GetVariationParams<1>());
				case 2: return Get(GetVariationParams<2>());
				case 3: return Get(GetVariationParams<3>());
				case 4: return Get(GetVariationParams<4>());
				case 5: return Get(GetVariationParams<5>());
				case 6: return Get(GetVariationParams<6>());
				case 7: return Get(GetVariationParams<7>());
				case 8: return Get(GetVariationParams<8>());
				case 9: return Get(GetVariationParams<9>());
				case 10: return Get(GetVariationParams<10>());
				case 11: return Get(GetVariationParams<11>());
				case 12: return Get(GetVariationParams<12>());
				case 13: return Get(GetVariationParams<13>());
				case 14: return Get(GetVariationParams<14>());
				case 15: return Get(GetVariationParams<15>());
				case 16: return Get(GetVariationParams<16>());
				case 17: return Get(GetVariationParams<17>());
				case 18: return Get(GetVariationParams<18>());
				case 19: return Get(GetVariationParams<19>());
				case 20: return Get(GetVariationParams<20>());
				case 21: return Get(GetVariationParams<21>());
				case 22: return Get(GetVariationParams<22>());
				case 23: return Get(GetVariationParams<23>());
				case 24: return Get(GetVariationParams<24>());
				case 25: return Get(GetVariationParams<25>());
				// The caller always passes an in-range variation (0..25); this fallback is purely defensive.
				default:
					B3D_ASSERT(false);
					return Get(GetVariationParams<0>());
				}
			}

			/**
			 * Fills @p outVariations with the sequence of FORMAT variation values that must be dispatched, in order, to fully
			 * compress the given base format. BC6H (base variation 4) expands to its 14 single-mode kernels (variations
			 * 4..17, encoder modes 1..14) and BC7 (base variation 18) to its 8 single-mode kernels (variations 18..25,
			 * encoder modes 0..7), each run over a shared running-best buffer; every other format is a single dispatch.
			 * Dispatch order is irrelevant because the host seeds gBestErr to +inf (no special "seed" pass).
			 */
			static void GetDispatchVariations(i32 baseVariation, TInlineArray<i32, 32>& outVariations)
			{
				outVariations.Clear();
				if(baseVariation == 4) // BC6H: 14 single-mode kernels.
				{
					for(i32 v = 4; v <= 17; ++v)
						outVariations.Add(v);
				}
				else if(baseVariation == 18) // BC7: 8 single-mode kernels.
				{
					for(i32 v = 18; v <= 25; ++v)
						outVariations.Add(v);
				}
				else
					outVariations.Add(baseVariation);
			}

		private:
			render::GpuParameterUniformBuffer mParametersBuffer;
		};

		/**
		 * Records the GPU compression on the render thread and sets up an asynchronous read-back. Must be called on the
		 * render thread: it creates the shared GPU resources, dispatches the compute kernel(s) tile by tile, then queues a
		 * non-blocking read-back of the packed blocks into @p destination. Completes @p op with @p destination once the GPU
		 * finishes and the blocks have been copied back, or with null on failure.
		 */
		void CompressOnRenderThread(const TInlineArray<i32, 32>& variations, const TShared<PixelData>& source, PixelFormat inputTexFormat, PixelFormat outputTexFormat, const TShared<PixelData>& destination, u32 maxTileSize, TAsyncOp<TShared<PixelData>> op)
		{
			AssertIfNotRenderThread();

			const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
			if(gpuDevice == nullptr)
			{
				op.CompleteOperation(nullptr);
				return;
			}

			GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();

			// Compile/fetch the shader variation(s)
			TInlineArray<TextureCompressMaterial*, 32> materials;
			for(const i32 dispatchVariation : variations)
			{
				TextureCompressMaterial* const material = TextureCompressMaterial::GetVariation(dispatchVariation);
				if(material == nullptr || material->GetComputePipeline() == nullptr)
				{
					op.CompleteOperation(nullptr);
					return;
				}

				materials.Add(material);
			}

			const u32 width = source->GetWidth();
			const u32 height = source->GetHeight();
			const u32 blockCountX = Math::DivideAndRoundUp(width, 4u);
			const u32 blockCountY = Math::DivideAndRoundUp(height, 4u);
			const u32 blockCount = blockCountX * blockCountY;
			if(blockCount == 0 || height == 0)
			{
				op.CompleteOperation(nullptr);
				return;
			}

			// Command buffer pool for the whole operation (uploads, tile dispatches, read-back).
			const TShared<render::GpuCommandBufferPool> pool = gpuDevice->CreateGpuCommandBufferPool(render::GpuCommandBufferPoolCreateInformation::CreateForThisThread());

			// Upload the whole source once into one sampled input texture, read as float4 in the shader. LDR formats use a
			// normalized RGBA8 texture; BC6H uses a full-float RGBA32F texture so HDR values survive. A 2D texture (rather
			// than a flat buffer) lets the texture cache exploit the spatial locality of each 4x4 block's 16 texels. 
			TextureCreateInformation inputTextureCreateInformation;
			inputTextureCreateInformation.Type = TEX_TYPE_2D;
			inputTextureCreateInformation.Format = inputTexFormat;
			inputTextureCreateInformation.Width = width;
			inputTextureCreateInformation.Height = height;
			inputTextureCreateInformation.Usage = TextureUsageFlag::StoreOnGPU; // device-local, sampled-only

			const TShared<render::Texture> inputTexture = gpuDevice->CreateTexture(inputTextureCreateInformation);
			if(inputTexture == nullptr)
			{
				op.CompleteOperation(nullptr);
				return;
			}

			// Output texture receives all packed blocks, one block per texel (RG32U for 64-bit blocks, RGBA32U for 128-bit),
			// at the block-grid resolution. Device-local + UAV so the compute kernel can write it; read back to the CPU after.
			TextureCreateInformation outputTextureCreateInformation;
			outputTextureCreateInformation.Type = TEX_TYPE_2D;
			outputTextureCreateInformation.Format = outputTexFormat;
			outputTextureCreateInformation.Width = blockCountX;
			outputTextureCreateInformation.Height = blockCountY;
			outputTextureCreateInformation.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

			const TShared<render::Texture> outputTexture = gpuDevice->CreateTexture(outputTextureCreateInformation);
			if(outputTexture == nullptr)
			{
				op.CompleteOperation(nullptr);
				return;
			}

			// Tile extent (a square, in blocks). maxTileSize is in pixels and has already been scaled by the per-format
			// multiplier (see GetFormatInfo / Compress), so a maxTileSize of 1024 means the texture is processed in
			// 1024x1024-pixel tiles (256x256 blocks). Aligned up to a multiple of 8 so non-cooperative tiles (8x8 thread groups)
			// fall on group boundaries and adjacent tiles never share a thread group.
			const u32 budgetBlocks = (maxTileSize < 4u) ? 1u : (maxTileSize / 4u);
			const u32 tileBlocks = Math::DivideAndRoundUp(Math::Max(1u, budgetBlocks), 8u) * 8u;
			const u32 tileCountX = Math::DivideAndRoundUp(blockCountX, tileBlocks);
			const u32 tileCountY = Math::DivideAndRoundUp(blockCountY, tileBlocks);

			// BC7 / BC6H evaluate each mode in its own dispatch (see TextureCompress.bsl), sharing a per-block running-best
			// error so each mode keeps its block when it beats the modes dispatched before it.  Single-dispatch formats skip it.
			const bool multiMode = materials.Size() > 1;
			const u32 bestErrorTextureWidth = Math::Min(tileBlocks, blockCountX);
			const u32 bestErrorTextureHeight = Math::Min(tileBlocks, blockCountY);

			TShared<render::Texture> bestErrorTexture;
			TShared<PixelData> bestErrorTextureInitialData;
			if(multiMode)
			{
				TextureCreateInformation bestErrorTextureCreateInformation;
				bestErrorTextureCreateInformation.Type = TEX_TYPE_2D;
				bestErrorTextureCreateInformation.Format = PF_R32F;
				bestErrorTextureCreateInformation.Width = bestErrorTextureWidth;
				bestErrorTextureCreateInformation.Height = bestErrorTextureHeight;
				bestErrorTextureCreateInformation.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

				bestErrorTexture = gpuDevice->CreateTexture(bestErrorTextureCreateInformation);
				if(bestErrorTexture == nullptr)
				{
					op.CompleteOperation(nullptr);
					return;
				}

				// +inf seed (one tile's worth). Fill row by row so any staging row-pitch padding is respected. FLT_MAX is set
				// explicitly (not via Clear/SetColors, which may clamp) so the first mode of every tile always wins.
				bestErrorTextureInitialData = PixelData::Create(bestErrorTextureWidth, bestErrorTextureHeight, 1, PF_R32F);
				for(u32 y = 0; y < bestErrorTextureHeight; ++y)
				{
					float* const row = (float*)(bestErrorTextureInitialData->GetData() + (u64)y * bestErrorTextureInitialData->GetRowPitch());
					for(u32 x = 0; x < bestErrorTextureWidth; ++x)
						row[x] = FLT_MAX;
				}
			}

			const Vector2I fullBlockCount((i32)blockCountX, (i32)blockCountY);
			const Vector2I fullTextureSize((i32)width, (i32)height);

			// Compress tile by tile, and mode by mode (for multi-mode encoders such as BC6H and BC7). After each tile/mode
			// we submit the command buffer and drain the GPU via WaitUntilIdle. This stops the OS watchdog (TDR) from triggering
			// on weaker GPUs due to excessively long tasks.
			bool inputUploaded = false;
			for(u32 tileY = 0; tileY < tileCountY; ++tileY)
			{
				const u32 tileStartBlockRow = tileY * tileBlocks;
				const u32 tileBlockRows = Math::Min(tileBlocks, blockCountY - tileStartBlockRow);

				for(u32 tileX = 0; tileX < tileCountX; ++tileX)
				{
					const u32 tileStartBlockColumn = tileX * tileBlocks;
					const u32 tileBlockColumnCount = Math::Min(tileBlocks, blockCountX - tileStartBlockColumn);
					const Vector2I tileDispatch((i32)tileBlockColumnCount, (i32)tileBlockRows);
					const Vector2I tileOffset((i32)tileStartBlockColumn, (i32)tileStartBlockRow);

					for(u32 materialIndex = 0; materialIndex < materials.Size(); ++materialIndex)
					{
						const TShared<render::GpuBuffer> parameterBuffer = render::gTextureCompressParameters.CreateTransientBuffer(gpuContext);
						if(parameterBuffer == nullptr)
						{
							op.CompleteOperation(nullptr);
							return;
						}

						render::GpuBufferMappedScope parameters = parameterBuffer->Map(GpuMapOption::Write);
						render::gTextureCompressParameters.gTextureSize.Set(parameters, fullTextureSize);
						render::gTextureCompressParameters.gBlockCount.Set(parameters, fullBlockCount);
						render::gTextureCompressParameters.gBlockOffset.Set(parameters, tileOffset);
						parameters.Unmap();

						render::GpuCommandBufferCreateInformation commandBufferInfo;
						commandBufferInfo.Name = "TextureCompress";

						const TShared<render::GpuCommandBuffer> commandBuffer = pool->Create(commandBufferInfo);

						// Upload the source onto the very first command buffer (the dispatch barriers make it visible to the
						// first sample); the input texture then persists, sampled by every later tile.
						if(!inputUploaded)
						{
							render::TextureUtility::Write(gpuContext, inputTexture, *source, 0, 0, render::TextureWriteFlag::Normal, commandBuffer);
							inputUploaded = true;
						}

						// Re-seed the per-tile running-best to +inf before this tile's first mode. The previous tile's writes are
						// already drained by the inter-tile WaitUntilIdle, so reusing the scratch is safe; the dispatch barriers
						// order this copy before the first mode reads it.
						if(multiMode && materialIndex == 0)
							render::TextureUtility::Write(gpuContext, bestErrorTexture, *bestErrorTextureInitialData, 0, 0, render::TextureWriteFlag::Normal, commandBuffer);

						// One mode dispatch (BC6H two-region uses 32-thread cooperative groups, everything else one thread per
						// block; see Execute), covering this tile's block region.
						materials[materialIndex]->Execute(*commandBuffer, inputTexture, outputTexture, parameters, bestErrorTexture, tileDispatch, variations[materialIndex]);

						gpuContext.SubmitCommandBuffer(commandBuffer);

						// Drain the GPU before the next mode/tile so it cannot run dispatches back-to-back and trip the watchdog.
						gpuDevice->WaitUntilIdle();
					}
				}
			}

			// Final command buffer: a single non-blocking read-back of the whole output texture
			render::GpuCommandBufferCreateInformation readbackInfo;
			readbackInfo.Name = "TextureCompressReadback";

			const TShared<render::GpuCommandBuffer> readbackCommandBuffer = pool->Create(readbackInfo);

			const u32 readSize = destination->GetConsecutiveSize();
			TAsyncOp<TShared<PixelData>> readOp = render::TextureUtility::ReadAsync(gpuContext, outputTexture, *readbackCommandBuffer);

			// Copy the packed blocks into the destination surface and complete the op once the GPU finishes. 
			readbackCommandBuffer->OnDidComplete.Connect(
				[op, readOp, outputTexture, inputTexture, bestErrorTexture, destination, readSize]() mutable
				{
					const TShared<PixelData> data = readOp.GetReturnValue();
					if(data == nullptr)
					{
						op.CompleteOperation(nullptr);
						return;
					}

					const u32 rows = data->GetHeight();
					const u32 destinationRowByteCount = (rows != 0) ? (readSize / rows) : readSize; // tightly-packed block row
					const u32 sourceRowPitch = data->GetRowPitch();
					const u8* const sourceData = (const u8*)data->GetData();

					u8* const destinationData = destination->GetData();
					for(u32 y = 0; y < rows; ++y)
						memcpy(destinationData + (u64)y * destinationRowByteCount, sourceData + (u64)y * sourceRowPitch, destinationRowByteCount);

					op.CompleteOperation(destination);
				});

			gpuContext.SubmitCommandBuffer(readbackCommandBuffer);
		}

	} // anonymous namespace

	bool GpuTextureCompressor::IsFormatSupported(PixelFormat format)
	{
		i32 variation;
		PixelFormat outputTexFormat;
		bool isHdr;
		float tileSizeMultiplier;

		return GetFormatInfo(format, variation, outputTexFormat, isHdr, tileSizeMultiplier);
	}

	TAsyncOp<TShared<PixelData>> GpuTextureCompressor::Compress(const TShared<PixelData>& source, const TShared<PixelData>& destination, const CompressionOptions& options)
	{
		// On failure we still return a *completed* op (with null) so a caller blocking on it never hangs.
		auto fnFailed = []()
		{
			TAsyncOp<TShared<PixelData>> op;
			op.CompleteOperation(nullptr);
			return op;
		};

		if(source == nullptr || destination == nullptr)
			return fnFailed();

		i32 variation;
		PixelFormat outputTexFormat;
		bool isHdr;
		float tileSizeMultiplier;

		if(!GetFormatInfo(options.Format, variation, outputTexFormat, isHdr, tileSizeMultiplier))
			return fnFailed();

		if(GetApplication().GetPrimaryGpuDevice() == nullptr)
			return fnFailed();

		// Scale the caller's tile budget by how heavy this format's per-block GPU work is, so each tile stays under the OS
		// GPU watchdog regardless of format (BC1/3 allow larger tiles, BC6H/BC7 force smaller ones; see GetFormatInfo()).
		const u32 effectiveTileSize = Math::Max(4u, (u32)((float)options.MaxTileSize * tileSizeMultiplier));

		// Convert the source to a surface the compute shader can sample: RGBA8 for LDR formats, RGBA32F for HDR (BC6H).
		const PixelFormat interimFormat = isHdr ? PF_RGBA32F : PF_RGBA8;
		const TShared<PixelData> convertedSource = PixelData::Create(source->GetWidth(), source->GetHeight(), 1, interimFormat);
		PixelUtility::BulkPixelConversion(*source, *convertedSource);

		// Determine the dispatch variation sequence for this format). BC7/BC6H expand to several mode-group variations dispatched 
		// in sequence; everything else is a single variation.
		TInlineArray<i32, 32> dispatchVariations;
		TextureCompressMaterial::GetDispatchVariations(variation, dispatchVariations);

		// Run inline if we are already on the render thread, otherwise marshal it across.
		TAsyncOp<TShared<PixelData>> op;
		auto fnGpuWork = [dispatchVariations, convertedSource, interimFormat, outputTexFormat, destination, effectiveTileSize, op]() mutable
		{
			CompressOnRenderThread(dispatchVariations, convertedSource, interimFormat, outputTexFormat, destination, effectiveTileSize, op);
		};

		if(B3D_CURRENT_THREAD_ID == GetRenderThread().GetThreadId())
			fnGpuWork();
		else
			GetRenderThread().PostCommand(std::move(fnGpuWork), "GPU texture compression");

		return op;
	}

} // namespace b3d
