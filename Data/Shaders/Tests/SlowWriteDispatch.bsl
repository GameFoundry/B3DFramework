//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader SlowWriteDispatch
{
	code
	{
		RWStructuredBuffer<uint> OutputBuffer;
		[numthreads(64, 1, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			uint state = threadId.x;
			for(uint iteration = 0; iteration < 16384; iteration++)
				state = state * 1664525 + 1013904223;

			OutputBuffer[threadId.x * 2] = 0x5A000000 | threadId.x;
			OutputBuffer[threadId.x * 2 + 1] = state;
		}
	};
};
