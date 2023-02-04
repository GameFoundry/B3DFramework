//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsGpuSort.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Math/BsRandom.h"
#include "Renderer/BsRendererUtility.h"

namespace bs {
namespace ct {

static constexpr u32 kBitCount = 32;
static constexpr u32 kRadixNumBits = 4;
static constexpr u32 kNumDigits = 1 << kRadixNumBits;
static constexpr u32 kKeyMask = (kNumDigits - 1);
static constexpr u32 kNumPasses = kBitCount / kRadixNumBits;

static constexpr u32 kNumThreads = 128;
static constexpr u32 kKeysPerLoop = 8;
static constexpr u32 kTileSize = kNumThreads * kKeysPerLoop;
static constexpr u32 kMaxNumGroups = 64;

RadixSortParamsDef gRadixSortParamsDef;

/** Contains various constants required during the GpuSort algorithm. */
struct GpuSortProperties
{
	GpuSortProperties(u32 count)
		: Count(count)
	{}

	const u32 Count;
	const u32 NumTiles = Count / kTileSize;
	const u32 NumGroups = Math::Clamp(NumTiles, 1U, kMaxNumGroups);

	const u32 TilesPerGroup = NumTiles / NumGroups;
	const u32 ExtraTiles = NumTiles % NumGroups;
	const u32 ExtraKeys = Count % kTileSize;
};

/** Set up common defines required by all radix sort shaders. */
void InitCommonDefines(ShaderDefines& defines)
{
	defines.Set("RADIX_NUM_BITS", kRadixNumBits);
	defines.Set("NUM_THREADS", kNumThreads);
	defines.Set("KEYS_PER_LOOP", kKeysPerLoop);
	defines.Set("MAX_NUM_GROUPS", kMaxNumGroups);
}

void RunSortTest();

/**
 * Creates a new GPU parameter block buffer according to gRadixSortParamDef definition and writes GpuSort properties
 * into the buffer.
 */
SPtr<GpuParamBlockBuffer> CreateGpuSortParams(const GpuSortProperties& props)
{
	SPtr<GpuParamBlockBuffer> buffer = gRadixSortParamsDef.CreateBuffer();

	gRadixSortParamsDef.gTilesPerGroup.Set(buffer, props.TilesPerGroup);
	gRadixSortParamsDef.gNumGroups.Set(buffer, props.NumGroups);
	gRadixSortParamsDef.gNumExtraTiles.Set(buffer, props.ExtraTiles);
	gRadixSortParamsDef.gNumExtraKeys.Set(buffer, props.ExtraKeys);
	gRadixSortParamsDef.gBitOffset.Set(buffer, 0);

	return buffer;
}

/**
 * Checks can the provided buffer be used for GPU sort operation. Returns a pointer to the error message if check failed
 * or nullptr if check passed.
 */
const char* CheckSortBuffer(GpuBuffer& buffer)
{
	static constexpr const char* kInvalidGpuWriteMsg =
		"All buffers provided to GpuSort must be created with GBU_LOADSTORE flags enabled.";
	static constexpr const char* kInvalidTypeMsg =
		"All buffers provided to GpuSort must be of GBT_STANDARD type.";
	static constexpr const char* kInvalidFormatMsg =
		"All buffers provided to GpuSort must use a 32-bit unsigned integer format.";

	const GpuBufferProperties& bufferProps = buffer.GetProperties();
	if((bufferProps.GetUsage() & GBU_LOADSTORE) != GBU_LOADSTORE)
		return kInvalidGpuWriteMsg;

	if(bufferProps.GetType() != GBT_STANDARD)
		return kInvalidTypeMsg;

	if(bufferProps.GetFormat() != BF_32X1U)
		return kInvalidFormatMsg;

	return nullptr;
}

/** Creates a helper buffers used for storing intermediate information during GpuSort::sort. */
SPtr<GpuBuffer> CreateHelperBuffer()
{
	GpuBufferCreateInformation desc;
	desc.ElementCount = kMaxNumGroups * kNumDigits;
	desc.Format = BF_32X1U;
	desc.Usage = GBU_LOADSTORE;
	desc.Type = GBT_STANDARD;

	return GpuBuffer::Create(desc);
}

void RadixSortClearMat::Initialize()
{
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gOutput", MOutputParam);
}

void RadixSortClearMat::InitDefinesInternal(ShaderDefines& defines)
{
	InitCommonDefines(defines);
}

void RadixSortClearMat::Execute(const SPtr<GpuBuffer>& outputOffsets)
{
	BS_RENMAT_PROFILE_BLOCK

	MOutputParam.Set(outputOffsets);

	Bind();
	RenderAPI::Instance().DispatchCompute(1);
}

void RadixSortCountMat::Initialize()
{
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gInputKeys", MInputKeysParam);
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gOutputCounts", MOutputCountsParam);
}

void RadixSortCountMat::InitDefinesInternal(ShaderDefines& defines)
{
	InitCommonDefines(defines);
}

void RadixSortCountMat::Execute(u32 numGroups, const SPtr<GpuParamBlockBuffer>& params, const SPtr<GpuBuffer>& inputKeys, const SPtr<GpuBuffer>& outputOffsets)
{
	BS_RENMAT_PROFILE_BLOCK

	MInputKeysParam.Set(inputKeys);
	MOutputCountsParam.Set(outputOffsets);

	mGPUParameters->SetParameterBlockBuffer("Params", params);

	Bind();
	RenderAPI::Instance().DispatchCompute(numGroups);
}

void RadixSortPrefixScanMat::Initialize()
{
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gInputCounts", MInputCountsParam);
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gOutputOffsets", MOutputOffsetsParam);
}

void RadixSortPrefixScanMat::InitDefinesInternal(ShaderDefines& defines)
{
	InitCommonDefines(defines);
}

void RadixSortPrefixScanMat::Execute(const SPtr<GpuParamBlockBuffer>& params, const SPtr<GpuBuffer>& inputCounts, const SPtr<GpuBuffer>& outputOffsets)
{
	BS_RENMAT_PROFILE_BLOCK

	MInputCountsParam.Set(inputCounts);
	MOutputOffsetsParam.Set(outputOffsets);

	mGPUParameters->SetParameterBlockBuffer("Params", params);

	Bind();
	RenderAPI::Instance().DispatchCompute(1);
}

void RadixSortReorderMat::Initialize()
{
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gInputOffsets", MInputOffsetsBufferParam);
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gInputKeys", MInputKeysBufferParam);
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gInputValues", MInputValuesBufferParam);
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gOutputKeys", MOutputKeysBufferParam);
	mGPUParameters->GetBufferParameter(GPT_COMPUTE_PROGRAM, "gOutputValues", MOutputValuesBufferParam);
}

void RadixSortReorderMat::InitDefinesInternal(ShaderDefines& defines)
{
	InitCommonDefines(defines);
}

void RadixSortReorderMat::Execute(u32 numGroups, const SPtr<GpuParamBlockBuffer>& params, const SPtr<GpuBuffer>& inputPrefix, const GpuSortBuffers& buffers, u32 inputBufferIdx)
{
	BS_RENMAT_PROFILE_BLOCK

	const u32 outputBufferIdx = (inputBufferIdx + 1) % 2;

	MInputOffsetsBufferParam.Set(inputPrefix);
	MInputKeysBufferParam.Set(buffers.Keys[inputBufferIdx]);
	MInputValuesBufferParam.Set(buffers.Values[inputBufferIdx]);
	MOutputKeysBufferParam.Set(buffers.Keys[outputBufferIdx]);
	MOutputValuesBufferParam.Set(buffers.Values[outputBufferIdx]);

	mGPUParameters->SetParameterBlockBuffer("Params", params);

	Bind();
	RenderAPI::Instance().DispatchCompute(numGroups);
}

GpuSort::GpuSort()
{
	mHelperBuffers[0] = CreateHelperBuffer();
	mHelperBuffers[1] = CreateHelperBuffer();
}

u32 GpuSort::Sort(const GpuSortBuffers& buffers, u32 numKeys, u32 keyMask)
{
	// Nothing to do if no input or output key buffers
	if(buffers.Keys[0] == nullptr || buffers.Keys[1] == nullptr)
		return 0;

	// Check if all buffers have been created with required options
	const char* errorMsg = nullptr;
	for(u32 i = 0; i < 2; i++)
	{
		errorMsg = CheckSortBuffer(*buffers.Keys[i]);
		if(errorMsg) break;

		if(buffers.Values[i])
		{
			errorMsg = CheckSortBuffer(*buffers.Values[i]);
			if(errorMsg) break;
		}
	}

	if(errorMsg)
	{
		B3D_LOG(Error, Renderer, "GpuSort failed: {0}", errorMsg);
		return 0;
	}

	// Check if all buffers have the same size
	bool validSize = buffers.Keys[0]->GetSize() == buffers.Keys[1]->GetSize();
	if(buffers.Values[0] && buffers.Values[1])
	{
		validSize = buffers.Keys[0]->GetSize() == buffers.Values[0]->GetSize() &&
			buffers.Keys[0]->GetSize() == buffers.Values[1]->GetSize();
	}

	if(!validSize)
	{
		B3D_LOG(Error, Renderer, "GpuSort failed: All sort buffers must have the same size.");
		return 0;
	}

	const GpuSortProperties gpuSortProps(numKeys);
	SPtr<GpuParamBlockBuffer> params = CreateGpuSortParams(gpuSortProps);

	u32 bitOffset = 0;
	u32 inputBufferIdx = 0;
	for(u32 i = 0; i < kNumPasses; i++)
	{
		if(((kKeyMask << bitOffset) & keyMask) != 0)
		{
			gRadixSortParamsDef.gBitOffset.Set(params, bitOffset);

			RadixSortClearMat::Get()->Execute(mHelperBuffers[0]);
			RadixSortCountMat::Get()->Execute(gpuSortProps.NumGroups, params, buffers.Keys[inputBufferIdx], mHelperBuffers[0]);
			RadixSortPrefixScanMat::Get()->Execute(params, mHelperBuffers[0], mHelperBuffers[1]);
			RadixSortReorderMat::Get()->Execute(gpuSortProps.NumGroups, params, mHelperBuffers[1], buffers, inputBufferIdx);

			inputBufferIdx = (inputBufferIdx + 1) % 2;
		}

		bitOffset += kRadixNumBits;
	}

	return inputBufferIdx;
}

GpuSortBuffers GpuSort::CreateSortBuffers(u32 numElements, bool values)
{
	GpuSortBuffers output;

	GpuBufferCreateInformation bufferDesc;
	bufferDesc.ElementCount = numElements;
	bufferDesc.Format = BF_32X1U;
	bufferDesc.Type = GBT_STANDARD;
	bufferDesc.Usage = GBU_LOADSTORE;

	output.Keys[0] = GpuBuffer::Create(bufferDesc);
	output.Keys[1] = GpuBuffer::Create(bufferDesc);

	if(values)
	{
		output.Values[0] = GpuBuffer::Create(bufferDesc);
		output.Values[1] = GpuBuffer::Create(bufferDesc);
	}

	return output;
}

// Note: This test isn't currently hooked up anywhere. It might be a good idea to set it up as a unit test, but it would
// require exposing parts of GpuSort to the public, which I don't feel like it's worth doing just for a test. So instead
// just make sure to run the test below if you modify any of the GpuSort code.
void RunSortTest()
{
	// Generate test keys
	static constexpr u32 kNumInputKeys = 10000;
	Vector<u32> inputKeys;
	inputKeys.reserve(kNumInputKeys);

	Random random;
	for(u32 i = 0; i < kNumInputKeys; i++)
		inputKeys.push_back(random.GetRange(0, 15) << 4 | std::min(kNumDigits - 1, (i / (kNumInputKeys / 16))));

	const auto count = (u32)inputKeys.size();
	u32 bitOffset = 4;
	u32 bitMask = (1 << kRadixNumBits) - 1;

	// Prepare buffers
	const GpuSortProperties gpuSortProps(count);
	SPtr<GpuParamBlockBuffer> params = CreateGpuSortParams(gpuSortProps);

	gRadixSortParamsDef.gBitOffset.Set(params, bitOffset);

	GpuSortBuffers sortBuffers = GpuSort::CreateSortBuffers(count);
	sortBuffers.Keys[0]->WriteData(0, sortBuffers.Keys[0]->GetSize(), inputKeys.data(), BWT_DISCARD);

	SPtr<GpuBuffer> helperBuffers[2];
	helperBuffers[0] = CreateHelperBuffer();
	helperBuffers[1] = CreateHelperBuffer();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////// Count keys per group //////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// SERIAL:
	Vector<u32> counts(gpuSortProps.NumGroups * kNumDigits);
	for(u32 groupIdx = 0; groupIdx < gpuSortProps.NumGroups; groupIdx++)
	{
		// Count keys per thread
		u32 localCounts[kNumThreads * kNumDigits] = { 0 };

		u32 tileIdx;
		u32 numTiles;
		if(groupIdx < gpuSortProps.ExtraTiles)
		{
			numTiles = gpuSortProps.TilesPerGroup + 1;
			tileIdx = groupIdx * numTiles;
		}
		else
		{
			numTiles = gpuSortProps.TilesPerGroup;
			tileIdx = groupIdx * numTiles + gpuSortProps.ExtraTiles;
		}

		u32 keyBegin = tileIdx * kTileSize;
		u32 keyEnd = keyBegin + numTiles * kTileSize;

		while(keyBegin < keyEnd)
		{
			for(u32 threadIdx = 0; threadIdx < kNumThreads; threadIdx++)
			{
				u32 key = inputKeys[keyBegin + threadIdx];
				u32 digit = (key >> bitOffset) & bitMask;

				localCounts[threadIdx * kNumDigits + digit] += 1;
			}

			keyBegin += kNumThreads;
		}

		if(groupIdx == (gpuSortProps.NumGroups - 1))
		{
			keyBegin = keyEnd;
			keyEnd = keyBegin + gpuSortProps.ExtraKeys;

			while(keyBegin < keyEnd)
			{
				for(u32 threadIdx = 0; threadIdx < kNumThreads; threadIdx++)
				{
					if((keyBegin + threadIdx) < keyEnd)
					{
						u32 key = inputKeys[keyBegin + threadIdx];
						u32 digit = (key >> bitOffset) & bitMask;

						localCounts[threadIdx * kNumDigits + digit] += 1;
					}
				}

				keyBegin += kNumThreads;
			}
		}

		// Sum up all key counts in a group
		static constexpr u32 kNumReduceThreads = 64;
		static constexpr u32 kNumReduceThreadsPerDigit = kNumReduceThreads / kNumDigits;
		static constexpr u32 kNumReduceElemsPerThreadPerDigit = kNumThreads / kNumReduceThreadsPerDigit;

		u32 reduceCounters[kNumReduceThreads] = { 0 };
		u32 reduceTotals[kNumReduceThreads] = { 0 };
		for(u32 threadId = 0; threadId < kNumThreads; threadId++)
		{
			if(threadId < kNumReduceThreads)
			{
				u32 digitIdx = threadId / kNumReduceThreadsPerDigit;
				u32 setIdx = threadId & (kNumReduceThreadsPerDigit - 1);

				u32 total = 0;
				for(u32 i = 0; i < kNumReduceElemsPerThreadPerDigit; i++)
				{
					u32 threadIdx = (setIdx * kNumReduceElemsPerThreadPerDigit + i) * kNumDigits;
					total += localCounts[threadIdx + digitIdx];
				}

				reduceCounters[digitIdx * kNumReduceThreadsPerDigit + setIdx] = total;
				reduceTotals[threadId] = total;
			}
		}

		// And do parallel reduction on the result of serial additions
		for(u32 i = 1; i < kNumReduceThreadsPerDigit; i <<= 1)
		{
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				if(threadId < kNumReduceThreads)
				{
					u32 digitIdx = threadId / kNumReduceThreadsPerDigit;
					u32 setIdx = threadId & (kNumReduceThreadsPerDigit - 1);

					reduceTotals[threadId] += reduceCounters[digitIdx * kNumReduceThreadsPerDigit + setIdx + i];
				}
			}

			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				if(threadId < kNumReduceThreads)
				{
					u32 digitIdx = threadId / kNumReduceThreadsPerDigit;
					u32 setIdx = threadId & (kNumReduceThreadsPerDigit - 1);

					reduceCounters[digitIdx * kNumReduceThreadsPerDigit + setIdx] = reduceTotals[threadId];
				}
			}
		}

		for(u32 threadId = 0; threadId < kNumThreads; threadId++)
		{
			if(threadId < kNumDigits)
				counts[groupIdx * kNumDigits + threadId] = reduceCounters[threadId * kNumReduceThreadsPerDigit];
		}
	}

	// PARALLEL:
	RadixSortClearMat::Get()->Execute(helperBuffers[0]);
	RadixSortCountMat::Get()->Execute(gpuSortProps.NumGroups, params, sortBuffers.Keys[0], helperBuffers[0]);
	RenderAPI::Instance().SubmitCommandBuffer(nullptr);

	// Compare with GPU count
	const u32 helperBufferLength = helperBuffers[0]->GetProperties().GetElementCount();
	Vector<u32> bufferCounts(helperBufferLength);
	helperBuffers[0]->ReadData(0, helperBufferLength * sizeof(u32), bufferCounts.data());

	for(u32 i = 0; i < (u32)counts.size(); i++)
		B3D_ASSERT(bufferCounts[i] == counts[i]);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////// Calculate offsets //////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// SERIAL:
	// Prefix sum over per-digit counts over all groups
	Vector<u32> perDigitPrefixSum(kNumDigits * kMaxNumGroups);
	for(u32 groupIdx = 0; groupIdx < gpuSortProps.NumGroups; groupIdx++)
	{
		for(u32 j = 0; j < kNumDigits; j++)
			perDigitPrefixSum[groupIdx * kNumDigits + j] = counts[groupIdx * kNumDigits + j];
	}

	// Prefix sum over per-digit counts over all groups
	//// Upsweep
	u32 offset = 1;
	for(u32 i = kMaxNumGroups >> 1; i > 0; i >>= 1)
	{
		for(u32 groupIdx = 0; groupIdx < kMaxNumGroups; groupIdx++)
		{
			if(groupIdx < i)
			{
				for(u32 j = 0; j < kNumDigits; j++)
				{
					u32 idx0 = (offset * (2 * groupIdx + 1) - 1) * kNumDigits + j;
					u32 idx1 = (offset * (2 * groupIdx + 2) - 1) * kNumDigits + j;

					perDigitPrefixSum[idx1] += perDigitPrefixSum[idx0];
				}
			}
		}

		offset <<= 1;
	}

	//// Downsweep
	u32 totalsPrefixSum[kNumDigits] = { 0 };
	for(u32 groupIdx = 0; groupIdx < kNumDigits; groupIdx++)
	{
		if(groupIdx < kNumDigits)
		{
			u32 idx = (kMaxNumGroups - 1) * kNumDigits + groupIdx;
			totalsPrefixSum[groupIdx] = perDigitPrefixSum[idx];
			perDigitPrefixSum[idx] = 0;
		}
	}

	for(u32 i = 1; i < kMaxNumGroups; i <<= 1)
	{
		offset >>= 1;

		for(u32 groupIdx = 0; groupIdx < kMaxNumGroups; groupIdx++)
		{
			if(groupIdx < i)
			{
				for(u32 j = 0; j < kNumDigits; j++)
				{
					u32 idx0 = (offset * (2 * groupIdx + 1) - 1) * kNumDigits + j;
					u32 idx1 = (offset * (2 * groupIdx + 2) - 1) * kNumDigits + j;

					u32 temp = perDigitPrefixSum[idx0];
					perDigitPrefixSum[idx0] = perDigitPrefixSum[idx1];
					perDigitPrefixSum[idx1] += temp;
				}
			}
		}
	}

	// Prefix sum over the total count
	for(u32 i = 1; i < kNumDigits; i++)
		totalsPrefixSum[i] += totalsPrefixSum[i - 1];

	// Make it exclusive by shifting
	for(u32 i = kNumDigits - 1; i > 0; i--)
		totalsPrefixSum[i] = totalsPrefixSum[i - 1];

	totalsPrefixSum[0] = 0;

	Vector<u32> offsets(gpuSortProps.NumGroups * kNumDigits);
	for(u32 groupIdx = 0; groupIdx < gpuSortProps.NumGroups; groupIdx++)
	{
		for(u32 i = 0; i < kNumDigits; i++)
			offsets[groupIdx * kNumDigits + i] = totalsPrefixSum[i] + perDigitPrefixSum[groupIdx * kNumDigits + i];
	}

	// PARALLEL:
	RadixSortPrefixScanMat::Get()->Execute(params, helperBuffers[0], helperBuffers[1]);
	RenderAPI::Instance().SubmitCommandBuffer(nullptr);

	// Compare with GPU offsets
	Vector<u32> bufferOffsets(helperBufferLength);
	helperBuffers[1]->ReadData(0, helperBufferLength * sizeof(u32), bufferOffsets.data());

	for(u32 i = 0; i < (u32)offsets.size(); i++)
		B3D_ASSERT(bufferOffsets[i] == offsets[i]);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////// Reorder ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// SERIAL:
	// Reorder within each tile
	Vector<u32> sortedKeys(inputKeys.size());
	u32 sGroupOffsets[kNumDigits];
	u32 sLocalScratch[kNumDigits * kNumThreads];
	u32 sTileTotals[kNumDigits];
	u32 sCurrentTileTotal[kNumDigits];

	for(u32 groupIdx = 0; groupIdx < gpuSortProps.NumGroups; groupIdx++)
	{
		for(u32 i = 0; i < kNumDigits; i++)
		{
			// Load offsets for this group to local memory
			sGroupOffsets[i] = offsets[groupIdx * kNumDigits + i];

			// Clear tile totals
			sTileTotals[i] = 0;
		}

		// Handle case when number of tiles isn't exactly divisible by number of groups, in
		// which case first N groups handle those extra tiles
		u32 tileIdx;
		u32 numTiles;
		if(groupIdx < gpuSortProps.ExtraTiles)
		{
			numTiles = gpuSortProps.TilesPerGroup + 1;
			tileIdx = groupIdx * numTiles;
		}
		else
		{
			numTiles = gpuSortProps.TilesPerGroup;
			tileIdx = groupIdx * numTiles + gpuSortProps.ExtraTiles;
		}

		// We need to generate per-thread offsets (prefix sum) of where to store the keys at
		// (This is equivalent to what was done in count & prefix sum shaders, except that was done per-group)

		//// First, count all digits
		u32 keyBegin[kNumThreads];
		for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			keyBegin[threadId] = tileIdx * kTileSize;

		auto prefixSum = [&sLocalScratch, &sCurrentTileTotal]()
		{
			// Upsweep to generate partial sums
			u32 offsets[kNumThreads];
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
				offsets[threadId] = 1;

			for(u32 i = kNumThreads >> 1; i > 0; i >>= 1)
			{
				for(u32 threadId = 0; threadId < kNumThreads; threadId++)
				{
					if(threadId < i)
					{
						// Note: If I run more than NUM_THREADS threads I wouldn't have to
						// iterate over all digits in a single thread
						// Note: Perhaps run part of this step serially for better performance
						for(u32 j = 0; j < kNumDigits; j++)
						{
							u32 idx0 = (offsets[threadId] * (2 * threadId + 1) - 1) * kNumDigits + j;
							u32 idx1 = (offsets[threadId] * (2 * threadId + 2) - 1) * kNumDigits + j;

							// Note: Check and remove bank conflicts
							sLocalScratch[idx1] += sLocalScratch[idx0];
						}
					}

					offsets[threadId] <<= 1;
				}
			}

			// Set tree roots to zero (prepare for downsweep)
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				if(threadId < kNumDigits)
				{
					u32 idx = (kNumThreads - 1) * kNumDigits + threadId;
					sCurrentTileTotal[threadId] = sLocalScratch[idx];

					sLocalScratch[idx] = 0;
				}
			}

			// Downsweep to calculate the prefix sum from partial sums that were generated
			// during upsweep
			for(u32 i = 1; i < kNumThreads; i <<= 1)
			{
				for(u32 threadId = 0; threadId < kNumThreads; threadId++)
				{
					offsets[threadId] >>= 1;

					if(threadId < i)
					{
						for(u32 j = 0; j < kNumDigits; j++)
						{
							u32 idx0 = (offsets[threadId] * (2 * threadId + 1) - 1) * kNumDigits + j;
							u32 idx1 = (offsets[threadId] * (2 * threadId + 2) - 1) * kNumDigits + j;

							// Note: Check and resolve bank conflicts
							u32 temp = sLocalScratch[idx0];
							sLocalScratch[idx0] = sLocalScratch[idx1];
							sLocalScratch[idx1] += temp;
						}
					}
				}
			}
		};

		for(u32 tileIdx = 0; tileIdx < numTiles; tileIdx++)
		{
			// Zero out local counter
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
				for(u32 i = 0; i < kNumDigits; i++)
					sLocalScratch[i * kNumThreads + threadId] = 0;

			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				for(u32 i = 0; i < kKeysPerLoop; i++)
				{
					u32 idx = keyBegin[threadId] + threadId * kKeysPerLoop + i;
					u32 key = inputKeys[idx];
					u32 digit = (key >> bitOffset) & kKeyMask;

					sLocalScratch[threadId * kNumDigits + digit] += 1;
				}
			}

			prefixSum();

			// Actually re-order the keys
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				u32 localOffsets[kNumDigits];
				for(u32 i = 0; i < kNumDigits; i++)
					localOffsets[i] = 0;

				for(u32 i = 0; i < kKeysPerLoop; i++)
				{
					u32 idx = keyBegin[threadId] + threadId * kKeysPerLoop + i;
					u32 key = inputKeys[idx];
					u32 digit = (key >> bitOffset) & kKeyMask;

					u32 offset = sGroupOffsets[digit] + sTileTotals[digit] + sLocalScratch[threadId * kNumDigits + digit] + localOffsets[digit];
					localOffsets[digit]++;

					// Note: First write to local memory then attempt to coalesce when writing to global?
					sortedKeys[offset] = key;
				}
			}

			// Update tile totals
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				if(threadId < kNumDigits)
					sTileTotals[threadId] += sCurrentTileTotal[threadId];
			}

			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
				keyBegin[threadId] += kTileSize;
		}

		if(groupIdx == (gpuSortProps.NumGroups - 1) && gpuSortProps.ExtraKeys > 0)
		{
			// Zero out local counter
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
				for(u32 i = 0; i < kNumDigits; i++)
					sLocalScratch[i * kNumThreads + threadId] = 0;

			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				for(u32 i = 0; i < kKeysPerLoop; i++)
				{
					u32 localIdx = threadId * kKeysPerLoop + i;

					if(localIdx >= gpuSortProps.ExtraKeys)
						continue;

					u32 idx = keyBegin[threadId] + localIdx;
					u32 key = inputKeys[idx];
					u32 digit = (key >> bitOffset) & kKeyMask;

					sLocalScratch[threadId * kNumDigits + digit] += 1;
				}
			}

			prefixSum();

			// Actually re-order the keys
			for(u32 threadId = 0; threadId < kNumThreads; threadId++)
			{
				u32 localOffsets[kNumDigits];
				for(u32 i = 0; i < kNumDigits; i++)
					localOffsets[i] = 0;

				for(u32 i = 0; i < kKeysPerLoop; i++)
				{
					u32 localIdx = threadId * kKeysPerLoop + i;

					if(localIdx >= gpuSortProps.ExtraKeys)
						continue;

					u32 idx = keyBegin[threadId] + localIdx;
					u32 key = inputKeys[idx];
					u32 digit = (key >> bitOffset) & kKeyMask;

					u32 offset = sGroupOffsets[digit] + sTileTotals[digit] + sLocalScratch[threadId * kNumDigits + digit] + localOffsets[digit];
					localOffsets[digit]++;

					// Note: First write to local memory then attempt to coalesce when writing to global?
					sortedKeys[offset] = key;
				}
			}
		}
	}

	// PARALLEL:
	RadixSortReorderMat::Get()->Execute(gpuSortProps.NumGroups, params, helperBuffers[1], sortBuffers, 0);
	RenderAPI::Instance().SubmitCommandBuffer(nullptr);

	// Compare with GPU keys
	Vector<u32> bufferSortedKeys(count);
	sortBuffers.Keys[1]->ReadData(0, count * sizeof(u32), bufferSortedKeys.data());

	for(u32 i = 0; i < count; i++)
		B3D_ASSERT(bufferSortedKeys[i] == sortedKeys[i]);

	// Ensure everything is actually sorted
	B3D_ASSERT(std::is_sorted(sortedKeys.begin(), sortedKeys.end()));
}
}} // namespace bs::ct
