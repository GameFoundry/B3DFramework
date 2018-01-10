#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#define SH_ORDER 3
#include "$ENGINE$\SHCommon.bslinc"
#include "$ENGINE$\PPBase.bslinc"

technique IrradianceComputeSHFrag
{
	mixin PPBase;
	mixin ReflectionCubemapCommon;
	mixin SHCommon;

	code
	{
		SamplerState gInputSamp;
		TextureCube gInputTex;
	
		[internal]
		cbuffer Params
		{
			uint gCubeFace;
			uint gFaceSize;
			uint gCoeffIdx;
		}			
				
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// Move to [-1,1] range
			float2 uv = input.uv0 * 2.0f - 1.0f;
			
			float3 dir = getDirFromCubeFace(gCubeFace, float2(uv.x, uv.y));
			dir = normalize(dir);
			
			// Need to calculate solid angle (weight) of the texel, as cube face corners have
			// much smaller solid angle, meaning many of them occupy the same area when projected
			// on a sphere. Without weighing that area would look too bright.
			float invFaceSize = 1.0f / gFaceSize;
			float weight = texelSolidAngle(uv.x, uv.y, invFaceSize);
			
			SHVector shBasis = SHBasis(dir);
			float3 radiance = gInputTex.SampleLevel(gInputSamp, dir, 0).rgb;
												
			float4 output;
			output.r = shBasis.v[gCoeffIdx] * radiance.r * weight;
			output.g = shBasis.v[gCoeffIdx] * radiance.g * weight;
			output.b = shBasis.v[gCoeffIdx] * radiance.b * weight;
			output.a = weight;
			
			return output;
		}
	};
};