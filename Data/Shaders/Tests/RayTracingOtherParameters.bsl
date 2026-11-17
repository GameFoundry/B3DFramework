//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader RayTracingOtherParameters
{
	code
	{
		// Declares other parameters than RayTracingPipeline, so their programs can't share a pipeline
		RWStructuredBuffer<uint> Output;

		// Metal has no ray tracing pipelines, and traces rays inline only
#if !defined(METAL)
		[shader("raygeneration")]
		void WriteZero()
		{
			Output[DispatchRaysIndex().x] = 0;
		}
#endif
	};
};
