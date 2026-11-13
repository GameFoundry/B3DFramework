//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader RayTracingPipeline
{
	code
	{
		// RecordSelection holds the TraceRay() arguments that pick the records: the hit group contribution, the geometry multiplier and the
		// miss index
		struct TestRay
		{
			float4 Origin;
			float4 Direction;
			uint4 RecordSelection;
		};

		struct Payload
		{
			uint Value;
		};

		RaytracingAccelerationStructure Scene;
		StructuredBuffer<TestRay> Rays;
		RWStructuredBuffer<uint> Results;

		// TODO(ps5, metal) - Ray tracing pipelines come with phases 5.3 and 5.4
#if !defined(PS5) && !defined(METAL)
		// Writes the value of the program that ran for each ray, or 1 if none did
		[shader("raygeneration")]
		void TraceTestRays()
		{
			uint rayIndex = DispatchRaysIndex().x;
			TestRay testRay = Rays[rayIndex];

			RayDesc ray;
			ray.Origin = testRay.Origin.xyz;
			ray.TMin = 0.0f;
			ray.Direction = testRay.Direction.xyz;
			ray.TMax = 100.0f;

			Payload payload;
			payload.Value = 1;

			TraceRay(Scene, RAY_FLAG_NONE, 0xFF, testRay.RecordSelection.x, testRay.RecordSelection.y, testRay.RecordSelection.z, ray, payload);
			Results[rayIndex] = payload.Value;
		}

		// Traces nothing, so the results show which ray generation record ran
		[shader("raygeneration")]
		void WriteIndices()
		{
			uint rayIndex = DispatchRaysIndex().x;
			Results[rayIndex] = 1000 + rayIndex;
		}

		[shader("miss")]
		void MissA(inout Payload payload)
		{
			payload.Value = 100;
		}

		[shader("miss")]
		void MissB(inout Payload payload)
		{
			payload.Value = 101;
		}

		[shader("closesthit")]
		void ClosestHitA(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			payload.Value = 10;
		}

		[shader("closesthit")]
		void ClosestHitB(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			payload.Value = 11;
		}

		[shader("closesthit")]
		void ClosestHitC(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			payload.Value = 12;
		}
#endif
	};
};
