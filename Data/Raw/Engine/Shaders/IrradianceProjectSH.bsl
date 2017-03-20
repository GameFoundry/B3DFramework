#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\SHCommon.bslinc"

Technique 
 : inherits("PPBase")
 : inherits("ReflectionCubemapCommon")
 : inherits("SHCommon") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			cbuffer Params
			{
				int gCubeFace;
			}	
		
			StructuredBuffer<SHVector5RGB> gSHCoeffs;

			float evaluateLambert(SHVector5 coeffs)
			{
				// Multiply irradiance SH coefficients by cosine lobe (Lambert diffuse) and evaluate resulting SH
				// See: http://cseweb.ucsd.edu/~ravir/papers/invlamb/josa.pdf for derivation of the
				// cosine lobe factors
				float output = 0.0f;
				
				// Band 0 (factor 1.0)
				output += coeffs.v0[0];
				
				// Band 1 (factor 2/3)
				float f = (2.0f/3.0f);
				float4 f4 = float4(f, f, f, f);
				
				output += dot(coeffs.v0.gba, f4.rgb);
				
                // Band 2 (factor 1/4)
				f = (1.0f/4.0f);
				f4 = float4(f, f, f, f);
				
				output += dot(coeffs.v1, f4);
				output += coeffs.v2.r * f;
				
				// Band 3 (factor 0)
				
				// Band 4 (factor -1/24)
				f = (-1.0f/24.0f);
				f4 = float4(f, f, f, f);
				
				output += dot(coeffs.v4, f4);
				output += dot(coeffs.v5, f4);
				output += coeffs.v6 * f;
				
				return output;
			}
			
			float4 main(VStoFS input) : SV_Target0
			{
				float2 scaledUV = input.uv0 * 2.0f - 1.0f;
				float3 dir = getDirFromCubeFace(gCubeFace, scaledUV);
				
				SHVector5 shBasis = SHBasis5(dir);
								
				SHVector5RGB coeffs = gSHCoeffs[0];
				SHMultiply(coeffs.R, shBasis);
				SHMultiply(coeffs.G, shBasis);
				SHMultiply(coeffs.B, shBasis);
				
				float3 output = 0;
				output.r = evaluateLambert(coeffs.R);
				output.g = evaluateLambert(coeffs.G);
				output.b = evaluateLambert(coeffs.B);
				
				return float4(output.rgb, 1.0f);
			}	
		};
	};
};