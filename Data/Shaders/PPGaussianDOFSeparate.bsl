#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPGaussianDOFCommon.bslinc"

shader PPGaussianDOFSeparate
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
		Texture2D gColorTex;
		
		SamplerState gDepthSamp;
		Texture2D gDepthTex;
		
		void addSample(float2 uv, float2 offset, float depth, inout float4 nearColor, inout float4 farColor)
		{
			float4 smp;
			smp.rgb = gColorTex.SampleLevel(gColorSamp, uv + gHalfPixelOffset * offset, 0.0f).rgb;
			smp.a = 1.0f;
			
			#if NEAR
			nearColor += smp * calcNearMask(depth);
			#endif
			#if FAR
			farColor += smp * calcFarMask(depth);
			#endif		
		}
		
		void fsmain(
			VStoFS input,
			out float4 output0 : SV_Target0
			#if NEAR && FAR
			, out float4 output1 : SV_Target1
			#endif
			)
		{
			float4 depth = -convertFromDeviceZ(gDepthTex.Gather(gDepthSamp, input.uv0));
			
			float4 nearColor = 0;
			float4 farColor = 0;
			
			// Samples start in bottom left and go in counter-clockwise order, in order to match
			// depth Gather
			addSample(input.uv0, float2(-1, 1), depth.x, nearColor, farColor);
			addSample(input.uv0, float2(1, 1), depth.y, nearColor, farColor);
			addSample(input.uv0, float2(1, -1), depth.z, nearColor, farColor);
			addSample(input.uv0, float2(-1, -1), depth.w, nearColor, farColor);
			
			nearColor *= 0.25f;
			farColor *= 0.25f;
			
			#if NEAR && FAR
				output0 = nearColor;
				output1 = farColor;
			#else
			#if NEAR
				output0 = nearColor;
			#else
				output0 = farColor;
			#endif
			#endif
		}	
	};
};
