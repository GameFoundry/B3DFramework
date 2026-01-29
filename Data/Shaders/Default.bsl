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
			out float4 OutSceneColor : SV_Target0,
			out GBufferData OutGBuffer)
		{
			SurfaceData surfaceData;
			surfaceData.albedo = float4(0.05f, 0.05f, 0.05f, 1.0f);
			surfaceData.worldNormal.xyz = input.tangentToWorldZ;
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			surfaceData.mask = gLayer;
			surfaceData.velocity = 0.0f;
			
			OutSceneColor = 0.0f;
			OutGBuffer = encodeGBuffer(surfaceData);
		}	
	};
};
