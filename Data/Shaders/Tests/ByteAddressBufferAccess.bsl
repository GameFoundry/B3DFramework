//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader ByteAddressBufferAccess
{
	code
	{
		ByteAddressBuffer Input;
		RWByteAddressBuffer Output;

		[numthreads(4, 1, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			// Each thread copies one word of the input, and adds to the shared counters
			Output.Store(threadId.x * 4, Input.Load(threadId.x * 4) + 1);
			Output.InterlockedAdd(16, 1);
			Output.InterlockedMax(32, threadId.x + 10);

			if(threadId.x == 0)
			{
				uint inputSize;
				Input.GetDimensions(inputSize);

				Output.Store2(20, Input.Load2(4));
				Output.Store(28, inputSize);
			}
		}
	};
};
