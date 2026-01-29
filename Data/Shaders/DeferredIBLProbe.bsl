#if MSAA
#define MSAA_COUNT 2
#else
#define MSAA_COUNT 1
#endif

#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"
#include "$ENGINE$\DirectLighting.bslinc"

shader DeferredIBLProbe
{
	mixin GBufferInput;
	mixin PerCameraData;
	mixin ImageBasedLighting;
	mixin DirectLighting;

	depth
	{
		write = false;
		
		#if INSIDE_GEOMETRY
		read = false;
		#else
		read = true;
		#endif
	};
	
	blend
	{
		target
		{
			enabled = true;
			color = { dstA, one, add };
			alpha = { dstA, zero, add };			
		};
	};
	
	raster
	{
		#if INSIDE_GEOMETRY
		cull = cw;
		#else
		cull = ccw;
		#endif
	};

	variations
	{
		MSAA = { true, false };
		INSIDE_GEOMETRY = { true, false };
		MSAA_RESOLVE_0TH = { true, false };
	};		
	
	#if MSAA
	stencil
	{
		enabled = true;
		readmask = 0x80;
		
		#if INSIDE_GEOMETRY
		back = { keep, keep, keep, eq };
		#else
		front = { keep, keep, keep, eq };
		#endif
		
		#if MSAA_RESOLVE_0TH
		reference = 0;
		#else
		reference = 0x80;
		#endif
	};
	#endif
	
	code
	{
		struct VStoFS
		{
			float4 position : SV_POSITION;
			float4 screenPos : TEXCOORD0;
		};

		struct VertexInput
		{
			float3 position : POSITION;
			uint vertexIdx : SV_VERTEXID;
		};
		
		cbuffer PerProbe
		{
			float3 gPosition;
			float3 gExtents;
			float gTransitionDistance;
			float4x4 gInvBoxTransform;
			uint gCubemapIdx;
			uint gType; // 0 - Sphere, 1 - Box
		}			
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
			
			// Position & scale geometry
			float3 worldPosition = input.position * gExtents + gPosition;
			
			output.screenPos = mul(gMatViewProj, float4(worldPosition, 1));
			output.position = output.screenPos;
			
			return output;
		}			

		float4 fsmain(VStoFS input, float4 pixelPos : SV_Position
			#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{
			#if MSAA_COUNT > 1
				#if MSAA_RESOLVE_0TH
					SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy, 0);
				#else
					SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy, sampleIdx);
				#endif
			#else
				SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy);
			#endif			
		
			if(surfaceData.worldNormal.w > 0.0f)
			{
				ReflProbeData probeData;
				probeData.position = gPosition;
				probeData.radius = gExtents.x;
				probeData.boxExtents = gExtents;
				probeData.transitionDistance = gTransitionDistance;
				probeData.invBoxTransform = gInvBoxTransform;
				probeData.cubemapIdx = gCubemapIdx;
				probeData.type = gType;
				probeData.padding = float2(0, 0);
				
				float2 ndcPos = input.screenPos.xy / input.screenPos.w;
				float3 worldPosition = NDCToWorld(ndcPos, surfaceData.depth);			

				float3 V = normalize(gViewOrigin - worldPosition);
				float3 N = surfaceData.worldNormal.xyz;
				float3 R = 2 * dot(V, N) * N - V;
				float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);				
			
				float mipLevel = mapRoughnessToMipLevel(surfaceData.roughness, gReflCubemapNumMips);			
				
				return evaluateProbe(worldPosition, specR, mipLevel, probeData);
			}
			else
				return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	};
};
