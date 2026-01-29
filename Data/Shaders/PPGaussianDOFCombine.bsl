#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPGaussianDOFCommon.bslinc"

shader PPGaussianDOFCombine
{
	mixin PPBase;
	mixin PPGaussianDOFCommon;
	
	variations
	{
		NEAR = { true, false };
		FAR = { true, false };
	};
	
	code
	{
		SamplerState gColorSamp;
		Texture2D gFocusedTex;
		Texture2D gNearTex;
		Texture2D gFarTex;
		
		SamplerState gDepthSamp;
		Texture2D gDepthTex;

		float4 fsmain(VStoFS input) : SV_Target0
		{
			float4 focusedColor = gFocusedTex.Sample(gColorSamp, input.uv0);
			float depth = -convertFromDeviceZ(gDepthTex.SampleLevel(gDepthSamp, input.uv0, 0));
			
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
			foregroundMask = saturate(foregroundMask * 5.0f);
			foregroundMask *= foregroundMask;
			
			combined = lerp(combined, nearColor.rgb, foregroundMask);
			
			// Alpha channel contains luma required for FXAA
			return float4(combined, focusedColor.a);
		}	
	};
};
