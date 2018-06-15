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
		
		groupshared uint sOffsets[NUM_DIGITS];
		
		void reorderKey(uint idx)
		{
			uint key = gInputKeys[idx];
			uint digit = (key >> gBitOffset) & KEY_MASK;
			
			// Note: Avoid this atomic somehow?
			uint offset;
			InterlockedAdd(sOffsets[digit], 1, offset);
			
			// Note: First write to local memory then attempt to coalesce when writing to global?
			gOutputKeys[offset] = key;
			gOutputValues[offset] = gInputValues[idx];		
		}
		
		[numthreads(NUM_THREADS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			uint threadId = groupThreadId.x;
			
			// Load offsets for this group to local memory
			if(threadId < NUM_DIGITS)
				sOffsets[threadId] = gInputOffsets[groupId.x * NUM_DIGITS + threadId];
			
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
			
			while(keyBegin < keyEnd)
			{
				for(uint i = 0; i < KEYS_PER_LOOP; i++)
				{
					uint idx = keyBegin + threadId * KEYS_PER_LOOP + i;
					reorderKey(idx);
				}
				
				keyBegin += TILE_SIZE;
			}
			
			if(groupId.x == (gNumGroups - 1) && gNumExtraKeys > 0)
			{
				for(uint i = 0; i < KEYS_PER_LOOP; i++)
				{
					if((threadId * KEYS_PER_LOOP + i) >= gNumExtraKeys)
						continue;
				
					uint idx = keyBegin + threadId * KEYS_PER_LOOP + i;
					reorderKey(idx);
				}
			}
		}
	};
};