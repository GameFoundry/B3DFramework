#include "$ENGINE$/RadixSortCommon.bslinc"

shader RadixSortClear
{
	mixin RadixSortCommon;

	code
	{
		RWBuffer<uint> gOutput;
		
		[numthreads(MAX_NUM_GROUPS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID)
		{
			uint threadId = groupThreadId.x;
			
			[unroll]
			for(uint i = 0; i < NUM_DIGITS; i++)
				gOutput[i * MAX_NUM_GROUPS + threadId] = 0.0f;
		}
	};
};
