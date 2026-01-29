#include "$ENGINE$/RadixSortCommon.bslinc"

shader RadixSortReorder
{
	mixin RadixSortCommon;

	code
	{
		Buffer<uint> gInputKeys;
		Buffer<uint> gInputValues;
		Buffer<uint> gInputOffsets;
		RWBuffer<uint> gOutputKeys;
		RWBuffer<uint> gOutputValues;
		
		groupshared uint sGroupOffsets[NUM_DIGITS];
		groupshared uint sLocalScratch[NUM_DIGITS * NUM_THREADS];
		groupshared uint sTileTotals[NUM_DIGITS];
		groupshared uint sCurrentTileTotal[NUM_DIGITS];
		
		// Transforms counts in sLocalScratch into a prefix sum. Also outputs a total sum in sTileTotalSum.
		void prefixSum(uint threadId)
		{
			// Upsweep to generate partial sums
			uint offset = 1;
			for (uint i = NUM_THREADS >> 1; i > 0; i >>= 1)
			{
				GroupMemoryBarrierWithGroupSync();
			
				if (threadId < i)
				{
					// Note: If I run more than NUM_THREADS threads I wouldn't have to
					// iterate over all digits in a single thread
					// Note: Perhaps run part of this step serially for better performance
					for (uint j = 0; j < NUM_DIGITS; j++)
					{
						uint idx0 = (offset * (2 * threadId + 1) - 1) * NUM_DIGITS + j;
						uint idx1 = (offset * (2 * threadId + 2) - 1) * NUM_DIGITS + j;

						// Note: Check and remove bank conflicts
						sLocalScratch[idx1] += sLocalScratch[idx0];
					}
				}

				offset <<= 1;
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			// Set tree roots to zero (prepare for downsweep)
			if(threadId < NUM_DIGITS)
			{
				uint idx = (NUM_THREADS - 1) * NUM_DIGITS + threadId;
				sCurrentTileTotal[threadId] = sLocalScratch[idx];
				
				sLocalScratch[idx] = 0;
			}

			// Downsweep to calculate the prefix sum from partial sums that were generated
			// during upsweep		
			for (uint i = 1; i < NUM_THREADS; i <<= 1)
			{
				GroupMemoryBarrierWithGroupSync();
				offset >>= 1;

				if (threadId < i)
				{
					for (uint j = 0; j < NUM_DIGITS; j++)
					{
						uint idx0 = (offset * (2 * threadId + 1) - 1) * NUM_DIGITS + j;
						uint idx1 = (offset * (2 * threadId + 2) - 1) * NUM_DIGITS + j;

						// Note: Check and resolve bank conflicts
						uint temp = sLocalScratch[idx0];
						sLocalScratch[idx0] = sLocalScratch[idx1];
						sLocalScratch[idx1] += temp;
					}
				}
			}
			
			GroupMemoryBarrierWithGroupSync();
		}	
		
		[numthreads(NUM_THREADS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			uint threadId = groupThreadId.x;
			
			if(threadId < NUM_DIGITS)
			{
				// Load offsets for this group to local memory
				sGroupOffsets[threadId] = gInputOffsets[groupId.x * NUM_DIGITS + threadId];
				
				// Clear tile totals
				sTileTotals[threadId] = 0;
			}
			
			// Handle case when number of tiles isn't exactly divisible by number of groups, in
			// which case first N groups handle those extra tiles
			uint tileIdx, tileCount;
			if(groupId.x < gNumExtraTiles)
			{
				tileCount = gTilesPerGroup + 1;
				tileIdx = groupId.x * tileCount;
			}
			else
			{
				tileCount = gTilesPerGroup;
				tileIdx = groupId.x * tileCount + gNumExtraTiles;
			}
			
			// We need to generate per-thread offsets (prefix sum) of where to store the keys at
			// (This is equivalent to what was done in count & prefix sum shaders, except that was done per-group)
			
			//// First, count all digits
			uint keyBegin = tileIdx * TILE_SIZE;
			uint keyEnd = keyBegin + tileCount * TILE_SIZE;	
				
			while(keyBegin < keyEnd)
			{
				GroupMemoryBarrierWithGroupSync();
				
				// Zero out local counter
				for(uint i = 0; i < NUM_DIGITS; i++)
					sLocalScratch[i * NUM_THREADS + threadId] = 0;
			
				GroupMemoryBarrierWithGroupSync();
				
				for(uint i = 0; i < KEYS_PER_LOOP; i++)
				{
					uint idx = keyBegin + threadId * KEYS_PER_LOOP + i;
					uint key = gInputKeys[idx];
					uint digit = (key >> gBitOffset) & KEY_MASK;
					
					sLocalScratch[threadId * NUM_DIGITS + digit] += 1;
				}
				
				// Calculate the prefix sum per-digit
				prefixSum(threadId);
				
				// Actually re-order the keys
				uint localOffsets[NUM_DIGITS];
				for(uint i = 0; i < NUM_DIGITS; i++)
					localOffsets[i] = 0;
					
				for(uint i = 0; i < KEYS_PER_LOOP; i++)
				{
					uint idx = keyBegin + threadId * KEYS_PER_LOOP + i;
					uint key = gInputKeys[idx];
					uint digit = (key >> gBitOffset) & KEY_MASK;
					
					uint offset = sGroupOffsets[digit] + sTileTotals[digit] + sLocalScratch[threadId * NUM_DIGITS + digit] + localOffsets[digit];
					localOffsets[digit]++;
					
					// Note: First write to local memory then attempt to coalesce when writing to global?
					gOutputKeys[offset] = key;
					gOutputValues[offset] = gInputValues[idx];
				}
				
				GroupMemoryBarrierWithGroupSync();
				
				if (threadId < NUM_DIGITS)
					sTileTotals[threadId] += sCurrentTileTotal[threadId];
				
				keyBegin += TILE_SIZE;
			}
			
			if(groupId.x == (gNumGroups - 1) && gNumExtraKeys > 0)
			{
				// Zero out local counter
				for(uint i = 0; i < NUM_DIGITS; i++)
					sLocalScratch[i * NUM_THREADS + threadId] = 0;			
			
				GroupMemoryBarrierWithGroupSync();
			
				for(uint i = 0; i < KEYS_PER_LOOP; i++)
				{
					uint localIdx = threadId * KEYS_PER_LOOP + i;
				
					if(localIdx >= gNumExtraKeys)
						continue;
				
					uint idx = keyBegin + localIdx;
					uint key = gInputKeys[idx];
					uint digit = (key >> gBitOffset) & KEY_MASK;
					
					sLocalScratch[threadId * NUM_DIGITS + digit] += 1;
				}
				
				// Calculate the prefix sum per-digit
				prefixSum(threadId);
				
				// Actually re-order the keys
				uint localOffsets[NUM_DIGITS];
				for(uint i = 0; i < NUM_DIGITS; i++)
					localOffsets[i] = 0;
					
				for(uint i = 0; i < KEYS_PER_LOOP; i++)
				{
					uint localIdx = threadId * KEYS_PER_LOOP + i;
				
					if(localIdx >= gNumExtraKeys)
						continue;
				
					uint idx = keyBegin + localIdx;
					uint key = gInputKeys[idx];
					uint digit = (key >> gBitOffset) & KEY_MASK;
					
					uint offset = sGroupOffsets[digit] + sTileTotals[digit] + sLocalScratch[threadId * NUM_DIGITS + digit] + localOffsets[digit];
					localOffsets[digit]++;
					
					// Note: First write to local memory then attempt to coalesce when writing to global?
					gOutputKeys[offset] = key;
					gOutputValues[offset] = gInputValues[idx];
				}
			}
		}
	};
};
