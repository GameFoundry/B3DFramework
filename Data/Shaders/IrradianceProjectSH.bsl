#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#define SH_ORDER 5
#include "$ENGINE$\SHCommon.bslinc"

shader IrradianceProjectSH
{
	mixin PPBase;
	mixin ReflectionCubemapCommon;
	mixin SHCommon;

	code
	{
		[internal]
		cbuffer Params
		{
			int gCubeFace;
		}	
	
		Texture2D gSHCoeffs;

		float evaluateLambert(SHVector coeffs)
		{
			// Multiply irradiance SH coefficients by cosine lobe (Lambert diffuse) and evaluate resulting SH
			// See: http://cseweb.ucsd.edu/~ravir/papers/invlamb/josa.pdf for derivation of the
			// cosine lobe factors
			float output = 0.0f;
			
			// Band 0 (factor 1.0)
			output += coeffs.v[0];
			
			// Band 1 (factor 2/3)
			float f = (2.0f/3.0f);
			for(int i = 1; i < 4; i++)
				output += coeffs.v[i] * f;
			
			// Band 2 (factor 1/4)
			f = (1.0f/4.0f);
			for(int i = 4; i < 9; i++)
				output += coeffs.v[i] * f;
						
			// Band 3 (factor 0)
			
			// Band 4 (factor -1/24)
			f = (-1.0f/24.0f);
			for(int i = 16; i < 25; i++)
				output += coeffs.v[i] * f;
			
			return output;
		}
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float2 scaledUV = input.uv0 * 2.0f - 1.0f;
			float3 dir = getDirFromCubeFace(gCubeFace, scaledUV);
			dir = normalize(dir);
			
			SHVector shBasis = SHBasis(dir);
							
			SHVectorRGB coeffs = SHLoad(gSHCoeffs, int2(0, 0));
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
