//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DTextureCompressorStages.h"

#include "Image/B3DTexture.h"
#include "Renderer/B3DRendererMaterial.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d::render
{
	namespace
	{
#if B3D_BUILD_TYPE_DEVELOPMENT
		TConfigVariable<u32> gGpuBC7ModeMask("gpu.BC7ModeMask",
			"Bit mask of the modes the GPU BC7 compressor encodes with (bit N enables mode N). Development builds only.",
			0xFFu);

		TConfigVariable<u32> gGpuBC7CandidateCount("gpu.BC7CandidateCount",
			"Number of the most promising partitions the GPU BC7 compressor fully encodes per block, in every partitioned mode, "
			"instead of the number the compression quality selects. A power of two from 2 to 64 (capped to the partitions a "
			"mode has); 0 uses the compression quality. Development builds only.",
			0u);

		TConfigVariable<u32> gGpuBC6HCandidateCount("gpu.BC6HCandidateCount",
			"Number of the most promising partitions the GPU BC6H compressor fully encodes per block, in every two-region mode, "
			"instead of the number the compression quality selects. A power of two from 2 to 32; 0 uses the compression "
			"quality. Development builds only.",
			0u);
#endif

		/**
		 * Number of the most promising partitions the partitioned modes fully encode per block, indexed by
		 * CompressionQuality. Each mode's estimate stage ranks all of its partitions cheaply, and the finalize stages fully
		 * encode only these. BC6H's ranking is close enough that more candidates barely change its quality.
		 */
		constexpr u32 kBC7CandidateCounts[4] = { 4, 8, 16, 64 };
		constexpr u32 kBC6HCandidateCounts[4] = { 2, 4, 8, 32 };

		/**
		 * Returns the number of partitions a partitioned mode with @p partitionCount partitions fully encodes per block:
		 * @p requestedCandidateCount if it is a power of two from 2 to 64, or @p qualityCandidateCount otherwise.
		 */
		u32 GetCandidateCount(u32 qualityCandidateCount, u32 requestedCandidateCount, u32 partitionCount)
		{
			const bool isValidRequest = requestedCandidateCount >= 2 && requestedCandidateCount <= 64 && Bitwise::IsPow2(requestedCandidateCount);
			return Math::Min(isValidRequest ? requestedCandidateCount : qualityCandidateCount, partitionCount);
		}

		/** Returns the number of partitions a BC7 mode with @p partitionCount partitions fully encodes per block. */
		u32 GetBC7CandidateCount(CompressionQuality quality, u32 partitionCount)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			return GetCandidateCount(kBC7CandidateCounts[(u32)quality], gGpuBC7CandidateCount.Get(), partitionCount);
#else
			return GetCandidateCount(kBC7CandidateCounts[(u32)quality], 0, partitionCount);
#endif
		}

		/** Returns the number of partitions the BC6H two-region modes fully encode per block. */
		u32 GetBC6HCandidateCount(CompressionQuality quality)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			return GetCandidateCount(kBC6HCandidateCounts[(u32)quality], gGpuBC6HCandidateCount.Get(), 32);
#else
			return GetCandidateCount(kBC6HCandidateCounts[(u32)quality], 0, 32);
#endif
		}

		/** Two-subset BC7 partitions (modes 1, 3 and 7). Bit N is set if texel N belongs to subset 1. */
		constexpr u32 kBC7TwoSubsetPartitions[64] = {
			0xCCCC, 0x8888, 0xEEEE, 0xECC8, 0xC880, 0xFEEC, 0xFEC8, 0xEC80, 0xC800, 0xFFEC, 0xFE80, 0xE800, 0xFFE8, 0xFF00, 0xFFF0, 0xF000,
			0xF710, 0x008E, 0x7100, 0x08CE, 0x008C, 0x7310, 0x3100, 0x8CCE, 0x088C, 0x3110, 0x6666, 0x366C, 0x17E8, 0x0FF0, 0x718E, 0x399C,
			0xAAAA, 0xF0F0, 0x5A5A, 0x33CC, 0x3C3C, 0x55AA, 0x9696, 0xA55A, 0x73CE, 0x13C8, 0x324C, 0x3BDC, 0x6996, 0xC33C, 0x9966, 0x0660,
			0x0272, 0x04E4, 0x4E40, 0x2720, 0xC936, 0x936C, 0x39C6, 0x639C, 0x9336, 0x9CC6, 0x817E, 0xE718, 0xCCF0, 0x0FCC, 0x7744, 0xEE22
		};

		/** Texel anchoring subset 1 of each two-subset BC7 partition. Subset 0 is always anchored by texel 0. */
		constexpr u32 kBC7TwoSubsetAnchors[64] = {
			15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
			15,  2,  8,  2,  2,  8,  8, 15,  2,  8,  2,  2,  8,  8,  2,  2,
			15, 15,  6,  8,  2,  8, 15, 15,  2,  8,  2,  2,  2, 15, 15,  6,
			 6,  2,  6,  8, 15, 15,  2,  2, 15, 15, 15, 15, 15,  2,  2, 15
		};

		/** Contents of TextureCompressTable::BC7TwoSubsetPartitions. */
		struct BC7TwoSubsetPartitionTable
		{
			BC7TwoSubsetPartitionTable()
			{
				for(u32 partitionIndex = 0; partitionIndex < 64; ++partitionIndex)
					Entries[partitionIndex] = kBC7TwoSubsetPartitions[partitionIndex] | (kBC7TwoSubsetAnchors[partitionIndex] << 16);
			}

			u32 Entries[64];
		};

		const BC7TwoSubsetPartitionTable gBC7TwoSubsetPartitionTable;

		/** Three-subset BC7 partitions (modes 0 and 2). Bit N is set if texel N belongs to subset 1, bit 16 + N if it belongs to subset 2. */
		constexpr u32 kBC7ThreeSubsetPartitions[64] = {
			0xF60008CC, 0x73008CC8, 0x3310CC80, 0x00CEEC00, 0xCC003300, 0xCC0000CC, 0x00CCFF00, 0x3300CCCC,
			0xF0000F00, 0xF0000FF0, 0xFF0000F0, 0x88884444, 0x88886666, 0xCCCC2222, 0xEC80136C, 0x7310008C,
			0xC80036C8, 0x310008CE, 0xCCC03330, 0x0CCCF000, 0xEE0000EE, 0x77008888, 0xCC0022C0, 0x33004430,
			0x00CC0C22, 0xFC880344, 0x06606996, 0x66009960, 0xC88C0330, 0xF9000066, 0x0CC0C22C, 0x73108C00,
			0xEC801300, 0x08CEC400, 0xEC80004C, 0x44442222, 0x0F0000F0, 0x49242492, 0x42942942, 0x0C30C30C,
			0x03C0C03C, 0xFF0000AA, 0x5500AA00, 0xCCCC3030, 0x0C0CC0C0, 0x66669090, 0x0FF0A00A, 0x5550AAA0,
			0xF0000AAA, 0x0E0EE0E0, 0x88887070, 0x99906660, 0xE00E0EE0, 0x88880770, 0xF0000666, 0x99006600,
			0xFF000066, 0xC00C0CC0, 0xCCCC0330, 0x90006000, 0x08088080, 0xEEEE1010, 0xFFF0000A, 0x731008CE
		};

		/** Texels anchoring subsets 1 and 2 of each three-subset BC7 partition. Subset 0 is always anchored by texel 0. */
		constexpr u32 kBC7ThreeSubsetAnchors[2][64] = {
			{
				 3,  3, 15, 15,  8,  3, 15, 15,  8,  8,  6,  6,  6,  5,  3,  3,
				 3,  3,  8, 15,  3,  3,  6, 10,  5,  8,  8,  6,  8,  5, 15, 15,
				 8, 15,  3,  5,  6, 10,  8, 15, 15,  3, 15,  5, 15, 15, 15, 15,
				 3, 15,  5,  5,  5,  8,  5, 10,  5, 10,  8, 13, 15, 12,  3,  3
			},
			{
				15,  8,  8,  3, 15, 15,  3,  8, 15, 15, 15, 15, 15, 15, 15,  8,
				15,  8, 15,  3, 15,  8, 15,  8,  3, 15,  6, 10, 15, 15, 10,  8,
				15,  3, 15, 10, 10,  8,  9, 10,  6, 15,  8, 15,  3,  6,  6,  8,
				15,  3, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  8
			}
		};

		/** Contents of TextureCompressTable::BC7ThreeSubsetPartitions. */
		struct BC7ThreeSubsetPartitionTable
		{
			BC7ThreeSubsetPartitionTable()
			{
				for(u32 partitionIndex = 0; partitionIndex < 64; ++partitionIndex)
				{
					Entries[partitionIndex * 2] = kBC7ThreeSubsetPartitions[partitionIndex];
					Entries[partitionIndex * 2 + 1] = kBC7ThreeSubsetAnchors[0][partitionIndex] | (kBC7ThreeSubsetAnchors[1][partitionIndex] << 8);
				}
			}

			u32 Entries[128];
		};

		const BC7ThreeSubsetPartitionTable gBC7ThreeSubsetPartitionTable;

		/** Names the scratch buffers are bound with, indexed by TextureCompressScratchBuffer. */
		constexpr const char* kScratchBufferNames[(u32)TextureCompressScratchBuffer::Count] = { "gBC7TwoSubsetCandidates", "gBC7ThreeSubsetCandidates", "gBC6HCandidates" };

		/** Names the tables are bound with, indexed by TextureCompressTable. */
		constexpr const char* kTableNames[(u32)TextureCompressTable::Count] = { "gBC7TwoSubsetPartitions", "gBC7ThreeSubsetPartitions" };

		class TextureCompressMaterial : public RendererMaterial<TextureCompressMaterial> { RMAT_DEF("TextureCompress.bsl") };
		class TextureCompressBC1Material : public RendererMaterial<TextureCompressBC1Material> { RMAT_DEF("TextureCompressBC1.bsl") };
		class TextureCompressBC7SingleSubsetMaterial : public RendererMaterial<TextureCompressBC7SingleSubsetMaterial> { RMAT_DEF("TextureCompressBC7SingleSubset.bsl") };
		class TextureCompressBC6HEstimateMaterial : public RendererMaterial<TextureCompressBC6HEstimateMaterial> { RMAT_DEF("TextureCompressBC6HEstimate.bsl") };
		class TextureCompressBC6HTwoRegionFinalizeMaterial : public RendererMaterial<TextureCompressBC6HTwoRegionFinalizeMaterial> { RMAT_DEF("TextureCompressBC6HTwoRegionFinalize.bsl") };
		class TextureCompressBC6HOneRegionMaterial : public RendererMaterial<TextureCompressBC6HOneRegionMaterial> { RMAT_DEF("TextureCompressBC6HOneRegion.bsl") };
		class TextureCompressBC7TwoSubsetEstimateMaterial : public RendererMaterial<TextureCompressBC7TwoSubsetEstimateMaterial> { RMAT_DEF("TextureCompressBC7TwoSubsetEstimate.bsl") };
		class TextureCompressBC7TwoSubsetFinalizeMaterial : public RendererMaterial<TextureCompressBC7TwoSubsetFinalizeMaterial> { RMAT_DEF("TextureCompressBC7TwoSubsetFinalize.bsl") };
		class TextureCompressBC7ThreeSubsetEstimateMaterial : public RendererMaterial<TextureCompressBC7ThreeSubsetEstimateMaterial> { RMAT_DEF("TextureCompressBC7ThreeSubsetEstimate.bsl") };
		class TextureCompressBC7ThreeSubsetFinalizeMaterial : public RendererMaterial<TextureCompressBC7ThreeSubsetFinalizeMaterial> { RMAT_DEF("TextureCompressBC7ThreeSubsetFinalize.bsl") };

		/** Returns a function fetching the variation of material @p T where shader define @p name is @p value. */
		template<class T>
		TextureCompressStage::GetMaterialFunction GetVariation(const char* name, i32 value)
		{
			return [name, value]() -> RendererMaterialBase* { return T::Get(ShaderVariationParameters({ ShaderVariationParameter(name, value) })); };
		}

		/** Returns a function fetching material @p T, whose shader has no variations. */
		template<class T>
		TextureCompressStage::GetMaterialFunction GetMaterial()
		{
			return []() -> RendererMaterialBase* { return T::Get(); };
		}

		/**
		 * Adds a stage that fully encodes the best few candidate partitions of a partitioned mode, using variation
		 * @p mode of the @p modeDefine define of material @p T. Its list of @p candidateCount partitions per block starts
		 * at entry @p listOffset of the @p listStride entries per block the estimate stage wrote to @p candidates. Each
		 * group encodes 64 / @p candidateCount blocks, one thread per candidate.
		 */
		template<class T>
		void AddFinalizeStage(TextureCompressStageList& stages, String name, const char* modeDefine, i32 mode, u32 candidateCount,
			TextureCompressScratchBuffer candidates, TextureCompressTable partitions, u32 listStride, u32 listOffset)
		{
			if(candidateCount == 0)
				return;

			TextureCompressStageRequirements requirements;
			requirements.ScratchElementsPerBlock[(u32)candidates] = listStride;
			requirements.UsesTable[(u32)partitions] = true;
			requirements.UsesBestError = true;
			requirements.StageParameters = Vector4I((i32)listStride, (i32)listOffset, 0, 0);

			TextureCompressStage::GetMaterialFunction fnGetMaterial = [modeDefine, mode, candidateCount]() -> RendererMaterialBase*
			{
				return T::Get(ShaderVariationParameters({ ShaderVariationParameter(modeDefine, mode), ShaderVariationParameter("CANDIDATE_COUNT", (i32)candidateCount) }));
			};

			stages.Add(B3DMakeShared<TextureCompressStage>(std::move(name), std::move(fnGetMaterial), Vector2I(64 / (i32)candidateCount, 1), requirements));
		}

		/** Adds the stages of BC7, which encodes every block with each enabled mode and keeps the lowest-error encoding. */
		void AddBC7Stages(TextureCompressStageList& stages, CompressionQuality quality)
		{
			u32 modeMask = 0xFFu;
#if B3D_BUILD_TYPE_DEVELOPMENT
			if((gGpuBC7ModeMask.Get() & 0xFFu) != 0)
				modeMask = gGpuBC7ModeMask.Get();
#endif

			auto fnIsModeEnabled = [modeMask](u32 mode) { return (modeMask & (1u << mode)) != 0; };

			// The partitioned modes come in two families, the three-subset modes (0 and 2) and the two-subset modes (1, 3
			// and 7). The modes of a family share their partitions. One estimate stage per family ranks them for all its
			// modes, one thread per partition, and each mode fully encodes only its best few candidates, with several blocks
			// per group.
			const u32 mode0CandidateCount = fnIsModeEnabled(0) ? GetBC7CandidateCount(quality, 16) : 0;
			const u32 mode2CandidateCount = fnIsModeEnabled(2) ? GetBC7CandidateCount(quality, 64) : 0;
			const u32 threeSubsetCandidateCount = mode0CandidateCount + mode2CandidateCount;
			if(threeSubsetCandidateCount > 0)
			{
				TextureCompressStageRequirements estimateRequirements;
				estimateRequirements.ScratchElementsPerBlock[(u32)TextureCompressScratchBuffer::BC7ThreeSubsetCandidates] = threeSubsetCandidateCount;
				estimateRequirements.UsesTable[(u32)TextureCompressTable::BC7ThreeSubsetPartitions] = true;
				estimateRequirements.StageParameters = Vector4I((i32)mode0CandidateCount, (i32)mode2CandidateCount, 0, 0);

				stages.Add(B3DMakeShared<TextureCompressStage>("BC7Estimate three-subset", GetMaterial<TextureCompressBC7ThreeSubsetEstimateMaterial>(), Vector2I(1, 1), estimateRequirements));
			}

			auto fnAddThreeSubsetFinalize = [&stages, threeSubsetCandidateCount](i32 mode, u32 candidateCount, u32 listOffset)
			{
				AddFinalizeStage<TextureCompressBC7ThreeSubsetFinalizeMaterial>(stages, StringUtility::Format("BC7Finalize mode {0}", mode), "BC7_MODE", mode, candidateCount,
					TextureCompressScratchBuffer::BC7ThreeSubsetCandidates, TextureCompressTable::BC7ThreeSubsetPartitions, threeSubsetCandidateCount, listOffset);
			};

			fnAddThreeSubsetFinalize(0, mode0CandidateCount, 0);
			fnAddThreeSubsetFinalize(2, mode2CandidateCount, mode0CandidateCount);

			// Per block, the two-subset list of mode 1 is followed by the lists of modes 3 and 7
			const u32 mode1CandidateCount = fnIsModeEnabled(1) ? GetBC7CandidateCount(quality, 64) : 0;
			const u32 mode3CandidateCount = fnIsModeEnabled(3) ? GetBC7CandidateCount(quality, 64) : 0;
			const u32 mode7CandidateCount = fnIsModeEnabled(7) ? GetBC7CandidateCount(quality, 64) : 0;
			const u32 twoSubsetCandidateCount = mode1CandidateCount + mode3CandidateCount + mode7CandidateCount;
			if(twoSubsetCandidateCount > 0)
			{
				TextureCompressStageRequirements estimateRequirements;
				estimateRequirements.ScratchElementsPerBlock[(u32)TextureCompressScratchBuffer::BC7TwoSubsetCandidates] = twoSubsetCandidateCount;
				estimateRequirements.UsesTable[(u32)TextureCompressTable::BC7TwoSubsetPartitions] = true;
				estimateRequirements.StageParameters = Vector4I((i32)mode1CandidateCount, (i32)mode3CandidateCount, (i32)mode7CandidateCount, fnIsModeEnabled(3) ? 1 : 0);

				stages.Add(B3DMakeShared<TextureCompressStage>("BC7Estimate two-subset", GetMaterial<TextureCompressBC7TwoSubsetEstimateMaterial>(), Vector2I(1, 1), estimateRequirements));
			}

			auto fnAddTwoSubsetFinalize = [&stages, twoSubsetCandidateCount](i32 mode, u32 candidateCount, u32 listOffset)
			{
				AddFinalizeStage<TextureCompressBC7TwoSubsetFinalizeMaterial>(stages, StringUtility::Format("BC7Finalize mode {0}", mode), "BC7_MODE", mode, candidateCount,
					TextureCompressScratchBuffer::BC7TwoSubsetCandidates, TextureCompressTable::BC7TwoSubsetPartitions, twoSubsetCandidateCount, listOffset);
			};

			fnAddTwoSubsetFinalize(1, mode1CandidateCount, 0);
			fnAddTwoSubsetFinalize(3, mode3CandidateCount, mode1CandidateCount);

			// The single-subset modes have no partitions. Every block is encoded by one thread per candidate: per rotation and
			// index selector in mode 4 (8), per rotation in mode 5 (4), and a single one in mode 6, with 64 threads per group.
			TextureCompressStageRequirements singleSubsetRequirements;
			singleSubsetRequirements.UsesBestError = true;

			constexpr i32 kSingleSubsetCandidateCounts[3] = { 8, 4, 1 };
			for(i32 mode = 4; mode <= 6; ++mode)
			{
				if(fnIsModeEnabled((u32)mode))
				{
					stages.Add(B3DMakeShared<TextureCompressStage>(StringUtility::Format("BC7Finalize mode {0}", mode), GetVariation<TextureCompressBC7SingleSubsetMaterial>("BC7_MODE", mode),
						Vector2I(64 / kSingleSubsetCandidateCounts[mode - 4], 1), singleSubsetRequirements));
				}
			}

			fnAddTwoSubsetFinalize(7, mode7CandidateCount, mode1CandidateCount + mode3CandidateCount);
		}

		/** Adds the stages of BC6H, which encodes every block with each mode and keeps the lowest-error encoding. */
		void AddBC6HStages(TextureCompressStageList& stages, CompressionQuality quality)
		{
			// The two-region modes (1..10) share their partitions. One estimate stage ranks them, one thread per partition,
			// and each mode fully encodes only the best few candidates, with several blocks per group.
			const u32 candidateCount = GetBC6HCandidateCount(quality);

			TextureCompressStageRequirements estimateRequirements;
			estimateRequirements.ScratchElementsPerBlock[(u32)TextureCompressScratchBuffer::BC6HCandidates] = candidateCount;
			estimateRequirements.UsesTable[(u32)TextureCompressTable::BC7TwoSubsetPartitions] = true;
			estimateRequirements.StageParameters = Vector4I((i32)candidateCount, 0, 0, 0);

			stages.Add(B3DMakeShared<TextureCompressStage>("BC6HEstimate two-region", GetMaterial<TextureCompressBC6HEstimateMaterial>(), Vector2I(2, 1), estimateRequirements));

			for(i32 mode = 1; mode <= 10; ++mode)
			{
				AddFinalizeStage<TextureCompressBC6HTwoRegionFinalizeMaterial>(stages, StringUtility::Format("BC6H mode {0}", mode), "BC6_MODE", mode, candidateCount,
					TextureCompressScratchBuffer::BC6HCandidates, TextureCompressTable::BC7TwoSubsetPartitions, candidateCount, 0);
			}

			// The one-region modes (11..14) encode each block with one thread
			TextureCompressStageRequirements oneRegionRequirements;
			oneRegionRequirements.UsesBestError = true;

			for(i32 mode = 11; mode <= 14; ++mode)
			{
				stages.Add(B3DMakeShared<TextureCompressStage>(StringUtility::Format("BC6H mode {0}", mode), GetVariation<TextureCompressBC6HOneRegionMaterial>("BC6_MODE", mode),
					Vector2I(64, 1), oneRegionRequirements));
			}
		}
	}

	TextureCompressStage::TextureCompressStage(String name, GetMaterialFunction fnGetMaterial, const Vector2I& blocksPerGroup, const TextureCompressStageRequirements& requirements)
		: mName(std::move(name)), mFnGetMaterial(std::move(fnGetMaterial)), mBlocksPerGroup(blocksPerGroup), mRequirements(requirements)
	{ }

	bool TextureCompressStage::Prepare()
	{
		mMaterial = mFnGetMaterial();
		if(mMaterial == nullptr || mMaterial->GetComputePipeline() == nullptr)
			return false;

		// One stage class serves every kernel, so the parameters a kernel doesn't declare are left unset
		const GpuPipelineParameterLayout& parameterLayout = *mMaterial->GetComputePipeline()->GetParameterLayout();
		parameterLayout.GetSampledTextureParameter("gInput", mInputParameter);
		parameterLayout.TryGetStorageTextureParameter("gOutput", mOutputParameter);
		parameterLayout.TryGetStorageTextureParameter("gBestErr", mBestErrorParameter);

		for(u32 bufferIndex = 0; bufferIndex < (u32)TextureCompressScratchBuffer::Count; ++bufferIndex)
			parameterLayout.TryGetStorageBufferParameter(kScratchBufferNames[bufferIndex], mScratchBufferParameters[bufferIndex]);

		for(u32 tableIndex = 0; tableIndex < (u32)TextureCompressTable::Count; ++tableIndex)
			parameterLayout.TryGetStorageBufferParameter(kTableNames[tableIndex], mTableParameters[tableIndex]);

		parameterLayout.TryGetUniformBufferParameter("Parameters", mParametersBuffer);
		return true;
	}

	TShared<GpuParameterSet> TextureCompressStage::PrepareDispatch(FrameGraphPass& pass, const TextureCompressBindings& bindings) const
	{
		B3D_ASSERT(mMaterial != nullptr);

		const TShared<GpuParameterSet> set = mMaterial->CreateGpuParameterSet(pass);
		mInputParameter.Set(set, bindings.Input);
		mOutputParameter.Set(set, bindings.Output, TextureSurface::kComplete);

		if(bindings.BestError != nullptr)
			mBestErrorParameter.Set(set, bindings.BestError, TextureSurface::kComplete);

		for(u32 bufferIndex = 0; bufferIndex < (u32)TextureCompressScratchBuffer::Count; ++bufferIndex)
		{
			if(bindings.ScratchBuffers[bufferIndex] != nullptr)
				mScratchBufferParameters[bufferIndex].Set(set, bindings.ScratchBuffers[bufferIndex]);
		}

		for(u32 tableIndex = 0; tableIndex < (u32)TextureCompressTable::Count; ++tableIndex)
		{
			if(bindings.Tables[tableIndex] != nullptr)
				mTableParameters[tableIndex].Set(set, bindings.Tables[tableIndex]);
		}

		mParametersBuffer.Set(set, *bindings.Parameters);
		return set;
	}

	void TextureCompressStage::Dispatch(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Vector2I& chunkBlockCount) const
	{
		GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();

		mMaterial->Bind(commandBuffer, set);
		commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(chunkBlockCount.X, mBlocksPerGroup.X), (u32)Math::DivideAndRoundUp(chunkBlockCount.Y, mBlocksPerGroup.Y));
	}

	bool GetTextureCompressFormatInformation(PixelFormat format, PixelFormat& outBlockTextureFormat, bool& outIsHDR)
	{
		outIsHDR = false;
		switch(format)
		{
		case PF_BC1:
		case PF_BC1a:
		case PF_BC4:
			outBlockTextureFormat = PF_RG32U;
			return true;
		case PF_BC3:
		case PF_BC5:
		case PF_BC7:
			outBlockTextureFormat = PF_RGBA32U;
			return true;
		case PF_BC6H:
			outBlockTextureFormat = PF_RGBA32U;
			outIsHDR = true;
			return true;
		default:
			return false;
		}
	}

	TextureCompressStageList GetTextureCompressStages(PixelFormat format, CompressionQuality quality)
	{
		TextureCompressStageList stages;

		// BC1 and BC3 encode each block with 16 threads that share its cluster fit, 4 blocks per group (TextureCompressBC1.bsl
		// FORMAT 0 and 1). BC4 and BC5 encode each block with one thread (TextureCompress.bsl FORMAT 2 and 3).
		auto fnAddBC1Stage = [&stages](const char* name, i32 variation)
		{
			stages.Add(B3DMakeShared<TextureCompressStage>(name, GetVariation<TextureCompressBC1Material>("FORMAT", variation), Vector2I(4, 1), TextureCompressStageRequirements()));
		};

		auto fnAddBC4Stage = [&stages](const char* name, i32 variation)
		{
			stages.Add(B3DMakeShared<TextureCompressStage>(name, GetVariation<TextureCompressMaterial>("FORMAT", variation), Vector2I(8, 8), TextureCompressStageRequirements()));
		};

		switch(format)
		{
		case PF_BC1:
		case PF_BC1a:
			fnAddBC1Stage("BC1", 0);
			break;
		case PF_BC3:
			fnAddBC1Stage("BC3", 1);
			break;
		case PF_BC4:
			fnAddBC4Stage("BC4", 2);
			break;
		case PF_BC5:
			fnAddBC4Stage("BC5", 3);
			break;
		case PF_BC6H:
			AddBC6HStages(stages, quality);
			break;
		case PF_BC7:
			AddBC7Stages(stages, quality);
			break;
		default:
			break;
		}

		return stages;
	}

	TShared<GpuBuffer> CreateTextureCompressScratchBuffer(GpuDevice& device, u32 elementCount)
	{
		const GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
		return device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), elementCount, flags));
	}

	TArrayView<const u32> GetTextureCompressTableData(TextureCompressTable table)
	{
		switch(table)
		{
		case TextureCompressTable::BC7TwoSubsetPartitions:
			return TArrayView<const u32>(gBC7TwoSubsetPartitionTable.Entries, 64);
		case TextureCompressTable::BC7ThreeSubsetPartitions:
			return TArrayView<const u32>(gBC7ThreeSubsetPartitionTable.Entries, 128);
		default:
			B3D_ASSERT(false);
			return TArrayView<const u32>();
		}
	}
} // namespace b3d::render
