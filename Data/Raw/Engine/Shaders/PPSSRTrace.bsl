#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\RayMarch.bslinc"

technique PPSSRTrace
{
	mixin PPBase;
	mixin PerCameraData;
	mixin GBufferInput;
	mixin RayMarch;

	stencil
	{
		enabled = true;
		reference = 0;
		front = { keep, keep, keep, eq };
	};	
	
	code
	{
		[internal]
		cbuffer Input
		{
			float4 gHiZUVMapping;
			int2 gHiZSize;
			int gHiZNumMips;
		}
		
		Texture2D gSceneColor;
		SamplerState gSceneColorSamp;

		float4 fsmain(VStoFS input, float4 pixelPos : SV_Position) : SV_Target0
		{
			// TODO - Support MSAA?
		
			SurfaceData surfData = getGBufferData(input.uv0);
			float3 P = NDCToWorld(input.screenPos, surfData.depth);
			float3 V = normalize(P - gViewOrigin);
			float3 N = surfData.worldNormal.xzy;
			
			// TODO - Allow number of steps and rays be customized using a quality level
			//  - And HiZ vs linear search
			
			// TODO - Use Hammersley + random to generate ray directions based on GGX BRDF
			//  - Clip BRDF lobe? And renormalize PDF?
			// TODO - Generate random ray step offset
			// TODO - Reject rays pointing under the surface
			float3 R = reflect(-V, N);
			
			RayMarchParams rayMarchParams;
			rayMarchParams.bufferSize = gHiZSize;
			rayMarchParams.numMips = gHiZNumMips;
			rayMarchParams.hiZUVMapping = gHiZUVMapping;
			rayMarchParams.rayOrigin = P;
			rayMarchParams.rayDir = R;
			rayMarchParams.rayLength = -surfData.depth; // Arbitrary since I resize the ray anyway?
			rayMarchParams.jitterOffset = 0.0f;
			
			// TODO - Fade based on roughness
			
			float4 rayHit = rayMarch(gDepthBufferTex, gDepthBufferSamp, rayMarchParams);
			if(rayHit.w < 1.0f) // Hit
				return gSceneColor.Sample(gSceneColorSamp, rayHit.xy);

			return 0.0f;
		}	
	};
};