#include "$ENGINE$/RadixSortCommon.bslinc"

shader RadixSortPrefixScan
{
	mixin RadixSortCommon;

	code
	{
		Buffer<uint> gInputCounts;
		RWBuffer<uint> gOutputOffsets;
		
		groupshared uint sDigitPrefixSum[MAX_NUM_GROUPS * NUM_DIGITS];
		groupshared uint sTotalPrefixSum[NUM_DIGITS];

		[numthreads(MAX_NUM_GROUPS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			uint threadId = groupThreadId.x;
			
			// Load per-group counts into local memory
			for(uint i = 0; i < NUM_DIGITS; i++)
			{
				sDigitPrefixSum[threadId * NUM_DIGITS + i] = gInputCounts[threadId * NUM_DIGITS + i];
			}
			
/* 			// Load per-group counts into local memory
			uint digitIdx = threadId & (NUM_DIGITS - 1);
			for(uint i = 0; i < NUM_DIGITS; i++)
			{
				// Strided loads to avoid bank conflicts:
				// Iteration 0: T0: 0, T1: 1, T2: 2, T3: 3, ..., T31: 31
				// Iteration 1: T0: 16, T1: 17, T2: 18, T15: 31, T16: 32 (0), ..., T31 : 47 (15)
				// ...
				uint idx = i * NUM_DIGITS + digitIdx;
				sDigitPrefixSum[idx] = gInputCounts[idx];
			} */
				
			// Calculate a prefix sum (each group accounting for counts of the
			// groups that comes before it, per digit)
			//// Upsweep to generate partial sums
			uint offset = 1;
			for (uint i = MAX_NUM_GROUPS >> 1; i > 0; i >>= 1)
			{
				GroupMemoryBarrierWithGroupSync();
			
				if (threadId < i)
				{
					// Note: If I run more than MAX_NUM_GROUPS threads I wouldn't have to
					// iterate over all digits in a single thread
					// Note: Perhaps run part of this step serially for better performance
					for (uint j = 0; j < NUM_DIGITS; j++)
					{
						uint idx0 = (offset * (2 * threadId + 1) - 1) * NUM_DIGITS + j;
						uint idx1 = (offset * (2 * threadId + 2) - 1) * NUM_DIGITS + j;

						// Note: Check and remove bank conflicts
						sDigitPrefixSum[idx1] += sDigitPrefixSum[idx0];
					}
				}

				offset <<= 1;
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			//// Last entry now contains the total count, save it and calculate its prefix
			if(threadId < NUM_DIGITS)
			{
				uint idx = (MAX_NUM_GROUPS - 1) * NUM_DIGITS + threadId;
				sTotalPrefixSum[threadId] = sDigitPrefixSum[idx];
				
				// Set tree roots to zero (prepare for downsweep)
				sDigitPrefixSum[idx] = 0;
			}

			//// Downsweep to calculate the prefix sum from partial sums that were generated
			//// during upsweep		
			for (uint i = 1; i < MAX_NUM_GROUPS; i <<= 1)
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
						uint temp = sDigitPrefixSum[idx0];
						sDigitPrefixSum[idx0] = sDigitPrefixSum[idx1];
						sDigitPrefixSum[idx1] += temp;
					}
				}
			}

			// Calculate prefix sum over the total (serially)
			if(threadId == 0)
			{
				for(uint i = 1; i < NUM_DIGITS; i++)
					sTotalPrefixSum[i] += sTotalPrefixSum[i - 1];

				// Make it an exclusive sum by shifting
				for(uint i = NUM_DIGITS - 1; i > 0; i--)
					sTotalPrefixSum[i] = sTotalPrefixSum[i - 1];

				sTotalPrefixSum[0] = 0;
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			// Add the total to per-digit prefix to generate the global offset,
			// and write to global memory
			[unroll]
			for(uint i = 0; i < NUM_DIGITS; i++)
			{
				uint idx = threadId * NUM_DIGITS + i;
				
				// Note: Check and resolve bank conflicts
				gOutputOffsets[idx] = sTotalPrefixSum[i] + sDigitPrefixSum[idx];
			}
		}
	};
};
