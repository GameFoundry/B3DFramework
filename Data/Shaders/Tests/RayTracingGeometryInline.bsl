//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "$ENGINE$\RayTracingGeometryAccess.bslinc"

shader RayTracingGeometryInline
{
	mixin RayTracingGeometryAccess;

	code
	{
		// What a ray reads from the triangle it hits: the interpolated position and 1 for a hit, the interpolated normal and U, then V,
		// and the triangle's vertex indices and the instance id
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

		// Traces ray N from RayOrigins[N] along +Z. Layouts holds the layouts of an instance's geometries from the instance id on.
		[numthreads(1, 1, 1)]
		void csmain(uint3 groupId : SV_GroupID)
		{
			RayDesc ray;
			ray.Origin = RayOrigins[groupId.x].xyz;
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 100.0f;

			RayHit hit = RayTraceClosest(Scene, RAY_FLAG_NONE, 0xFF, ray);

			GeometryHit result;
			result.PositionAndHit = float4(0.0f, 0.0f, 0.0f, 0.0f);
			result.NormalAndU = float4(0.0f, 0.0f, 0.0f, 0.0f);
			result.V = float4(0.0f, 0.0f, 0.0f, 0.0f);
			result.IndicesAndInstanceId = uint4(0, 0, 0, 0);

			if(hit.IsHit)
			{
				RayTracingGeometryLayout layout = Layouts[hit.InstanceID + hit.GeometryIndex];
				uint3 indices = LoadTriangleIndices(layout, hit.PrimitiveIndex);
				float2 uv = InterpolateUv(layout, indices, hit.Barycentrics);

				result.PositionAndHit = float4(InterpolatePosition(layout, indices, hit.Barycentrics), 1.0f);
				result.NormalAndU = float4(InterpolateNormal(layout, indices, hit.Barycentrics), uv.x);
				result.V = float4(uv.y, 0.0f, 0.0f, 0.0f);
				result.IndicesAndInstanceId = uint4(indices, hit.InstanceID);
			}

			Hits[groupId.x] = result;
		}
	};
};
