#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader MSAACoverage
{
	mixin PPBase;
	mixin GBufferInput;
	mixin PerCameraData;

	variations
	{
		MSAA_COUNT = { 2, 4, 8 };
	};	
	
	code
	{	
		float fsmain(VStoFS input) : SV_Target0
		{
			SurfaceData surfaceData[MSAA_COUNT];

			[unroll]
			for(uint i = 0; i < MSAA_COUNT; ++i)
				surfaceData[i] = getGBufferData((int2)input.uv0, i);

			float3 albedo = surfaceData[0].albedo.xyz;
			float3 normal = surfaceData[0].worldNormal.xyz;
			float depth = surfaceData[0].depth;

			[unroll]
			for(int i = 1; i < MSAA_COUNT; i++)
			{
				float3 otherAlbedo = surfaceData[i].albedo.xyz;
				float3 otherNormal = surfaceData[i].worldNormal.xyz;
				float otherDepth = surfaceData[i].depth;

				[branch]
				if((abs(depth - otherDepth) > 0.01f) || 
				   (dot(normal, otherNormal) < 0.99f) || 
				   (abs(dot(albedo - otherAlbedo, float3(1, 1, 1))) > 0.01f))
				{
					return 1.0f;
				}
			}
			
			return 0.0f;			
		}	
	};
};
