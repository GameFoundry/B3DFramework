#include "$ENGINE$\VolumeRenderBase.bslinc"
#include "$ENGINE$\PPTonemapCommon.bslinc"
#include "$ENGINE$\PPWhiteBalance.bslinc"

Parameters =
{

};

Technique
 : inherits("VolumeRenderBase")
 : inherits("PPTonemapCommon")
 : inherits("PPWhiteBalance") =
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

Technique
 : inherits("VolumeRenderBase")
 : inherits("PPTonemapCommon")
 : inherits("PPWhiteBalance") =
{
	Language = "GLSL";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
	
		Fragment =
		{
			uniform Input
			{
				// [0]: x - shoulder strength, y - linear strength, z - linear angle, w - toe strength
				// [1]: x - toe numerator, y - toe denominator, z - linear white point, w - unused
				vec4 gTonemapParams[2];
				
				float gGammaAdjustment;
				// 0 - sRGB, 1 - Rec.709, 2 - 2.2 gamma
				uint gGammaCorrectionType;
				
				vec3 gSaturation;
				vec3 gContrast;
				vec3 gGain;
				vec3 gOffset;
			};
		
			/**
			 * Filmic curve used for tonemapping.
			 *
			 * @param 	linearColor		Linear color.
			 * @return					Transformed color.
			 */			
			void FilmicCurve(vec3 color, out vec3 result)
			{
				// Formula from John Hable's Uncharted 2 presentation
				vec3 a = color * (gTonemapParams[0].x * color + vec3(gTonemapParams[0].y * gTonemapParams[0].z));
				vec3 b = vec3(gTonemapParams[0].w * gTonemapParams[1].x);
				vec3 c = color * (gTonemapParams[0].x * color + vec3(gTonemapParams[0].y));
				vec3 d = vec3(gTonemapParams[0].w * gTonemapParams[1].y);
				
				result = (a + b)/(c + d) - vec3(gTonemapParams[1].x / gTonemapParams[1].y);
			}
			
			/**
			 * Applies filmic curve tonemapping to the provided color.
			 *
			 * @param 	linearColor		Linear color in ACEScg color space.
			 * @return					Tonemapped color in ACEScg color space.
			 */		
			void FilmicTonemapping(vec3 color, out vec3 result)
			{
				vec3 filmicColor;
				FilmicCurve(color, filmicColor);
			
				vec3 filmicWhitePoint;
				FilmicCurve(vec3(gTonemapParams[1].z), filmicWhitePoint);
			
				result = filmicColor / filmicWhitePoint;
			}
			
			/**
			 * Applies color grading to the provided color.
			 *
			 * @param 	linearColor		Linear color in ACEScg color space.
			 * @return					Graded color in ACEScg color space.
			 */				
			void ColorGrading(vec3 color, out vec3 result)
			{
				const vec3 RGBToY = vec3(0.2722287168f, 0.6740817658f, 0.0536895174f);
			
				float luminance = dot(color, RGBToY);
				
				color = max(vec3(0.0f), mix(luminance.xxx, color, gSaturation));
				color = pow(color * (1.0f / 0.18f), gContrast) * 0.18f;
				color = color * gGain + gOffset;

				result = color;
			}		
			
			in GStoFS
			{
				vec2 uv0;
				flat uint layerIdx;
			} input;
			
			out vec4 fragColor;
			
			void main()
			{
				// Constants
				const mat3x3 sRGBToACES2065Matrix = XYZToACES2065Matrix * (D65ToD60Matrix * sRGBToXYZMatrix);
				const mat3x3 sRGBToACEScgMatrix = XYZToACEScgMatrix * (D65ToD60Matrix * sRGBToXYZMatrix);
				const mat3x3 ACEScgTosRGBMatrix = XYZTosRGBMatrix * (D60ToD65Matrix * ACEScgToXYZMatrix);
				
				// By default pixel centers will be sampled, but we want to encode the entire range, so
				// offset the sampling by half a pixel, and extend the entire range by one pixel.
				vec2 uv = input.uv0 - (0.5f / LUT_SIZE);
				vec3 logColor = vec3(uv * LUT_SIZE / float(LUT_SIZE - 1), input.layerIdx / float(LUT_SIZE - 1));
				
				vec3 linearColor;
				LogToLinearColor(logColor, linearColor);
				
				WhiteBalance(linearColor, linearColor);
				linearColor = sRGBToACEScgMatrix * linearColor;
				ColorGrading(linearColor, linearColor);
				
				// Note: Improve this so it's closer to the ACES curve?
				FilmicTonemapping(linearColor, linearColor);
				// TODO - Does the white point provided in filmic curve conflict with the white balancing?
				
				linearColor = ACEScgTosRGBMatrix * linearColor;
				
				// Transform to gamma space
				vec3 gammaColor = pow(linearColor, vec3(gGammaAdjustment)); // User adjustment, usually 1.0f
					
				if(gGammaCorrectionType == 0)
					LinearToGammasRGB(gammaColor, gammaColor);
				else if(gGammaCorrectionType == 1)
					LinearToGammaRec709(gammaColor, gammaColor);
				else
					gammaColor = pow(gammaColor, vec3(1.0f/2.2f));
				
				// TODO - Divide by 1.05f here and then re-apply it when decoding from the texture?
				fragColor = vec4(gammaColor, 0.0f);
			}	
		};
	};
};