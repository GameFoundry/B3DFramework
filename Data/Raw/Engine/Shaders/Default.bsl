#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\Surface.bslinc"

Technique : base("Surface") =
{
	Pass =
	{
		Fragment =
		{
			void main(
				in VStoFS input, 
				out float4 OutGBufferA : SV_Target0,
				out float4 OutGBufferB : SV_Target1,
				out float2 OutGBufferC : SV_Target2)
			{
				SurfaceData surfaceData;
				surfaceData.albedo = float4(0.05f, 0.05f, 0.05f, 1.0f);
				surfaceData.worldNormal.xyz = input.tangentToWorldZ;
				surfaceData.roughness = 1.0f;
				surfaceData.metalness = 0.0f;
				
				encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC);
			}	
		};
	};
};