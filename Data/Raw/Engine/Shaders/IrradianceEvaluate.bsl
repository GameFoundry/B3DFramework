#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\SHCommon.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

technique IrradianceEvaluate
{
	mixin PPBase;
	mixin SHCommon;
	mixin GBufferInput;
	mixin PerCameraData;

	blend
	{
		target	
		{
			enabled = true;
			color = { one, one, add };
		};
	};	
	
	code
	{
		#if MSAA_COUNT > 1
			Texture2DMS<uint> gInputTex;
		#else
			Texture2D<uint> gInputTex;
		#endif
		
		struct ProbeVolume
		{
			uint4 indices;
			float3x4 transform;
		};
		
		StructuredBuffer<SHVector3RGB> gSHCoeffs;
		StructuredBuffer<ProbeVolume> gProbeVolumes;
		
		TextureCube gSkyIrradianceTex;
		SamplerState gSkyIrradianceSamp;

		cbuffer Params
		{
			float gSkyBrightness;
		}				
		
		float3 getSkyIndirectDiffuse(float3 dir)
		{
			return gSkyIrradianceTex.SampleLevel(gSkyIrradianceSamp, dir, 0).rgb * gSkyBrightness;
		}
		
		float evaluateLambert(SHVector3 coeffs)
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
			output += coeffs.v2 * f;
						
			return output;
		}		
		
		float3 fsmain(VStoFS input
			#if MSAA_COUNT > 1
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{
			uint2 pixelPos = trunc(input.uv0);
		
			SurfaceData surfaceData;
			#if MSAA_COUNT > 1
				surfaceData = getGBufferData(pixelPos, sampleIdx);
			#else
				surfaceData = getGBufferData(pixelPos);
			#endif		
			
			float3 radiance;
			#if SKY_ONLY
				radiance = gSkyIrradianceTex.SampleLevel(gSkyIrradianceSamp, surfaceData.worldNormal, 0).rgb * gSkyBrightness;
			#else
				uint volumeIdx;
				#if MSAA_COUNT > 1
					volumeIdx = gInputTex.Load(uint3(pixelPos, 0), sampleIdx).x;
				#else
					volumeIdx = gInputTex.Load(uint3(pixelPos, 0)).x;
				#endif
				
				ProbeVolume volume = gProbeVolumes[volumeIdx];
				
				float3 P = NDCToWorld(input.screenPos, surfaceData.depth);
				float3 offset = float3(volume.transform[0][3], volume.transform[1][3], volume.transform[2][3]);
				float3 factors = mul((float3x3)volume.transform, P - offset);			
				float4 coords = float4(factors, 1.0f - factors.x - factors.y - factors.z);
				
				// Ignore extra points we added to make the volume cover everything
				coords = volume.indices != -1 ? coords : float4(0, 0, 0, 0);
				
				// Renormalize after potential change
				float sum = coords.x + coords.y + coords.z + coords.w;
				coords /= sum;
				
				SHVector3RGB shCoeffs = gSHCoeffs[volume.indices[0]];
				
				SHMultiply(shCoeffs, coords.x);
				SHMultiplyAdd(shCoeffs, gSHCoeffs[volume.indices[1]], coords.y);
				SHMultiplyAdd(shCoeffs, gSHCoeffs[volume.indices[2]], coords.z);
				SHMultiplyAdd(shCoeffs, gSHCoeffs[volume.indices[3]], coords.w);
				
				SHVector3 shBasis = SHBasis3(surfaceData.worldNormal);
				SHMultiply(shCoeffs.R, shBasis);
				SHMultiply(shCoeffs.G, shBasis);
				SHMultiply(shCoeffs.B, shBasis);
				
				radiance.r = evaluateLambert(shCoeffs.R);
				radiance.g = evaluateLambert(shCoeffs.G);
				radiance.b = evaluateLambert(shCoeffs.B);
			#endif // SKY_ONLY
			
			return radiance * surfaceData.albedo.rgb;
		}	
	};
};