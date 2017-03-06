#include "$ENGINE$\PPBase.bslinc"

Parameters =
{
	int			gCubeFace;
	SamplerCUBE	gInputSamp : alias("gInputTex");
	TextureCUBE gInputTex;
};

Blocks =
{
	Block Input;
};

Technique : inherits("PPBase") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			#define PI 3.1415926
		
			// From Hacker's Delight
			float reverseBits(uint bits) 
			{
				bits = (bits << 16u) | (bits >> 16u);
				bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
				bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
				bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
				bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
				
				return float(bits) * 2.3283064365386963e-10; // 0x100000000
			}
			
			float2 hammersleySequence(uint i, uint count)
			{
				float2 output;
				output.x = i / (float)count;
				output.y = reverseBits(i);
				
				return output;
			}
			
			// Returns cos(theta) in x and phi in y
			float2 importanceSampleGGX(float2 e, float roughness4)
			{
				// See GGXImportanceSample.nb for derivation (essentially, take base GGX, normalize it,
				// generate PDF, split PDF into marginal probability for theta and conditional probability
				// for phi. Plug those into the CDF, invert it.)				
				float cosTheta = sqrt((1.0f - e.x) / (1.0f + (roughness4 - 1.0f) * e.y));
				float phi = 2.0f * PI * e.y;
				
				return float2(cosTheta, phi);
			}
			
			float3 sphericalToCartesian(float cosTheta, float sinTheta, float phi)
			{
				float3 output;
				output.x = sinTheta * cos(phi);
				output.y = sinTheta * sin(phi);
				output.z = cosTheta;
				
				return output;
			}
			
			float pdfGGX(float cosTheta, float sinTheta, float roughness4)
			{
				float d = (cosTheta*roughness4 - cosTheta) * cosTheta + 1;
				return roughness4 * cosTheta * sinTheta / (d*d*PI);
			}
			
			float mapRoughnessToMipLevel(float roughness, uint maxMipLevel)
			{
				// We use the following equation:
				//    mipLevel = log10(roughness) / log10(dropPercent)
				//
				// Where dropPercent represent by what % to drop the roughness with each mip level.
				// We convert to log2 and a assume a drop percent value of 0.6. This gives us:
				//    mipLevel = -1.35692 * log2(roughness);
				
				return max(0, maxMipLevel - (-1.35692 * log2(roughness)));
			}
			
			float mapMipLevelToRoughness(uint mipLevel)
			{
				// Inverse of mapRoughnessToMipLevel()
				return 1.0f - exp2(-0.7369655941662063 * mipLevel);
			}
			
			cbuffer Input
			{
				int gCubeFace;
				int gMipLevel;
				float gPrecomputedMipFactor;
			}	
		
			SamplerState gInputSamp;
			TextureCube gInputTex;

			float4 main(VStoFS input) : SV_Target0
			{
				float2 scaledUV = input.uv0 * 2.0f - 1.0f;
														
				float3 N;
				if(gCubeFace == 0)
					N = float3(1.0f, -scaledUV.y, -scaledUV.x);
				else if(gCubeFace == 1)
					N = float3(-1.0f, -scaledUV.y, scaledUV.x);
				else if(gCubeFace == 2)
					N = float3(scaledUV.x, 1.0f, scaledUV.y);
				else if(gCubeFace == 3)
					N = float3(scaledUV.x, -1.0f, -scaledUV.y);
				else if(gCubeFace == 4)
					N = float3(scaledUV.x, -scaledUV.y, 1.0f);
				else
					N = float3(-scaledUV.x, -scaledUV.y, -1.0f);
				
				N = normalize(N);
				
				// Determine which mip level to sample from depending on PDF and cube -> sphere mapping distortion
				float distortion = rcp(pow(N.x * N.x + N.y * N.y + N.z * N.z, 3.0f/2.0f));
				
				float roughness = mapMipLevelToRoughness(gMipLevel);
				float roughness2 = roughness * roughness;
				float roughness4 = roughness2 * roughness2;
				
				float4 sum = 0;
				for(uint i = 0; i < NUM_SAMPLES; i++)
				{
					float random = hammersleySequence(i, NUM_SAMPLES);
					float2 sphericalH = importanceSampleGGX(random, roughness4);
					
					float cosTheta = sphericalH.x;
					float phi = sphericalH.y;
					
					float sinTheta = sqrt(1.0f - cosTheta);
					
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
					
					// sum += H * GGX / PDF. In GGX/PDF most factors cancel out and we're left with 1/sin*cos
					sum += gInputTex.SampleLevel(gInputSamp, H, mipLevel) / (cosTheta * sinTheta);
				}
				
				return sum / NUM_SAMPLES;
			}	
		};
	};
};

Technique : inherits("PPBase") =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			in VStoFS
			{
				layout(location = 0) vec2 uv0;
			} FSInput;		
		
			layout(location = 0) out vec4 fragColor;
		
			layout(binding = 0) uniform Input
			{
				int gCubeFace;
			};
			
			layout(binding = 1) uniform samplerCube gInputTex;
			
			void main()
			{
				vec2 scaledUV = FSInput.uv0 * 2.0f - 1.0f;
				
				vec3 dir;
				if(gCubeFace == 0)
					dir = vec3(1.0f, -scaledUV.y, -scaledUV.x);
				else if(gCubeFace == 1)
					dir = vec3(-1.0f, -scaledUV.y, scaledUV.x);
				else if(gCubeFace == 2)
					dir = vec3(scaledUV.x, 1.0f, scaledUV.y);
				else if(gCubeFace == 3)
					dir = vec3(scaledUV.x, -1.0f, -scaledUV.y);
				else if(gCubeFace == 4)
					dir = vec3(scaledUV.x, -scaledUV.y, 1.0f);
				else
					dir = vec3(-scaledUV.x, -scaledUV.y, -1.0f);
				
				fragColor = texture(gInputTex, dir);
			}	
		};
	};
};