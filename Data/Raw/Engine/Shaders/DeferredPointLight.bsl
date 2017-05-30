#include "$ENGINE$\DeferredLightCommon.bslinc"

Technique : inherits("DeferredLightCommon") =
{
	Pass =
	{
		Common = 
		{
			struct VStoFS
			{
				float4 position : SV_POSITION;
				float4 screenPos : TEXCOORD0;
			};
		};
	
		Vertex =
		{
			struct VertexInput
			{
				float3 position : POSITION;
				uint vertexIdx : SV_VERTEXID;
			};
			
			VStoFS main(VertexInput input)
			{
				VStoFS output;
				
				float3 worldPosition;
				uint numSides = gLightGeometry.x;
				if(numSides > 0) // Generate spot light geometry
				{
					uint numSlices = gLightGeometry.y;
					float radius = gLightGeometry.w;
					float angle = gLightSpotAnglesAndSqrdInvAttRadius.x;
				
					// Extra scale to ensure edges lie on the circle, not inside it
					// TODO - These can be precomputed
					float extraRadiusScale = 1.0f / cos(PI / (float)numSides);
					float angleTan = tan(angle);
					float height = radius / angleTan;
		
					uint sphereStartIdx = numSides * numSlices;
					// Cone vertices
					if (input.vertexIdx < sphereStartIdx)
					{
						uint sliceIdx = input.vertexIdx / numSides;
						uint sideIdx = input.vertexIdx % numSides;

						float curAngle = sideIdx * 2 * PI / (float)numSides;
						float sliceOffset = height * sliceIdx / (float)(numSlices - 1);
						float sliceRadius = sliceOffset * angleTan * extraRadiusScale;

						float4 localPos = float4(sliceRadius * cos(curAngle), 
							sliceRadius * sin(curAngle), -sliceOffset, 1.0f);
						worldPosition = (mul(gMatConeTransform, localPos)).xyz;
					}
					else // Sphere cap vertices
					{
						uint sphereVertexIdx = input.vertexIdx - sphereStartIdx;
						uint sliceIdx = sphereVertexIdx / numSides;
						uint sideIdx = sphereVertexIdx % numSides;

						float curAngle = sideIdx * 2 * PI / (float)numSides;
						float sliceOffset = radius * sliceIdx / (float)(numSlices - 1);
						float sliceRadius = sqrt(max(0.0f, radius * radius - sliceOffset * sliceOffset)) * extraRadiusScale;

						float4 localPos = float4(sliceRadius * cos(curAngle), 
							sliceRadius * sin(curAngle), -height - sliceOffset, 1.0f);
						worldPosition = (mul(gMatConeTransform, localPos)).xyz;
					}
				}
				else // Scale and position pre-generated sphere geometry
				{
					worldPosition = input.position * gLightGeometry.z + gLightPositionAndSrcRadius.xyz;
				}
				
				output.screenPos = mul(gMatViewProj, float4(worldPosition, 1));
				output.position = output.screenPos;
				
				return output;
			}			
		};
		
		Fragment = 
		{		
			float4 main(VStoFS input, uint sampleIdx : SV_SampleIndex) : SV_Target0
			{
				float2 ndcPos = input.screenPos.xy / input.screenPos.w;
				uint2 pixelPos = NDCToScreen(ndcPos);
				
				#if MSAA_COUNT > 1
				SurfaceData surfaceData = getGBufferData(pixelPos, sampleIdx);
				#else
				SurfaceData surfaceData = getGBufferData(pixelPos);
				#endif

				if(surfaceData.worldNormal.w > 0.0f)
				{
					// x, y are now in clip space, z, w are in view space
					// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
					// z, w eliminated (since they are already in view space)
					// Note: Multiply by depth should be avoided if using ortographic projection
					float4 mixedSpacePos = float4(ndcPos.xy * -surfaceData.depth, surfaceData.depth, 1);
					float4 worldPosition4D = mul(gMatScreenToWorld, mixedSpacePos);
					float3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;

					float3 V = normalize(gViewOrigin - worldPosition);
					float3 N = surfaceData.worldNormal.xyz;
					float3 R = 2 * dot(V, N) * N - V;
					float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
					
					float roughness2 = max(surfaceData.roughness, 0.08f);
					roughness2 *= roughness2;
					
					LightData lightData = getLightData();
					
					bool isSpot = gShiftedLightPositionAndType.w > 0.5f;
					if(isSpot)
						return float4(getLuminanceSpot(lightData, worldPosition, V, R, roughness2, surfaceData), 1.0f);
					else // Radial
						return float4(getLuminanceRadial(lightData, worldPosition, V, R, roughness2, surfaceData), 1.0f);
				}
				else
					return float4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		};
	};
};