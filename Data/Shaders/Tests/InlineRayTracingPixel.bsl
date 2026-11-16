//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "$ENGINE$\PPBase.bslinc"

shader InlineRayTracingPixel
{
	mixin PPBase;

	code
	{
		RaytracingAccelerationStructure Scene;

		// Traces a ray along +Z from the pixel's texture coordinates on the XY plane. Red marks a hit, and green a hit at the distance of
		// the test geometry.
		float4 fsmain(VStoFS input) : SV_Target0
		{
			RayDesc ray;
			ray.Origin = float3(input.uv0, 0.0f);
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 100.0f;

			RayHit hit = RayTraceClosest(Scene, 0, 0xFF, ray);

			float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
			if(hit.IsHit)
			{
				color.r = 1.0f;
				if(abs(hit.T - 5.0f) < 0.01f)
					color.g = 1.0f;
			}

			return color;
		}
	};
};
