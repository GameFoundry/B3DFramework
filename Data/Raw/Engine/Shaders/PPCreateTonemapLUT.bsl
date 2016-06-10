#include "$ENGINE$\VolumeRenderBase.bslinc"
#include "$ENGINE$\PPTonemapCommon.bslinc"
#include "$ENGINE$\PPWhiteBalance.bslinc"

Parameters =
{

};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
	
		Fragment =
		{
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
			
			float4 main(GStoFS input) : SV_Target0
			{
				// Constants
				const float3x3 sRGBToACES2065Matrix = mul(XYZToACES2065Matrix, mul(D65ToD60Matrix, sRGBToXYZMatrix));
				const float3x3 sRGBToACEScgMatrix = mul(XYZToACEScgMatrix, mul(D65ToD60Matrix, sRGBToXYZMatrix));
				const float3x3 ACEScgTosRGBMatrix = mul(XYZTosRGBMatrix, mul(D60ToD65Matrix, ACEScgToXYZMatrix));
				
				// By default pixel centers will be sampled, but we want to encode the entire range, so
				// offset the sampling by half a pixel, and extend the entire range by one pixel.
				float2 uv = input.uv0 - (0.5f / LUT_SIZE);
				float3 logColor = float3(uv * LUT_SIZE / (float)(LUT_SIZE - 1), input.layerIdx / (float)(LUT_SIZE - 1));
				
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
				
				// TODO - Divide by 1.05f here and then re-apply it when decoding from the texture?
				return float4(gammaColor, 0.0f);
			}	
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			// TODO
		};
	};
};