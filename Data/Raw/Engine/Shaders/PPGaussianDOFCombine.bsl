#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPGaussianDOFCommon.bslinc"

technique PPGaussianDOFCombine
{
	mixin PPBase;
	mixin PPGaussianDOFCommon;
	
	code
	{
		SamplerState gColorSamp;
		Texture2D gFocusedTex;
		Texture2D gNearTex;
		Texture2D gFarTex;
		
		SamplerState gDepthSamp;
		Texture2D gDepthTex;

		float3 fsmain(VStoFS input) : SV_Target0
		{
			float4 focusedColor = gFocusedTex.Sample(gColorSamp, input.uv0);
			float depth = convertFromDeviceZ(gDepthTex.Sample(gDepthSamp, input.uv0));
			
			float4 nearColor = 0;
			float4 farColor = 0;
			
			float bias = 0.00001f; // To avoid div by zero
			#if NEAR
				nearColor = gNearTex.Sample(gColorSamp, input.uv0);
				nearColor.rgb /= (nearColor.a + bias);
			#endif
			#if FAR
				farColor = gFarTex.Sample(gColorSamp, input.uv0);
				farColor.rgb /= (farColor.a + bias);
			#endif
			
			float3 combined;
			
			// Background layer
			combined = farColor.rgb;
			
			// Focused layer
			//// This uses value of background layer as long as the background mask strength is
			//// higher than 0.2f. For strength lower than 0.2f it blends with the focused layer.
			float focusedMask = calcFarMask(depth);
			focusedMask = saturate(1.0f - focusedMask * 5.0f);
			focusedMask *= focusedMask;
			
			combined = lerp(combined, focusedColor.rgb, focusedMask);
			
			// Foreground layer
			//// Same type of blending as with the layer above
			float foregroundMask = calcNearMask(depth);
			foregroundMask = saturate(1.0f - foregroundMask * 5.0f);
			foregroundMask *= foregroundMask;
			
			combined = lerp(combined, nearColor.rgb, foregroundMask);
			
			return combined;
		}	
	};
};