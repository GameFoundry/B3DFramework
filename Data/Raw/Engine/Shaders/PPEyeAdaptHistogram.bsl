Parameters =
{
	Texture2D 	gSceneColorTex;
	RWTexture2D gOutputTex;
};

Blocks =
{
	Block Input;
};

#define NUM_BUCKETS (THREADGROUP_SIZE_X * THREADGROUP_SIZE_Y)

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Compute =
		{	
			cbuffer Input
			{
				// xy - offset, zw - size
				uint4 gPixelOffsetAndSize;
			
				// x - histogram scale, y - histogram offset
				float2 gHistogramParams;
				uint2 gThreadGroupCount;
			}		
		
			Texture2D gSceneColorTex;
			RWTexture2D<float4> gOutputTex;
			
			// Keep elements in this order as it ensures coalesced memory operations for non-random ops
			groupshared float sharedData[NUM_BUCKETS][THREADGROUP_SIZE_X][THREADGROUP_SIZE_Y];
			
			float calcHistogramPos(float luminance)
			{
				return saturate(log2(luminance) * gHistogramParams.x + gHistogramParams.y);
			}			
			
			[numthreads(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y, 1)]
			void main(
				uint3 groupId : SV_GroupID,
				uint3 groupThreadId : SV_GroupThreadID,
				uint3 dispatchThreadId : SV_DispatchThreadID,
				uint threadIndex : SV_GroupIndex)
			{
				// Clear everything
				for(uint i = 0; i < NUM_BUCKETS; i++)
					sharedData[i][groupThreadId.x][groupThreadId.y] = 0.0f;
					
				GroupMemoryBarrierWithGroupSync();
				
				// Sort all pixel luminance for the current thread into histogram buckets
				uint2 tileSize = uint2(LOOP_COUNT_X, LOOP_COUNT_Y);
				uint2 maxExtent = gPixelOffsetAndSize.xy + gPixelOffsetAndSize.zw;
				
				uint2 tileStart = dispatchThreadId.xy * tileSize + gPixelOffsetAndSize.xy;
				for(uint y = 0; y < LOOP_COUNT_Y; y++)
				{
					uint2 texelPos = tileStart + uint2(0, y);
					if(texelPos.y > maxExtent.y)
						break;
				
					for(uint x = 0; x < LOOP_COUNT_X; x++)
					{
						if(texelPos.x > maxExtent.x)
							break;
					
						float4 hdrColor = gSceneColorTex.Load(int3(texelPos, 0));
						float luminance = dot(hdrColor.rgb, float3(0.299f, 0.587f, 0.114f)); // TODO - Perhaps just use max() of all values?
						
						float histogramPos = calcHistogramPos(luminance);
						float bucket = histogramPos * (NUM_BUCKETS - 1) * 0.9999f;
					
						uint bucketAIdx = (uint)bucket;
						uint bucketBIdx = bucketAIdx + 1;
						
						float weightB = frac(bucket);
						float weightA = 1.0f - weightB;
						
						if(bucketAIdx != 0)
							sharedData[bucketAIdx][groupThreadId.x][groupThreadId.y] += weightA;
					
						sharedData[bucketBIdx][groupThreadId.x][groupThreadId.y] += weightB;
					
						texelPos.x++;
					}
				}
				
				GroupMemoryBarrierWithGroupSync();
				
				// Accumulate bucketed values from all threads in the group
				if(threadIndex < (NUM_BUCKETS / 4))
				{
					float4 sum = 0.0f;
					for(uint y = 0; y < THREADGROUP_SIZE_Y; y++)
					{
						for(uint x = 0; x < THREADGROUP_SIZE_X; x++)
						{
							sum += float4(
								sharedData[threadIndex * 4 + 0][x][y],
								sharedData[threadIndex * 4 + 1][x][y],
								sharedData[threadIndex * 4 + 2][x][y],
								sharedData[threadIndex * 4 + 3][x][y]
							);
						}
					}
					
					// Normalize and output histogram for the group (single line per group)
					float groupArea = THREADGROUP_SIZE_X * LOOP_COUNT_X * THREADGROUP_SIZE_Y * LOOP_COUNT_Y;

					gOutputTex[uint2(threadIndex, groupId.x + groupId.y * gThreadGroupCount.x)] = sum / groupArea;					
					
				}
			}	
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Compute =
		{
			// TODO
		};
	};
};

