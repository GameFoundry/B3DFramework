//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader BarrierSample
{
	code
	{
		Texture2D<uint> InputTexture;
		RWStructuredBuffer<uint> OutputBuffer;
		[numthreads(16, 16, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			OutputBuffer[threadId.y * 16 + threadId.x] = InputTexture.Load(int3(threadId.xy, 0));
		}
	};
};
