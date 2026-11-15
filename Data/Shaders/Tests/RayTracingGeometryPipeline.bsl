//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "$ENGINE$\RayTracingGeometryAccess.bslinc"

shader RayTracingGeometryPipeline
{
	mixin RayTracingGeometryAccess;

	code
	{
		// Layout of RayTracingGeometryInline's GeometryHit
		struct GeometryHit
		{
			float4 PositionAndHit;
			float4 NormalAndU;
			float4 V;
			uint4 IndicesAndInstanceId;
		};

		RaytracingAccelerationStructure Scene;
		StructuredBuffer<float4> RayOrigins;
		StructuredBuffer<RayTracingGeometryLayout> Layouts;
		RWStructuredBuffer<GeometryHit> Hits;

		// TODO(ps5, metal) - Ray tracing pipelines come with phases 5.3 and 5.4
#if !defined(PS5) && !defined(METAL)
		// Traces ray N from RayOrigins[N] along +Z, and writes what the closest hit program read into Hits[N]
		[shader("raygeneration")]
		void TraceGeometryRays()
		{
			uint rayIndex = DispatchRaysIndex().x;

			RayDesc ray;
			ray.Origin = RayOrigins[rayIndex].xyz;
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 100.0f;

			GeometryHit payload;
			payload.PositionAndHit = float4(0.0f, 0.0f, 0.0f, 0.0f);
			payload.NormalAndU = float4(0.0f, 0.0f, 0.0f, 0.0f);
			payload.V = float4(0.0f, 0.0f, 0.0f, 0.0f);
			payload.IndicesAndInstanceId = uint4(0, 0, 0, 0);

			TraceRay(Scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
			Hits[rayIndex] = payload;
		}

		[shader("miss")]
		void MissGeometry(inout GeometryHit payload)
		{
		}

		// Layouts holds the layouts of an instance's geometries from the instance id on
		[shader("closesthit")]
		void ReadGeometry(inout GeometryHit payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			RayTracingGeometryLayout layout = Layouts[InstanceID() + GeometryIndex()];
			uint3 indices = LoadTriangleIndices(layout, PrimitiveIndex());
			float2 uv = InterpolateUv(layout, indices, attributes.barycentrics);

			payload.PositionAndHit = float4(InterpolatePosition(layout, indices, attributes.barycentrics), 1.0f);
			payload.NormalAndU = float4(InterpolateNormal(layout, indices, attributes.barycentrics), uv.x);
			payload.V = float4(uv.y, 0.0f, 0.0f, 0.0f);
			payload.IndicesAndInstanceId = uint4(indices, InstanceID());
		}
#endif
	};
};
