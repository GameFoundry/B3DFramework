#include "$ENGINE$/RadixSortCommon.bslinc"

shader RadixSortCount
{
	mixin RadixSortCommon;

	code
	{
		#define NUM_COUNTERS (NUM_THREADS * NUM_DIGITS)
		#define NUM_REDUCE_THREADS 64
		#define NUM_REDUCE_THREADS_PER_DIGIT (NUM_REDUCE_THREADS/NUM_DIGITS)
		#define NUM_REDUCE_ELEMS_PER_THREAD_PER_DIGIT (NUM_THREADS/NUM_REDUCE_THREADS_PER_DIGIT)
	
		Buffer<uint> gInputKeys;
		RWBuffer<uint> gOutputCounts;
		
		groupshared uint sCounters[NUM_COUNTERS];
		groupshared uint sReduceCounters[NUM_REDUCE_THREADS];
		
		[numthreads(NUM_THREADS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			uint threadId = groupThreadId.x;
		
			// Initialize counters to zero
			for(uint i = 0; i < NUM_DIGITS; i++)
				sCounters[threadId * NUM_DIGITS + i] = 0;
				
			if(threadId < NUM_REDUCE_THREADS)
				sReduceCounters[threadId] = 0;
				
			GroupMemoryBarrierWithGroupSync();
			
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
			
			uint keyBegin = tileIdx * TILE_SIZE;
			uint keyEnd = keyBegin + tileCount * TILE_SIZE;	
			
			// For each key determine its digits and count how many digits of each type
			// there are. We shift and mask the key using a radix in order to only handle
			// M digits at a time. Multiple passes are therefore required to fully sort
			// the solution.
			while(keyBegin < keyEnd)
			{
				uint key = gInputKeys[keyBegin + threadId];
				uint digit = (key >> gBitOffset) & KEY_MASK;
				
				sCounters[threadId * NUM_DIGITS + digit] += 1;
				
				keyBegin += NUM_THREADS;
			}
			
			// Unless the number of keys is an exact multiple of the number of tiles, there will
			// be an extra set of keys that require per-threading checking in case we go out of
			// range. We handle this as a special case for the last group, to avoid paying the
			// cost of the check for every key.
			if(groupId.x == (gNumGroups - 1))
			{
				keyBegin = keyEnd;
				keyEnd = keyBegin + gNumExtraKeys;
				
				while(keyBegin < keyEnd)
				{
					if((keyBegin + threadId) < keyEnd)
					{
						uint key = gInputKeys[keyBegin + threadId];
						uint digit = (key >> gBitOffset) & KEY_MASK;
						
						sCounters[threadId * NUM_DIGITS + digit] += 1;
					}
					
					keyBegin += NUM_THREADS;
				}
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			// Reduce the counts for all threads in a group into a single NUM_DIGITS array
			if(threadId < NUM_REDUCE_THREADS)
			{
				uint digitIdx = threadId / NUM_REDUCE_THREADS_PER_DIGIT;
				uint setIdx = threadId & (NUM_REDUCE_THREADS_PER_DIGIT - 1);
				
				// First do the sum sequentially to a certain extent (shown to be faster
				// than doing it fully parallel). In the end we end up with
				// NUM_REDUCE_THREADS_PER_DIGIT sets of digits
				uint total = 0;
				for(uint i = 0; i < NUM_REDUCE_ELEMS_PER_THREAD_PER_DIGIT; i++)
				{
					// Note: Check & reduce bank conflicts
					uint threadIdx = (setIdx * NUM_REDUCE_ELEMS_PER_THREAD_PER_DIGIT + i) * NUM_DIGITS;
					total += sCounters[threadIdx + digitIdx];
				}
				
				sReduceCounters[digitIdx * NUM_REDUCE_THREADS_PER_DIGIT + setIdx] = total;
				
				// And do parallel reduction on the result of serial additions
				[unroll]
				for(uint i = 1; i < NUM_REDUCE_THREADS_PER_DIGIT; i <<= 1)
				{
					// Not using sync because operations at this point should happen in the same warp
					WarpGroupMemoryBarrier();
				
					// Note: Check & reduce bank conflicts
					total += sReduceCounters[digitIdx * NUM_REDUCE_THREADS_PER_DIGIT + setIdx + i];
					sReduceCounters[digitIdx * NUM_REDUCE_THREADS_PER_DIGIT + setIdx] = total;
				} 
			}
				
			GroupMemoryBarrierWithGroupSync();

			// Write the summed up per-digit counts to global memory
			if(threadId < NUM_DIGITS)
			{
				gOutputCounts[groupId.x * NUM_DIGITS + threadId] = sReduceCounters[threadId * NUM_REDUCE_THREADS_PER_DIGIT];
			}
		}
	};
};
