#include "$ENGINE$\DepthOfFieldCommon.bslinc"
#if MSAA_MODE != 0
	#define MSAA_COUNT 2
#endif
#include "$ENGINE$\DepthInput.bslinc"
#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader PPBokehDOF
{
	mixin DepthOfFieldCommon;
	mixin PPBase;
	mixin DepthInput;
	mixin PerCameraData;

	variations
	{
		// 0 - None
		// 1 - Resolve single sample only
		// 2 - Resolve all samples
		MSAA_MODE = { 0, 1, 2 };
	};
	
	code
	{
		cbuffer Params
		{
			float2 gLayerUVScaleOffset;
			float2 gFocusedImageSize;
		};
		
		Texture2D gUnfocusedTex;
		SamplerState gUnfocusedSampler
		{
			AddressU = CLAMP;
			AddressV = CLAMP;
			AddressW = CLAMP;
		};
		
		#if MSAA_MODE == 0
		Texture2D gFocusedTex;
		SamplerState gFocusedSampler;
		#else
		Texture2DMS<float4> gFocusedTex;
		#endif
			
		float4 fsmain(VStoFS input
			#if MSAA_MODE == 2
			,uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{
			float2 uv = input.uv0;
			float2 screenPos = uv * gFocusedImageSize;
			
			#if MSAA_MODE == 0
				float deviceZ = gDepthBufferTex.SampleLevel(gDepthBufferSamp, uv, 0).r;
				float3 focusedColor = gFocusedTex.Sample(gFocusedSampler, uv).rgb;
			#elif MSAA_MODE == 1
				float deviceZ = gDepthBufferTex.Load(screenPos, 0).r;
				float3 focusedColor = gFocusedTex.Load(screenPos, 0).rgb;
			#else
				float deviceZ = gDepthBufferTex.Load(screenPos, sampleIdx).r;
				float3 focusedColor = gFocusedTex.Load(screenPos, sampleIdx).rgb;
			#endif
			
			float depth = -convertFromDeviceZ(deviceZ);
			
			float2 halfUV = uv;
			halfUV.y = halfUV.y * gLayerUVScaleOffset.x;
			
			float4 blurredFocusedAndFar = gUnfocusedTex.Sample(gUnfocusedSampler, halfUV);
			float4 blurredNear = gUnfocusedTex.Sample(gUnfocusedSampler, halfUV + float2(0, gLayerUVScaleOffset.y));
			
			float2 layer = computeLayerContributions(depth);
			
			float farMask = layer.g;
			float focusedMask = 1.0f - saturate(circleOfConfusionPhysical(depth));
			float nearMask = layer.r;

			float4 combined = 0;

			// Blend half-res far layer
			combined.rgb = lerp(focusedColor, blurredFocusedAndFar.rgb / max(blurredFocusedAndFar.a, 0.001f), farMask);

			// Blend full-res in-focus area
			combined = lerp(combined, float4(focusedColor, 1), focusedMask);

			// Blend half-res near layer
			combined = lerp(combined, float4(blurredNear.rgb / max(blurredNear.a, 0.001f), 0), nearMask);

			float weight = combined.a;
			return float4(lerp(combined.rgb, focusedColor.rgb, weight), 1.0f);
		}	
	};
};