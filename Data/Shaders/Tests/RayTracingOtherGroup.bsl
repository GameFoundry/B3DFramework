//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader RayTracingOtherGroup
{
	code
	{
		// Declares the parameters of RayTracingPipeline, but its programs compile in another group, so they can't share a pipeline
		struct TestRay
		{
			float4 Origin;
			float4 Direction;
			uint4 RecordSelection;
		};

		RaytracingAccelerationStructure Scene;
		StructuredBuffer<TestRay> Rays;
		RWStructuredBuffer<uint> Results;

		// Metal has no ray tracing pipelines, and traces rays inline only
#if !defined(METAL)
		[shader("raygeneration")]
		void WriteOtherIndices()
		{
			uint rayIndex = DispatchRaysIndex().x;
			Results[rayIndex] = 2000 + rayIndex;
		}
#endif
	};
};
