#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\SHCommon.bslinc"

Technique
 : inherits("ReflectionCubemapCommon")
 : inherits("SHCommon")
{
	Language = "HLSL11";
	
	Pass =
	{
		Compute = 
		{
			struct SHCoeffsAndWeight
			{
				SHVector5RGB coeffs;
				float weight;
			}
		
			SamplerState gInputSamp;
			TextureCube gInputTex;
		
			RWStructuredBuffer<SHCoeffsAndWeight> gOutput;
			
			cbuffer Params
			{
				uint gCubeFace;
				uint gFaceSize;
				uint2 gDispatchSize;
			}			
			
			groupshared SHCoeffsAndWeight sCoeffs[TILE_WIDTH * TILE_HEIGHT];

			/** 
			 * Integrates area of a cube face projected onto the surface of the sphere, from [0, 0] to [u, v]. 
			 * u & v expected in [-1, -1] to [1, 1] range.
			 *
			 * See http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/ for derivation.
			 */
			float integrateProjectedCubeArea(float u, float v)
			{
				return atan2(u * v, sqrt(u * u + v * v + 1.0f));
			}
			
			/** Calculates solid angle of a texel projected onto a sphere. */
			float texelSolidAngle(float u, float v, float invFaceSize)
			{
				float x0 = u - invFaceSize;
				float x1 = u + invFaceSize;
				float y0 = v - invFaceSize;
				float y1 = v + invFaceSize;

				return   integrateProjectedCubeArea(x1, y1)
                       - integrateProjectedCubeArea(x0, y1)
                       - integrateProjectedCubeArea(x1, y0)
                       + integrateProjectedCubeArea(x0, y0);
			}
			
			[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
			void main(
				uint groupIdx : SV_GroupIndex,
				uint groupId : SV_GroupID,
				uint3 dispatchThreadId : SV_DispatchThreadID)
			{
				SHCoeffsAndWeight data;
				data.weight = 0;
				
				SHZero(data.coeffs.R);
				SHZero(data.coeffs.G);
				SHZero(data.coeffs.B);
				
				float invFaceSize = 1.0f / gFaceSize;
			
				uint2 pixelCoords = dispatchThreadId.xy * PIXELS_PER_THREAD;
				uint2 pixelCoordsEnd = pixelCoords + uint2(PIXELS_PER_THREAD, PIXELS_PER_THREAD);
				for(uint y = pixelCoords.y; y < pixelCoordsEnd.y; y++)
				{
					for(uint x = pixelCoords.x; x < pixelCoordsEnd.x; x++)
					{
						// Ignore pixels out of valid range
						if (x >= gFaceSize || y >= gFaceSize)
							break;
							
						// Map from [0, size-1] to [-1.0 + invSize, 1.0 - invSize].
                        // Reference: u = 2.0 * (x + 0.5) / faceSize - 1.0;
                        //      	  v = 2.0 * (y + 0.5) / faceSize - 1.0;
						// (+0.5 in order to sample center of texel)
                        float u = x * invFaceSize - 1.0f;
                        float v = x * invFaceSize - 1.0f;
						
						float3 dir = getDirFromCubeFace(gCubeFace, float2(u, v));
						
						// Need to calculate solid angle (weight) of the texel, as cube face corners have
						// much smaller solid angle, meaning many of them occupy the same area when projected
						// on a sphere. Without weighing that area would look too bright.
						float weight = texelSolidAngle(u, v, invFaceSize);
						
						SHVector5 shBasis = SHBasis5(dir);
						float3 radiance = gInputTex.SampleLevel(gInputSamp, dir, 0);
						
						SHMultiplyAdd(data.coeffs.R, shBasis, radiance.r * weight);
						SHMultiplyAdd(data.coeffs.G, shBasis, radiance.g * weight);
						SHMultiplyAdd(data.coeffs.B, shBasis, radiance.b * weight);
						
						data.weight += weight;
					}
				}
				
				sCoeffs[groupIdx] = data;
				
				GroupMemoryBarrierWithGroupSync();
				
				int numThreads = TILE_WIDTH * TILE_HEIGHT;
				[unroll]
				for(int tc = numThreads / 2; tc > 0; tc >>= 1)
				{
					if(groupIdx < tc)
					{
						SHAdd(sCoeffs[groupIdx].R, sCoeffs[groupIdx + tc].R);
						SHAdd(sCoeffs[groupIdx].G, sCoeffs[groupIdx + tc].G);
						SHAdd(sCoeffs[groupIdx].B, sCoeffs[groupIdx + tc].B);

						sCoeffs[groupIdx].weight += sCoeffs[groupIdx + tc].weight;
					}
				
					GroupMemoryBarrierWithGroupSync();
				}
				
				if(groupIdx == 0)
				{
					uint outputIdx = groupId.y * gDispatchSize.x + groupId.x;
					gOutput[outputIdx] = sCoeffs[0];
				}
			}
		};
	};
};