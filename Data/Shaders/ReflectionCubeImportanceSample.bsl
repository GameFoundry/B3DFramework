#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\ImportanceSampling.bslinc"

shader ReflectionCubeImportanceSample
{
	mixin PPBase;
	mixin ReflectionCubemapCommon;
	mixin ImportanceSampling;

	code
	{
		cbuffer Input
		{
			int gCubeFace;
			int gMipLevel;
			int gNumMips;
			float gPrecomputedMipFactor;
		}	
	
		SamplerState gInputSamp;
		TextureCube gInputTex;

		float4 fsmain(VStoFS input) : SV_Target0
		{
			float2 scaledUV = input.uv0 * 2.0f - 1.0f;
									
			float3 N = getDirFromCubeFace(gCubeFace, scaledUV);
			N = normalize(N);
			
			// Determine which mip level to sample from depending on PDF and cube -> sphere mapping distortion
			float distortion = rcp(pow(N.x * N.x + N.y * N.y + N.z * N.z, 3.0f/2.0f));
			
			float roughness = mapMipLevelToRoughness(gMipLevel, gNumMips);
			float roughness2 = roughness * roughness;
			float roughness4 = roughness2 * roughness2;
			
			float4 sum = 0;
			for(uint i = 0; i < NUM_SAMPLES; i++)
			{
				float2 random = hammersleySequence(i, NUM_SAMPLES);
				float2 sphericalH = importanceSampleGGX(random, roughness4);
				
				float cosTheta = sphericalH.x;
				float phi = sphericalH.y;
				
				float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
				
				float3 H = sphericalToCartesian(cosTheta, sinTheta, phi);
				float PDF = pdfGGX(cosTheta, sinTheta, roughness4);
				
				// Transform H to world space
				float3 up = abs(H.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
				float3 tangentX = normalize(cross(up, N));
				float3 tangentY = cross(N, tangentX);
				
				H = tangentX * H.x + tangentY * H.y + N * H.z; 
				
				// Calculating mip level from distortion and pdf and described by http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
				int mipLevel = max(gPrecomputedMipFactor - 0.5f * log2(PDF * distortion), 0);
				
				// Note: Adding +1 bias as it looks better
				mipLevel++;
				
				// We need a light direction to properly evaluate the NoL term of the evaluation integral
				//  Li(u) * brdf(u, v) * (u.n) / pdf(u, v)
				// which we don't have, so we assume a viewing direction is equal to normal and calculate lighting dir from it and half-vector
				float3 L = 2 * dot(N, H) * H - N;
				float NoL = saturate(dot(N, L));
				
				// sum += radiance * GGX(h, roughness) * NoL / PDF. In GGX/PDF most factors cancel out and we're left with 1/cos (sine factor of the PDF only needed for the integral (I think), so we don't include it)
				if(NoL > 0)
					sum += gInputTex.SampleLevel(gInputSamp, H, mipLevel) * NoL / cosTheta;
			}
			
			return sum / NUM_SAMPLES;
		}	
	};
};
