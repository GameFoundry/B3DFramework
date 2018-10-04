//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsGpuSort.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Math/BsRandom.h"
#include "Renderer/BsRendererUtility.h"

namespace bs { namespace ct
{
	static constexpr UINT32 BIT_COUNT = 32;
	static constexpr UINT32 RADIX_NUM_BITS = 4;
	static constexpr UINT32 NUM_DIGITS = 1 << RADIX_NUM_BITS;
	static constexpr UINT32 KEY_MASK = (NUM_DIGITS - 1);
	static constexpr UINT32 NUM_PASSES = BIT_COUNT / RADIX_NUM_BITS;

	static constexpr UINT32 NUM_THREADS = 128;
	static constexpr UINT32 KEYS_PER_LOOP = 8;
	static constexpr UINT32 TILE_SIZE = NUM_THREADS * KEYS_PER_LOOP;
	static constexpr UINT32 MAX_NUM_GROUPS = 64;

	RadixSortParamsDef gRadixSortParamsDef;

	/** Contains various constants required during the GpuSort algorithm. */
	struct GpuSortProperties
	{
		GpuSortProperties(UINT32 count)
			: count(count)
		{ }

		const UINT32 count;
		const UINT32 numTiles = count / TILE_SIZE;
		const UINT32 numGroups = Math::clamp(numTiles, 1U, MAX_NUM_GROUPS);

		const UINT32 tilesPerGroup = numTiles / numGroups;
		const UINT32 extraTiles = numTiles % numGroups;
		const UINT32 extraKeys = count % TILE_SIZE;
	};

	/** Set up common defines required by all radix sort shaders. */
	void initCommonDefines(ShaderDefines& defines)
	{
		defines.set("RADIX_NUM_BITS", RADIX_NUM_BITS);
		defines.set("NUM_THREADS", NUM_THREADS);
		defines.set("KEYS_PER_LOOP", KEYS_PER_LOOP);
		defines.set("MAX_NUM_GROUPS", MAX_NUM_GROUPS);
	}

	/** 
	 * Creates a new GPU parameter block buffer according to gRadixSortParamDef definition and writes GpuSort properties
	 * into the buffer.
	 */
	SPtr<GpuParamBlockBuffer> createGpuSortParams(const GpuSortProperties& props)
	{
		SPtr<GpuParamBlockBuffer> buffer = gRadixSortParamsDef.createBuffer();

		gRadixSortParamsDef.gTilesPerGroup.set(buffer, props.tilesPerGroup);
		gRadixSortParamsDef.gNumGroups.set(buffer, props.numGroups);
		gRadixSortParamsDef.gNumExtraTiles.set(buffer, props.extraTiles);
		gRadixSortParamsDef.gNumExtraKeys.set(buffer, props.extraKeys);
		gRadixSortParamsDef.gBitOffset.set(buffer, 0);

		return buffer;
	}

	/** 
	 * Checks can the provided buffer be used for GPU sort operation. Returns a pointer to the error message if check failed
	 * or nullptr if check passed.
	 */
	const char* checkSortBuffer(GpuBuffer& buffer)
	{
		static constexpr const char* INVALID_GPU_WRITE_MSG = 
			"All buffers provided to GpuSort must be created with GBU_LOADSTORE flags enabled.";
		static constexpr const char* INVALID_TYPE_MSG = 
			"All buffers provided to GpuSort must be of GBT_STANDARD type.";
		static constexpr const char* INVALID_FORMAT_MSG = 
			"All buffers provided to GpuSort must use a 32-bit unsigned integer format.";

		const GpuBufferProperties& bufferProps = buffer.getProperties();
		if ((bufferProps.getUsage() & GBU_LOADSTORE) != GBU_LOADSTORE)
			return INVALID_GPU_WRITE_MSG;

		if(bufferProps.getType() != GBT_STANDARD)
			return INVALID_TYPE_MSG;

		if(bufferProps.getFormat() != BF_32X1U)
			return INVALID_FORMAT_MSG;

		return nullptr;
	}

	/** Creates a helper buffers used for storing intermediate information during GpuSort::sort. */
	SPtr<GpuBuffer> createHelperBuffer()
	{
		GPU_BUFFER_DESC desc;
		desc.elementCount = MAX_NUM_GROUPS * NUM_DIGITS;
		desc.format = BF_32X1U;
		desc.usage = GBU_LOADSTORE;
		desc.type = GBT_STANDARD;

		return GpuBuffer::create(desc);
	}

	RadixSortClearMat::RadixSortClearMat()
	{
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputParam);
	}

	void RadixSortClearMat::_initDefines(ShaderDefines& defines)
	{
		initCommonDefines(defines);
	}

	void RadixSortClearMat::execute(const SPtr<GpuBuffer>& outputOffsets)
	{
		BS_RENMAT_PROFILE_BLOCK

		mOutputParam.set(outputOffsets);

		bind();
		RenderAPI::instance().dispatchCompute(1);
	}

	RadixSortCountMat::RadixSortCountMat()
	{
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gInputKeys", mInputKeysParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutputCounts", mOutputCountsParam);
	}

	void RadixSortCountMat::_initDefines(ShaderDefines& defines)
	{
		initCommonDefines(defines);
	}

	void RadixSortCountMat::execute(UINT32 numGroups, const SPtr<GpuParamBlockBuffer>& params, 
		const SPtr<GpuBuffer>& inputKeys, const SPtr<GpuBuffer>& outputOffsets)
	{
		BS_RENMAT_PROFILE_BLOCK

		mInputKeysParam.set(inputKeys);
		mOutputCountsParam.set(outputOffsets);

		mParams->setParamBlockBuffer("Params", params);

		bind();
		RenderAPI::instance().dispatchCompute(numGroups);
	}

	RadixSortPrefixScanMat::RadixSortPrefixScanMat()
	{
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gInputCounts", mInputCountsParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutputOffsets", mOutputOffsetsParam);
	}

	void RadixSortPrefixScanMat::_initDefines(ShaderDefines& defines)
	{
		initCommonDefines(defines);
	}

	void RadixSortPrefixScanMat::execute(const SPtr<GpuParamBlockBuffer>& params, const SPtr<GpuBuffer>& inputCounts, 
		const SPtr<GpuBuffer>& outputOffsets)
	{
		BS_RENMAT_PROFILE_BLOCK

		mInputCountsParam.set(inputCounts);
		mOutputOffsetsParam.set(outputOffsets);

		mParams->setParamBlockBuffer("Params", params);

		bind();
		RenderAPI::instance().dispatchCompute(1);
	}

	RadixSortReorderMat::RadixSortReorderMat()
	{
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gInputOffsets", mInputOffsetsBufferParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gInputKeys", mInputKeysBufferParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gInputValues", mInputValuesBufferParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutputKeys", mOutputKeysBufferParam);
		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gOutputValues", mOutputValuesBufferParam);
	}

	void RadixSortReorderMat::_initDefines(ShaderDefines& defines)
	{
		initCommonDefines(defines);
	}

	void RadixSortReorderMat::execute(UINT32 numGroups, const SPtr<GpuParamBlockBuffer>& params, 
		const SPtr<GpuBuffer>& inputPrefix, const GpuSortBuffers& buffers, UINT32 inputBufferIdx)
	{
		BS_RENMAT_PROFILE_BLOCK

		const UINT32 outputBufferIdx = (inputBufferIdx + 1) % 2;

		mInputOffsetsBufferParam.set(inputPrefix);
		mInputKeysBufferParam.set(buffers.keys[inputBufferIdx]);
		mInputValuesBufferParam.set(buffers.values[inputBufferIdx]);
		mOutputKeysBufferParam.set(buffers.keys[outputBufferIdx]);
		mOutputValuesBufferParam.set(buffers.values[outputBufferIdx]);
		
		mParams->setParamBlockBuffer("Params", params);

		bind();
		RenderAPI::instance().dispatchCompute(numGroups);
	}

	GpuSort::GpuSort()
	{
		mHelperBuffers[0] = createHelperBuffer();
		mHelperBuffers[1] = createHelperBuffer();
	}

	UINT32 GpuSort::sort(const GpuSortBuffers& buffers, UINT32 numKeys, UINT32 keyMask)
	{
		// Nothing to do if no input or output key buffers
		if(buffers.keys[0] == nullptr || buffers.keys[1] == nullptr)
			return 0;

		// Check if all buffers have been created with required options
		const char* errorMsg = nullptr;
		for(UINT32 i = 0; i < 2; i++)
		{
			errorMsg = checkSortBuffer(*buffers.keys[i]);
			if(errorMsg) break;

			if(buffers.values[i])
			{
				errorMsg = checkSortBuffer(*buffers.values[i]);
				if(errorMsg) break;
			}
		}

		if(errorMsg)
		{
			LOGERR("GpuSort failed: " + String(errorMsg));
			return 0;
		}

		// Check if all buffers have the same size
		bool validSize = buffers.keys[0]->getSize() == buffers.keys[1]->getSize();
		if(buffers.values[0] && buffers.values[1])
		{
			validSize = buffers.keys[0]->getSize() == buffers.values[0]->getSize() &&
				buffers.keys[0]->getSize() == buffers.values[1]->getSize();

		}

		if (!validSize)
		{
			LOGERR("GpuSort failed: All sort buffers must have the same size.");
			return 0;
		}

		const GpuSortProperties gpuSortProps(numKeys);
		SPtr<GpuParamBlockBuffer> params = createGpuSortParams(gpuSortProps);

		UINT32 bitOffset = 0;
		UINT32 inputBufferIdx = 0;
		for(UINT32 i = 0; i < NUM_PASSES; i++)
		{
			if(((KEY_MASK << bitOffset) & keyMask) != 0)
			{
				gRadixSortParamsDef.gBitOffset.set(params, bitOffset);

				RadixSortClearMat::get()->execute(mHelperBuffers[0]);
				RadixSortCountMat::get()->execute(gpuSortProps.numGroups, params, buffers.keys[inputBufferIdx], mHelperBuffers[0]);
				RadixSortPrefixScanMat::get()->execute(params, mHelperBuffers[0], mHelperBuffers[1]);
				RadixSortReorderMat::get()->execute(gpuSortProps.numGroups, params, mHelperBuffers[1], buffers, inputBufferIdx);

				inputBufferIdx = (inputBufferIdx + 1) % 2;
			}

			bitOffset += RADIX_NUM_BITS;
		}

		return inputBufferIdx;
	}

	GpuSortBuffers GpuSort::createSortBuffers(UINT32 numElements, bool values)
	{
		GpuSortBuffers output;

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.elementCount = numElements;
		bufferDesc.format = BF_32X1U;
		bufferDesc.type = GBT_STANDARD;
		bufferDesc.usage = GBU_LOADSTORE;

		output.keys[0] = GpuBuffer::create(bufferDesc);
		output.keys[1] = GpuBuffer::create(bufferDesc);

		if(values)
		{
			output.values[0] = GpuBuffer::create(bufferDesc);
			output.values[1] = GpuBuffer::create(bufferDesc);
		}

		return output;
	}

	// Note: This test isn't currently hooked up anywhere. It might be a good idea to set it up as a unit test, but it would
	// require exposing parts of GpuSort to the public, which I don't feel like it's worth doing just for a test. So instead
	// just make sure to run the test below if you modify any of the GpuSort code.
	void runSortTest()
	{
		// Generate test keys
		static constexpr UINT32 NUM_INPUT_KEYS = 10000;
		Vector<UINT32> inputKeys;
		inputKeys.reserve(NUM_INPUT_KEYS);

		Random random;
		for(UINT32 i = 0; i < NUM_INPUT_KEYS; i++)
			inputKeys.push_back(random.getRange(0, 15) << 4 | std::min(NUM_DIGITS - 1, (i / (NUM_INPUT_KEYS / 16))));

		const auto count = (UINT32)inputKeys.size();
		UINT32 bitOffset = 4;
		UINT32 bitMask = (1 << RADIX_NUM_BITS) - 1;

		// Prepare buffers
		const GpuSortProperties gpuSortProps(count);
		SPtr<GpuParamBlockBuffer> params = createGpuSortParams(gpuSortProps);

		gRadixSortParamsDef.gBitOffset.set(params, bitOffset);

		GpuSortBuffers sortBuffers = GpuSort::createSortBuffers(count);
		sortBuffers.keys[0]->writeData(0, sortBuffers.keys[0]->getSize(), inputKeys.data(), BWT_DISCARD);

		SPtr<GpuBuffer> helperBuffers[2];
		helperBuffers[0] = createHelperBuffer();
		helperBuffers[1] = createHelperBuffer();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////// Count keys per group //////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// SERIAL:
		Vector<UINT32> counts(gpuSortProps.numGroups * NUM_DIGITS);
		for(UINT32 groupIdx = 0; groupIdx < gpuSortProps.numGroups; groupIdx++)
		{
			// Count keys per thread
			UINT32 localCounts[NUM_THREADS * NUM_DIGITS] = { 0 };

			UINT32 tileIdx;
			UINT32 numTiles;
			if(groupIdx < gpuSortProps.extraTiles)
			{
				numTiles = gpuSortProps.tilesPerGroup + 1;
				tileIdx = groupIdx * numTiles;
			}
			else
			{
				numTiles = gpuSortProps.tilesPerGroup;
				tileIdx = groupIdx * numTiles + gpuSortProps.extraTiles;
			}

			UINT32 keyBegin = tileIdx * TILE_SIZE;
			UINT32 keyEnd = keyBegin + numTiles * TILE_SIZE;	

			while(keyBegin < keyEnd)
			{
				for(UINT32 threadIdx = 0; threadIdx < NUM_THREADS; threadIdx++)
				{
					UINT32 key = inputKeys[keyBegin + threadIdx];
					UINT32 digit = (key >> bitOffset) & bitMask;

					localCounts[threadIdx * NUM_DIGITS + digit] += 1;
				}

				keyBegin += NUM_THREADS;
			}

			if(groupIdx == (gpuSortProps.numGroups - 1))
			{
				keyBegin = keyEnd;
				keyEnd = keyBegin + gpuSortProps.extraKeys;

				while(keyBegin < keyEnd)
				{
					for (UINT32 threadIdx = 0; threadIdx < NUM_THREADS; threadIdx++)
					{
						if((keyBegin + threadIdx) < keyEnd)
						{
							UINT32 key = inputKeys[keyBegin + threadIdx];
							UINT32 digit = (key >> bitOffset) & bitMask;

							localCounts[threadIdx * NUM_DIGITS + digit] += 1;
						}
					}

					keyBegin += NUM_THREADS;
				}
			}

			// Sum up all key counts in a group
			static constexpr UINT32 NUM_REDUCE_THREADS = 64;
			static constexpr UINT32 NUM_REDUCE_THREADS_PER_DIGIT = NUM_REDUCE_THREADS / NUM_DIGITS;
			static constexpr UINT32 NUM_REDUCE_ELEMS_PER_THREAD_PER_DIGIT = NUM_THREADS / NUM_REDUCE_THREADS_PER_DIGIT;

			UINT32 reduceCounters[NUM_REDUCE_THREADS] = { 0 };
			UINT32 reduceTotals[NUM_REDUCE_THREADS] = { 0 };
			for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
			{
				if(threadId < NUM_REDUCE_THREADS)
				{
					UINT32 digitIdx = threadId / NUM_REDUCE_THREADS_PER_DIGIT;
					UINT32 setIdx = threadId & (NUM_REDUCE_THREADS_PER_DIGIT - 1);

					UINT32 total = 0;
					for(UINT32 i = 0; i < NUM_REDUCE_ELEMS_PER_THREAD_PER_DIGIT; i++)
					{
						UINT32 threadIdx = (setIdx * NUM_REDUCE_ELEMS_PER_THREAD_PER_DIGIT + i) * NUM_DIGITS;
						total += localCounts[threadIdx + digitIdx];
					}

					reduceCounters[digitIdx * NUM_REDUCE_THREADS_PER_DIGIT + setIdx] = total;
					reduceTotals[threadId] = total;

				}
			}

			// And do parallel reduction on the result of serial additions
			for (UINT32 i = 1; i < NUM_REDUCE_THREADS_PER_DIGIT; i <<= 1)
			{
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					if (threadId < NUM_REDUCE_THREADS)
					{
						UINT32 digitIdx = threadId / NUM_REDUCE_THREADS_PER_DIGIT;
						UINT32 setIdx = threadId & (NUM_REDUCE_THREADS_PER_DIGIT - 1);

						reduceTotals[threadId] += reduceCounters[digitIdx * NUM_REDUCE_THREADS_PER_DIGIT + setIdx + i];
					}
				}

				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					if (threadId < NUM_REDUCE_THREADS)
					{
						UINT32 digitIdx = threadId / NUM_REDUCE_THREADS_PER_DIGIT;
						UINT32 setIdx = threadId & (NUM_REDUCE_THREADS_PER_DIGIT - 1);

						reduceCounters[digitIdx * NUM_REDUCE_THREADS_PER_DIGIT + setIdx] = reduceTotals[threadId];
					}
				}
			}

			for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
			{
				if(threadId < NUM_DIGITS)
					counts[groupIdx * NUM_DIGITS + threadId] = reduceCounters[threadId * NUM_REDUCE_THREADS_PER_DIGIT];
			}
		}

		// PARALLEL:
		RadixSortClearMat::get()->execute(helperBuffers[0]);
		RadixSortCountMat::get()->execute(gpuSortProps.numGroups, params, sortBuffers.keys[0], helperBuffers[0]);

		// Compare with GPU count
		const UINT32 helperBufferLength = helperBuffers[0]->getProperties().getElementCount();
		Vector<UINT32> bufferCounts(helperBufferLength);
		helperBuffers[0]->readData(0, helperBufferLength * sizeof(UINT32), bufferCounts.data());

		for(UINT32 i = 0; i < (UINT32)counts.size(); i++)
			assert(bufferCounts[i] == counts[i]);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////// Calculate offsets //////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// SERIAL:
		// Prefix sum over per-digit counts over all groups
		Vector<UINT32> perDigitPrefixSum(NUM_DIGITS * MAX_NUM_GROUPS);
		for(UINT32 groupIdx = 0; groupIdx < gpuSortProps.numGroups; groupIdx++)
		{
			for (UINT32 j = 0; j < NUM_DIGITS; j++)
				perDigitPrefixSum[groupIdx * NUM_DIGITS + j] = counts[groupIdx * NUM_DIGITS + j];
		}

		// Prefix sum over per-digit counts over all groups
		//// Upsweep
		UINT32 offset = 1;
		for (UINT32 i = MAX_NUM_GROUPS >> 1; i > 0; i >>= 1)
		{
			for (UINT32 groupIdx = 0; groupIdx < MAX_NUM_GROUPS; groupIdx++)
			{
				if (groupIdx < i)
				{
					for (UINT32 j = 0; j < NUM_DIGITS; j++)
					{
						UINT32 idx0 = (offset * (2 * groupIdx + 1) - 1) * NUM_DIGITS + j;
						UINT32 idx1 = (offset * (2 * groupIdx + 2) - 1) * NUM_DIGITS + j;

						perDigitPrefixSum[idx1] += perDigitPrefixSum[idx0];
					}
				}
			}

			offset <<= 1;
		}

		//// Downsweep
		UINT32 totalsPrefixSum[NUM_DIGITS] = { 0 };
		for(UINT32 groupIdx = 0; groupIdx < NUM_DIGITS; groupIdx++)
		{
			if (groupIdx < NUM_DIGITS)
			{
				UINT32 idx = (MAX_NUM_GROUPS - 1) * NUM_DIGITS + groupIdx;
				totalsPrefixSum[groupIdx] = perDigitPrefixSum[idx];
				perDigitPrefixSum[idx] = 0;
			}
		}

		for (UINT32 i = 1; i < MAX_NUM_GROUPS; i <<= 1)
		{
			offset >>= 1;

			for (UINT32 groupIdx = 0; groupIdx < MAX_NUM_GROUPS; groupIdx++)
			{
				if (groupIdx < i)
				{
					for (UINT32 j = 0; j < NUM_DIGITS; j++)
					{
						UINT32 idx0 = (offset * (2 * groupIdx + 1) - 1) * NUM_DIGITS + j;
						UINT32 idx1 = (offset * (2 * groupIdx + 2) - 1) * NUM_DIGITS + j;

						UINT32 temp = perDigitPrefixSum[idx0];
						perDigitPrefixSum[idx0] = perDigitPrefixSum[idx1];
						perDigitPrefixSum[idx1] += temp;
					}
				}
			}
		}

		// Prefix sum over the total count
		for(UINT32 i = 1; i < NUM_DIGITS; i++)
			totalsPrefixSum[i] += totalsPrefixSum[i - 1];

		// Make it exclusive by shifting
		for(UINT32 i = NUM_DIGITS - 1; i > 0; i--)
			totalsPrefixSum[i] = totalsPrefixSum[i - 1];

		totalsPrefixSum[0] = 0;

		Vector<UINT32> offsets(gpuSortProps.numGroups * NUM_DIGITS);
		for (UINT32 groupIdx = 0; groupIdx < gpuSortProps.numGroups; groupIdx++)
		{
			for (UINT32 i = 0; i < NUM_DIGITS; i++)
				offsets[groupIdx * NUM_DIGITS + i] = totalsPrefixSum[i] + perDigitPrefixSum[groupIdx * NUM_DIGITS + i];
		}

		// PARALLEL:
		RadixSortPrefixScanMat::get()->execute(params, helperBuffers[0], helperBuffers[1]);

		// Compare with GPU offsets
		Vector<UINT32> bufferOffsets(helperBufferLength);
		helperBuffers[1]->readData(0, helperBufferLength * sizeof(UINT32), bufferOffsets.data());

		for(UINT32 i = 0; i < (UINT32)offsets.size(); i++)
			assert(bufferOffsets[i] == offsets[i]);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////// Reorder ////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// SERIAL:
		// Reorder within each tile
		Vector<UINT32> sortedKeys(inputKeys.size());
		UINT32 sGroupOffsets[NUM_DIGITS];
		UINT32 sLocalScratch[NUM_DIGITS * NUM_THREADS];
		UINT32 sTileTotals[NUM_DIGITS];
		UINT32 sCurrentTileTotal[NUM_DIGITS];

		for(UINT32 groupIdx = 0; groupIdx < gpuSortProps.numGroups; groupIdx++)
		{
			for(UINT32 i = 0; i < NUM_DIGITS; i++)
			{
				// Load offsets for this group to local memory
				sGroupOffsets[i] = offsets[groupIdx * NUM_DIGITS + i];

				// Clear tile totals
				sTileTotals[i] = 0;
			}

			// Handle case when number of tiles isn't exactly divisible by number of groups, in
			// which case first N groups handle those extra tiles
			UINT32 tileIdx;
			UINT32 numTiles;
			if(groupIdx < gpuSortProps.extraTiles)
			{
				numTiles = gpuSortProps.tilesPerGroup + 1;
				tileIdx = groupIdx * numTiles;
			}
			else
			{
				numTiles = gpuSortProps.tilesPerGroup;
				tileIdx = groupIdx * numTiles + gpuSortProps.extraTiles;
			}

			// We need to generate per-thread offsets (prefix sum) of where to store the keys at
			// (This is equivalent to what was done in count & prefix sum shaders, except that was done per-group)

			//// First, count all digits
			UINT32 keyBegin[NUM_THREADS];
			for(UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				keyBegin[threadId] = tileIdx * TILE_SIZE;

			auto prefixSum = [&sLocalScratch, &sCurrentTileTotal]()
			{
				// Upsweep to generate partial sums
				UINT32 offsets[NUM_THREADS];
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
					offsets[threadId] = 1;

				for (UINT32 i = NUM_THREADS >> 1; i > 0; i >>= 1)
				{
					for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
					{
						if (threadId < i)
						{
							// Note: If I run more than NUM_THREADS threads I wouldn't have to
							// iterate over all digits in a single thread
							// Note: Perhaps run part of this step serially for better performance
							for (UINT32 j = 0; j < NUM_DIGITS; j++)
							{
								UINT32 idx0 = (offsets[threadId] * (2 * threadId + 1) - 1) * NUM_DIGITS + j;
								UINT32 idx1 = (offsets[threadId] * (2 * threadId + 2) - 1) * NUM_DIGITS + j;

								// Note: Check and remove bank conflicts
								sLocalScratch[idx1] += sLocalScratch[idx0];
							}
						}

						offsets[threadId] <<= 1;
					}
				}

				// Set tree roots to zero (prepare for downsweep)
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					if (threadId < NUM_DIGITS)
					{
						UINT32 idx = (NUM_THREADS - 1) * NUM_DIGITS + threadId;
						sCurrentTileTotal[threadId] = sLocalScratch[idx];

						sLocalScratch[idx] = 0;
					}
				}

				// Downsweep to calculate the prefix sum from partial sums that were generated
				// during upsweep		
				for (UINT32 i = 1; i < NUM_THREADS; i <<= 1)
				{
					for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
					{
						offsets[threadId] >>= 1;

						if (threadId < i)
						{
							for (UINT32 j = 0; j < NUM_DIGITS; j++)
							{
								UINT32 idx0 = (offsets[threadId] * (2 * threadId + 1) - 1) * NUM_DIGITS + j;
								UINT32 idx1 = (offsets[threadId] * (2 * threadId + 2) - 1) * NUM_DIGITS + j;

								// Note: Check and resolve bank conflicts
								UINT32 temp = sLocalScratch[idx0];
								sLocalScratch[idx0] = sLocalScratch[idx1];
								sLocalScratch[idx1] += temp;
							}
						}
					}
				}
			};

			for(UINT32 tileIdx = 0; tileIdx < numTiles; tileIdx++)
			{
				// Zero out local counter
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
					for (UINT32 i = 0; i < NUM_DIGITS; i++)
						sLocalScratch[i * NUM_THREADS + threadId] = 0;

				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					for (UINT32 i = 0; i < KEYS_PER_LOOP; i++)
					{
						UINT32 idx = keyBegin[threadId] + threadId * KEYS_PER_LOOP + i;
						UINT32 key = inputKeys[idx];
						UINT32 digit = (key >> bitOffset) & KEY_MASK;

						sLocalScratch[threadId * NUM_DIGITS + digit] += 1;
					}
				}

				prefixSum();

				// Actually re-order the keys
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					UINT32 localOffsets[NUM_DIGITS];
					for (UINT32 i = 0; i < NUM_DIGITS; i++)
						localOffsets[i] = 0;

					for (UINT32 i = 0; i < KEYS_PER_LOOP; i++)
					{
						UINT32 idx = keyBegin[threadId] + threadId * KEYS_PER_LOOP + i;
						UINT32 key = inputKeys[idx];
						UINT32 digit = (key >> bitOffset) & KEY_MASK;

						UINT32 offset = sGroupOffsets[digit] + sTileTotals[digit] + sLocalScratch[threadId * NUM_DIGITS + digit] + localOffsets[digit];
						localOffsets[digit]++;

						// Note: First write to local memory then attempt to coalesce when writing to global?
						sortedKeys[offset] = key;
					}
				}

				// Update tile totals
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					if (threadId < NUM_DIGITS)
						sTileTotals[threadId] += sCurrentTileTotal[threadId];
				}

				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
					keyBegin[threadId] += TILE_SIZE;
			}

			if (groupIdx == (gpuSortProps.numGroups - 1) && gpuSortProps.extraKeys > 0)
			{
				// Zero out local counter
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
					for (UINT32 i = 0; i < NUM_DIGITS; i++)
						sLocalScratch[i * NUM_THREADS + threadId] = 0;

				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					for (UINT32 i = 0; i < KEYS_PER_LOOP; i++)
					{
						UINT32 localIdx = threadId * KEYS_PER_LOOP + i;

						if (localIdx >= gpuSortProps.extraKeys)
							continue;

						UINT32 idx = keyBegin[threadId] + localIdx;
						UINT32 key = inputKeys[idx];
						UINT32 digit = (key >> bitOffset) & KEY_MASK;

						sLocalScratch[threadId * NUM_DIGITS + digit] += 1;
					}
				}

				prefixSum();

				// Actually re-order the keys
				for (UINT32 threadId = 0; threadId < NUM_THREADS; threadId++)
				{
					UINT32 localOffsets[NUM_DIGITS];
					for (UINT32 i = 0; i < NUM_DIGITS; i++)
						localOffsets[i] = 0;

					for (UINT32 i = 0; i < KEYS_PER_LOOP; i++)
					{
						UINT32 localIdx = threadId * KEYS_PER_LOOP + i;

						if (localIdx >= gpuSortProps.extraKeys)
							continue;

						UINT32 idx = keyBegin[threadId] + localIdx;
						UINT32 key = inputKeys[idx];
						UINT32 digit = (key >> bitOffset) & KEY_MASK;

						UINT32 offset = sGroupOffsets[digit] + sTileTotals[digit] + sLocalScratch[threadId * NUM_DIGITS + digit] + localOffsets[digit];
						localOffsets[digit]++;

						// Note: First write to local memory then attempt to coalesce when writing to global?
						sortedKeys[offset] = key;
					}
				}
			}
		}

		// PARALLEL:
		RadixSortReorderMat::get()->execute(gpuSortProps.numGroups, params, helperBuffers[1], sortBuffers, 0);

		// Compare with GPU keys
		Vector<UINT32> bufferSortedKeys(count);
		sortBuffers.keys[1]->readData(0, count * sizeof(UINT32), bufferSortedKeys.data());

		for(UINT32 i = 0; i < count; i++)
			assert(bufferSortedKeys[i] == sortedKeys[i]);

		// Ensure everything is actually sorted
		assert(std::is_sorted(sortedKeys.begin(), sortedKeys.end()));
	}
}}
