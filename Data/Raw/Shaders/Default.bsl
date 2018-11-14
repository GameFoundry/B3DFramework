#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\GBufferOutput.bslinc"

shader Surface
{
	mixin BasePass;
	mixin GBufferOutput;

	code
	{
		void fsmain(
			in VStoFS input, 
			out float3 OutSceneColor : SV_Target0,
			out float4 OutGBufferA : SV_Target1,
			out float4 OutGBufferB : SV_Target2,
			out float2 OutGBufferC : SV_Target3,
			out uint OutGBufferD : SV_Target4)
		{
			SurfaceData surfaceData;
			surfaceData.albedo = float4(0.05f, 0.05f, 0.05f, 1.0f);
			surfaceData.worldNormal.xyz = input.tangentToWorldZ;
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			surfaceData.mask = gLayer;
			
			encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC, OutGBufferD);
			
			OutSceneColor = 0.0f;
		}	
	};
};