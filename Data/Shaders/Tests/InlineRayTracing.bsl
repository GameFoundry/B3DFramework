//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader InlineRayTracing
{
	code
	{
		// Mode 0 traces the closest hit, mode 1 any hit, and mode 2 the closest hit with callbacks that reject every candidate
		struct TestRay
		{
			float4 OriginAndTMin;
			float4 DirectionAndTMax;
			uint4 FlagsMaskMode;
		};

		struct TestHit
		{
			float4 HitDistanceBarycentrics;
			uint4 Indices;
			uint4 FacingAndCallbacks;
		};

		struct RejectAll
		{
			uint CallCount;

			bool AnyHit(RayCandidate candidate)
			{
				CallCount++;
				return false;
			}
		};

		RaytracingAccelerationStructure Scene;
		StructuredBuffer<TestRay> Rays;
		RWStructuredBuffer<TestHit> Hits;

		[numthreads(1, 1, 1)]
		void csmain(uint3 groupId : SV_GroupID)
		{
			TestRay testRay = Rays[groupId.x];

			RayDesc ray;
			ray.Origin = testRay.OriginAndTMin.xyz;
			ray.TMin = testRay.OriginAndTMin.w;
			ray.Direction = testRay.DirectionAndTMax.xyz;
			ray.TMax = testRay.DirectionAndTMax.w;

			uint rayFlags = testRay.FlagsMaskMode.x;
			uint instanceMask = testRay.FlagsMaskMode.y;

			RejectAll rejectAll;
			rejectAll.CallCount = 0;

			RayHit hit;
			if(testRay.FlagsMaskMode.z == 1)
				hit = RayTraceAny(Scene, rayFlags, instanceMask, ray);
			else if(testRay.FlagsMaskMode.z == 2)
				hit = RayTraceClosest(Scene, rayFlags, instanceMask, ray, rejectAll);
			else
				hit = RayTraceClosest(Scene, rayFlags, instanceMask, ray);

			TestHit result;
			result.HitDistanceBarycentrics = float4(hit.IsHit ? 1.0f : 0.0f, hit.T, hit.Barycentrics);
			result.Indices = uint4(hit.InstanceIndex, hit.InstanceID, hit.GeometryIndex, hit.PrimitiveIndex);
			result.FacingAndCallbacks = uint4(hit.FrontFace ? 1 : 0, rejectAll.CallCount, 0, 0);
			Hits[groupId.x] = result;
		}
	};
};
