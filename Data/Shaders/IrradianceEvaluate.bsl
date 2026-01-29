#if MSAA
#define MSAA_COUNT 2
#else
#define MSAA_COUNT 1
#endif

#include "$ENGINE$\PPBase.bslinc"
#define SH_ORDER 3
#include "$ENGINE$\SHCommon.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader IrradianceEvaluate
{
	mixin PPBase;
	mixin SHCommon;
	mixin GBufferInput;
	mixin PerCameraData;
	
	variations
	{
		MSAA = { true, false };
		MSAA_RESOLVE_0TH = { true, false };
		SKY_ONLY = { true, false };
	};
	
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
		
		struct Tetrahedron
		{
			uint4 indices;
			uint2 offsets[4];
			float3x4 transform;
		};
		
		struct TetrahedronFace
		{
			float4 corners[3];
			float4 normals[3];
			uint isQuadratic;
			float padding[3];
		};		
		
		Texture2D gSHCoeffs;
		Buffer<uint4> gTetrahedra;
		Buffer<float4> gTetFaces;
		
		TextureCube gSkyIrradianceTex;
		Texture2D gAmbientOcclusionTex;
		SamplerState gLinearSamp;

		cbuffer Params
		{
			float gSkyBrightness;
			uint gNumTetrahedra;
		}
		
		Tetrahedron getTetrahedron(uint idx)
		{
			Tetrahedron output;
			
			uint4 data[6];
			
			[unroll]
			for(int i = 0; i < 6; i++)
				data[i] = gTetrahedra.Load(idx * 6 + i);
			
			output.indices = data[0];
			output.offsets[0] = data[1].xy;
			output.offsets[1] = data[1].zw;
			output.offsets[2] = data[2].xy;
			output.offsets[3] = data[2].zw;
			output.transform = float3x4(asfloat(data[3]), asfloat(data[4]), asfloat(data[5]));
			
			return output;
		}
		
		TetrahedronFace getTetrahedronFace(uint idx)
		{
			TetrahedronFace output;
			
			float4 data[7];
			
			[unroll]
			for(int i = 0; i < 7; i++)
				data[i] = gTetFaces.Load(idx * 7 + i);
			
			[unroll]
			for(int i = 0; i < 3; i++)
			{
				output.corners[i] = data[i];
				output.normals[i] = data[3 + i];
				output.padding[i] = 0;
			}
			
			output.isQuadratic = asuint(data[6].x);
			
			return output;		
		}
		
		float3 getSkyIndirectDiffuse(float3 dir)
		{
			return gSkyIrradianceTex.SampleLevel(gLinearSamp, dir, 0).rgb * gSkyBrightness;
		}
		
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

			return output;
		}	

		float solveQuadratic(float A, float B, float C)
		{
			const float EPSILON = 0.00001f;
		
			if (abs(A) > EPSILON)
			{
				float p = B / (2 * A);
				float q = C / A;
				float D = p * p - q;

				return sqrt(D) - p;
			}
			else
			{
				if(abs(B) > EPSILON)
					return -C / B;
			
				return 0.0f;
			}
		}
		
		float solveCubic(float A, float B, float C)
		{
			const float EPSILON = 0.00001f;
			const float THIRD = 1.0f / 3.0f;
		
			float sqA = A * A;
			float p = THIRD * (-THIRD * sqA + B);
			float q = (0.5f) * ((2.0f / 27.0f) * A * sqA - THIRD * A * B + C);

			float cbp = p * p * p;
			float D = q * q + cbp;
			
			float t;
			if(D < 0.0f)
			{
				float phi = THIRD * acos(-q / sqrt(-cbp));
				t = (2 * sqrt(-p)) * cos(phi);
			}
			else
			{
				float sqrtD = sqrt(D);
				float u = pow(sqrtD + abs(q), 1.0f / 3.0f);

				
				if (q > 0.0f)
					t = -u + p / u;
				else
					t = u - p / u;
			}
			
			return t - THIRD * A;
		}
		
		float3 calcTriBarycentric(float3 p, float3 a, float3 b, float3 c)
		{
			float3 v0 = b - a;
			float3 v1 = c - a;
			float3 v2 = p - a;
			
			float d00 = dot(v0, v0);
			float d01 = dot(v0, v1);
			float d11 = dot(v1, v1);
			float d20 = dot(v2, v0);
			float d21 = dot(v2, v1);
			
			float denom = d00 * d11 - d01 * d01;
			float3 coords;
			coords.x = (d11 * d20 - d01 * d21) / denom;
			coords.y = (d00 * d21 - d01 * d20) / denom;
			coords.z = 1.0f - coords.x - coords.y;
			
			return coords;
		}
		
		float4 fsmain(VStoFS input
			#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{
			uint2 pixelPos = trunc(input.uv0);
		
			SurfaceData surfaceData;
			#if MSAA_COUNT > 1
				#if MSAA_RESOLVE_0TH
					surfaceData = getGBufferData(pixelPos, 0);
				#else
					surfaceData = getGBufferData(pixelPos, sampleIdx);
				#endif
			#else
				surfaceData = getGBufferData(pixelPos);
			#endif		
			
			float3 irradiance = 0;
			#if SKY_ONLY
				irradiance = gSkyIrradianceTex.SampleLevel(gLinearSamp, surfaceData.worldNormal, 0).rgb * gSkyBrightness;
			#else
				uint volumeIdx;
				#if MSAA_COUNT > 1
					#if MSAA_RESOLVE_0TH
						volumeIdx = gInputTex.Load(uint3(pixelPos, 0), 0).x;
					#else
						volumeIdx = gInputTex.Load(uint3(pixelPos, 0), sampleIdx).x;
					#endif
				#else
					volumeIdx = gInputTex.Load(uint3(pixelPos, 0)).x;
				#endif
				
				if(volumeIdx == 0xFFFF) // Using 16-bit texture, so need to compare like this
					irradiance = gSkyIrradianceTex.SampleLevel(gLinearSamp, surfaceData.worldNormal, 0).rgb * gSkyBrightness;
				else
				{
					Tetrahedron volume = getTetrahedron(volumeIdx);
					
					float3 P = NDCToWorld(input.screenPos, surfaceData.depth);
					
					float4 coords;
					[branch]
					if(volumeIdx >= gNumTetrahedra)
					{
						uint faceIdx = volumeIdx - gNumTetrahedra;
						TetrahedronFace face = getTetrahedronFace(faceIdx);
					
						float3 factors = mul(volume.transform, float4(P, 1.0f));
						float A = factors.x;
						float B = factors.y;
						float C = factors.z;

						float t;
						if(face.isQuadratic > 0)
							t = solveQuadratic(A, B, C);
						else
							t = solveCubic(A, B, C);
							
						float3 triA = face.corners[0].xyz + t * face.normals[0].xyz;
						float3 triB = face.corners[1].xyz + t * face.normals[1].xyz;
						float3 triC = face.corners[2].xyz + t * face.normals[2].xyz;
						
						float3 bary = calcTriBarycentric(P, triA, triB, triC);
						
						coords.x = bary.z;
						coords.yz = bary.xy;
						coords.w = 0.0f;
					}
					else
					{
						float3 factors = mul(volume.transform, float4(P, 1.0f));			
						coords = float4(factors, 1.0f - factors.x - factors.y - factors.z);
					}

					SHVectorRGB shCoeffs;
					SHZero(shCoeffs);
					
					for(uint i = 0; i < 4; ++i)
					{
						if(coords[i] > 0.0f)
						{
							SHVectorRGB coeff = SHLoad(gSHCoeffs, volume.offsets[i]);
							SHMultiplyAdd(shCoeffs, coeff, coords[i]);
						}
					}
					
					SHVector shBasis = SHBasis(surfaceData.worldNormal);
					SHMultiply(shCoeffs.R, shBasis);
					SHMultiply(shCoeffs.G, shBasis);
					SHMultiply(shCoeffs.B, shBasis);
					
					irradiance.r = evaluateLambert(shCoeffs.R);
					irradiance.g = evaluateLambert(shCoeffs.G);
					irradiance.b = evaluateLambert(shCoeffs.B);
				}
			#endif // SKY_ONLY
			
			float2 uv = NDCToUV(input.screenPos);
			float ao = gAmbientOcclusionTex.Sample(gLinearSamp, uv);
			
			float3 diffuseColor = lerp(surfaceData.albedo.rgb, float3(0.0f, 0.0f, 0.0f), surfaceData.metalness);
			return float4(irradiance * diffuseColor * ao, 1.0f);
		}	
	};
};
