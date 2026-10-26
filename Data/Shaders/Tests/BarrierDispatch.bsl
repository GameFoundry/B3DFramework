//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader BarrierDispatch
{
	code
	{
		RWStructuredBuffer<uint> OutputBuffer;
		RWTexture2D<uint> OutputTexture;
		[numthreads(16, 16, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			const uint value = 0x5A000000 | (threadId.y * 16 + threadId.x);
			OutputBuffer[threadId.y * 16 + threadId.x] = value;
			OutputTexture[threadId.xy] = value;
		}
	};
};
