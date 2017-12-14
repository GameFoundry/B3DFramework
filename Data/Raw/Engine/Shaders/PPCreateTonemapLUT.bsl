#include "$ENGINE$\PPTonemapCommon.bslinc"
#include "$ENGINE$\PPWhiteBalance.bslinc"
#include "$ENGINE$\PPBase.bslinc"

technique PPCreateTonemapLUT
{
	mixin PPTonemapCommon;
	mixin PPWhiteBalance;
	
	#if VOLUME_LUT
	#else
	mixin PPBase;
	#endif
	
	code
	{
		[internal]
		cbuffer Input
		{
			// [0]: x - shoulder strength, y - linear strength, z - linear angle, w - toe strength
			// [1]: x - toe numerator, y - toe denominator, z - linear white point, w - unused
			float4 gTonemapParams[2];
			
			float gGammaAdjustment;
			// 0 - sRGB, 1 - Rec.709, 2 - 2.2 gamma
			uint gGammaCorrectionType;
			
			float3 gSaturation;
			float3 gContrast;
			float3 gGain;
			float3 gOffset;
		};
	
		/**
		 * Filmic curve used for tonemapping.
		 *
		 * @param 	linearColor		Linear color.
		 * @return					Transformed color.
		 */			
		float3 FilmicCurve(float3 color)
		{
			// Formula from John Hable's Uncharted 2 presentation
			float3 a = color * (gTonemapParams[0].x * color + gTonemapParams[0].y * gTonemapParams[0].z);
			float b = gTonemapParams[0].w * gTonemapParams[1].x;
			float3 c = color * (gTonemapParams[0].x * color + gTonemapParams[0].y);
			float d = gTonemapParams[0].w * gTonemapParams[1].y;
			
			return (a + b)/(c + d) - gTonemapParams[1].x / gTonemapParams[1].y;
		}
		
		/**
		 * Applies filmic curve tonemapping to the provided color.
		 *
		 * @param 	linearColor		Linear color in ACEScg color space.
		 * @return					Tonemapped color in ACEScg color space.
		 */		
		float3 FilmicTonemapping(float3 color)
		{
			return FilmicCurve(color) / FilmicCurve(gTonemapParams[1].z);
		}
		
		/**
		 * Applies color grading to the provided color.
		 *
		 * @param 	linearColor		Linear color in ACEScg color space.
		 * @return					Graded color in ACEScg color space.
		 */				
		float3 ColorGrading(float3 color)
		{
			const float3 RGBToY = float3(0.2722287168f, 0.6740817658f, 0.0536895174f);
		
			float luminance = dot(color, RGBToY);
			
			color = max(0, lerp(luminance.xxx, color, gSaturation));
			color = pow(color * (1.0f / 0.18f), gContrast) * 0.18f;
			color = color * gGain + gOffset;

			return color;
		}
		
		/** Generates tonemap information to the specified color (in log encoded space). */
		float3 tonemapColor(float3 logColor)
		{
			// Constants
			const float3x3 sRGBToACES2065Matrix = mul(XYZToACES2065Matrix, mul(D65ToD60Matrix, sRGBToXYZMatrix));
			const float3x3 sRGBToACEScgMatrix = mul(XYZToACEScgMatrix, mul(D65ToD60Matrix, sRGBToXYZMatrix));
			const float3x3 ACEScgTosRGBMatrix = mul(XYZTosRGBMatrix, mul(D60ToD65Matrix, ACEScgToXYZMatrix));		
		
			float3 linearColor = LogToLinearColor(logColor);
			
			linearColor = WhiteBalance(linearColor);
			linearColor = mul(sRGBToACEScgMatrix, linearColor);
			linearColor = ColorGrading(linearColor);
			
			// Note: Improve this so it's closer to the ACES curve?
			linearColor = FilmicTonemapping(linearColor);
			// TODO - Does the white point provided in filmic curve conflict with the white balancing?
			
			linearColor = mul(ACEScgTosRGBMatrix, linearColor);
			
			// Transform to gamma space
			float3 gammaColor = pow(linearColor, gGammaAdjustment); // User adjustment, usually 1.0f
				
			if(gGammaCorrectionType == 0)
				gammaColor = LinearToGammasRGB(gammaColor);
			else if(gGammaCorrectionType == 1)
				gammaColor = LinearToGammaRec709(gammaColor);
			else
				gammaColor = pow(gammaColor, 1.0f/2.2f);
				
			return gammaColor;
		}
		
		#if VOLUME_LUT
		RWTexture3D<unorm float4> gOutputTex;
		
		[numthreads(8, 8, 1)]
		void csmain(
			uint3 dispatchThreadId : SV_DispatchThreadID,
			uint threadIndex : SV_GroupIndex)
		{
			float3 logColor = float3(dispatchThreadId.xyz / (float)(LUT_SIZE - 1));
			float3 gammaColor = tonemapColor(logColor);
							
			// TODO - Divide by 1.05f here and then re-apply it when decoding from the texture?
			gOutputTex[dispatchThreadId] = float4(gammaColor, 1.0f);	
		}
		#else
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float3 logColor;
			
			float2 uv = input.uv0;
			// Make sure uv maps to [0,1], as it's currently specified for pixel centers
			// (This way we get non-extrapolated color values for 0 and 1)
			uv -= float2(0.5f / (LUT_SIZE * LUT_SIZE), 0.5f / LUT_SIZE);
			uv *= LUT_SIZE / (LUT_SIZE - 1);
			
			// Red goes from 0 to 1, in each slice along X (LUT_SIZE number of slices)
			logColor.r = frac(uv.x * LUT_SIZE);
			
			// Green value is constant within each slice, and increases by 1/LUT_SIZE with each slice along X
			logColor.g = uv.x - logColor.r / LUT_SIZE;
			
			// Blue increases linearly with y
			logColor.b = uv.y;
			
			float3 gammaColor = tonemapColor(logColor);
							
			// TODO - Divide by 1.05f here and then re-apply it when decoding from the texture?
			return float4(gammaColor, 1.0f);	
		}	
		#endif
	};
};