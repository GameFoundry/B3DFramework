//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader IndexedWriteDispatch
{
	code
	{
		cbuffer Parameters { uint Bias; };
		[pushConstant] cbuffer Constants { uint OutputIndex; };
		RWStructuredBuffer<uint> OutputBuffer;

		[numthreads(1, 1, 1)]
		void csmain()
		{
			OutputBuffer[OutputIndex] = Bias + OutputIndex;
		}
	};
};
