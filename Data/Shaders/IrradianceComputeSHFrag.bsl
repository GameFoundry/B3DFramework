#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#define SH_ORDER 3
#include "$ENGINE$\SHCommon.bslinc"
#include "$ENGINE$\PPBase.bslinc"

shader IrradianceComputeSHFrag
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
			uint gCoeffEntryIdx;
			uint gCoeffComponentIdx;
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
			
			SHVector b = SHBasis(dir);

			// Copy to array of float4’s because of a driver bug on macOS that causes
			// non-const indexing of an array of floats to return incorrect values
			float4 v[3];
			v[0] = float4(b.v[0], b.v[1], b.v[2], b.v[3]);
			v[1] = float4(b.v[4], b.v[5], b.v[6], b.v[7]);
			v[2] = float4(b.v[8], 0.0f, 0.0f, 0.0f);

			float3 radiance = gInputTex.SampleLevel(gInputSamp, dir, 0).rgb;
			float shCoeff = v[gCoeffEntryIdx][gCoeffComponentIdx];			
		
			float4 output;
			output.r = shCoeff * radiance.r * weight;
			output.g = shCoeff * radiance.g * weight;
			output.b = shCoeff * radiance.b * weight;
			output.a = weight;

			return output;
		}
	};
};
