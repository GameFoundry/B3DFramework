//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader InlineRayTracingBindless
{
	code
	{
		[pushConstant] cbuffer Indices
		{
			uint SceneIndex;
		};

		RWStructuredBuffer<float4> Hit;

		[numthreads(1, 1, 1)]
		void csmain()
		{
			RaytracingAccelerationStructure scene = ResourceDescriptorHeap[SceneIndex];

			RayDesc ray;
			ray.Origin = float3(0.25f, 10.25f, 0.0f);
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 100.0f;

			RayHit hit = RayTraceClosest(scene, RAY_FLAG_NONE, 0xFF, ray);
			Hit[0] = float4(hit.IsHit ? 1.0f : 0.0f, hit.T, (float)hit.InstanceID, (float)hit.InstanceIndex);
		}
	};
};
