#include "$ENGINE$/GBufferInput.bslinc"
#include "$ENGINE$/ShadowProjectionCommon.bslinc"

shader ShadowProjectOmni
{
	mixin GBufferInput;
	mixin ShadowProjectionCommon;

	variations
	{
		SHADOW_QUALITY = { 1, 2, 3, 4 };
		VIEWER_INSIDE_VOLUME = { true, false };
		NEEDS_TRANSFORM = { true, false };
		MSAA_COUNT = { 1, 2 };
	};	
	
	blend
	{
		target
		{
			enabled = true;
			writemask = R;
			color = { one, one, max };
		};
	};
	
	#if VIEWER_INSIDE_VOLUME
	depth
	{
		read = false;
		write = false;
	};
	
	raster
	{
		cull = cw;
	};
	#else
	depth
	{
		write = false;
	};
	
	raster
	{
		cull = ccw;
	};
	#endif
	
	code
	{
		// Random samples on a disc of radius 2.5. Random values generated using low discrepancy
		// Hammersley sequence and then mapped to the disc.
		static const float2 discSamples4[4] =
		{
			float2(0, 0),
			float2(-1.25, 6.69872e-08),
			float2(4.73671e-08, 1.76777),
			float2(-1.74038e-07, -2.16506)
		};
		
		static const float2 discSamples12[12] =
		{
			float2(0, 0),
			float2(-0.721688, 3.86751e-08),
			float2(2.73474e-08, 1.02062),
			float2(-1.00481e-07, -1.25),
			float2(1.02062, 1.02062),
			float2(-1.14109, -1.14109),
			float2(-1.25, 1.25),
			float2(1.35015, -1.35015),
			float2(1.88586, 0.781149),
			float2(-2.00026, -0.828534),
			float2(-0.873351, 2.10846),
			float2(0.915979, -2.21137)
		};

		static const float2 discSamples32[32] =
		{
			float2(0, 0),
			float2(-0.441942, 2.36836e-08),
			float2(1.67468e-08, 0.625),
			float2(-6.15317e-08, -0.765466),
			float2(0.625, 0.625),
			float2(-0.698771, -0.698771),
			float2(-0.765465, 0.765466),
			float2(0.826797, -0.826797),
			float2(1.15485, 0.478354),
			float2(-1.2249, -0.507371),
			float2(-0.534816, 1.29116),
			float2(0.56092, -1.35418),
			float2(0.585862, 1.4144),
			float2(-0.609785, -1.47215),
			float2(-1.52772, 0.632803),
			float2(1.58134, -0.655014),
			float2(1.7338, 0.344874),
			float2(-1.78716, -0.355488),
			float2(-0.365794, 1.83897),
			float2(0.375818, -1.88936),
			float2(1.09804, 1.64334),
			float2(-1.12516, -1.68392),
			float2(-1.72355, 1.15164),
			float2(1.76228, -1.17752),
			float2(1.80018, 1.20284),
			float2(-1.83731, -1.22765),
			float2(-1.25196, 1.87369),
			float2(1.27581, -1.90938),
			float2(0.456226, 2.2936),
			float2(-0.464301, -2.3342),
			float2(-2.3741, 0.472239),
			float2(2.41335, -0.480045)
		};
		
		TextureCube gShadowCubeTex;
		SamplerComparisonState gShadowCubeSampler;
		
		cbuffer Params
		{
			float4x4 gFaceVPMatrices[6];
			float4 gLightPosAndRadius;
			float gInvResolution;
			float gFadePercent;
			float gDepthBias;
		};			
		
		// Returns occlusion where 1 = fully shadowed, 0 = not shadowed
		float cubemapPCF(float3 worldPos, float3 lightPos, float lightRadius)
		{
			float3 fromLight = worldPos - lightPos;
			float distToLight = length(fromLight);
			
			// No occlusion if outside radius
			if(distToLight > lightRadius)
				return 0.0f;
				
			float3 lightDir = fromLight / distToLight;
			
			float3 up = abs(lightDir.z) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
			float3 side = normalize(cross(up, lightDir));
			up = cross(lightDir, side);
			
			up *= gInvResolution;
			side *= gInvResolution;
			
			// Determine cube face to sample from
			float3 absFromLight = abs(fromLight);
			float maxComponent = max(absFromLight.x, max(absFromLight.y, absFromLight.z));
			
			int faceIdx = 0;
			if(maxComponent == absFromLight.x)
				faceIdx = fromLight.x > 0.0f ? 0 : 1;
			else if(maxComponent == absFromLight.z)
				faceIdx = fromLight.z > 0.0f ? 4 : 5;
			else
				faceIdx = fromLight.y > 0.0f ? 2 : 3;
			
			// Get position of the receiver in shadow space
			float4 shadowPos = mul(gFaceVPMatrices[faceIdx], float4(worldPos, 1));
			
			float receiverDepth = NDCZToDeviceZ(shadowPos.z / shadowPos.w);
			float shadowBias = gDepthBias / shadowPos.w;
			
			float occlusion = 0.0f;
			#if SHADOW_QUALITY <= 1
				occlusion = gShadowCubeTex.SampleCmp(gShadowCubeSampler, lightDir, receiverDepth - shadowBias);
			#elif SHADOW_QUALITY == 2
				[unroll]
				for(int i = 0; i < 4; ++i)
				{
					float3 sampleDir = lightDir + side * discSamples4[i].x + up * discSamples4[i].y;
					occlusion += gShadowCubeTex.SampleCmp(gShadowCubeSampler, sampleDir, receiverDepth - shadowBias);
				}
				
				occlusion /= 4;
			#elif SHADOW_QUALITY == 3
				[unroll]
				for(int i = 0; i < 12; ++i)
				{
					float3 sampleDir = lightDir + side * discSamples12[i].x + up * discSamples12[i].y;
					occlusion += gShadowCubeTex.SampleCmp(gShadowCubeSampler, sampleDir, receiverDepth - shadowBias);
				}
				
				occlusion /= 12;
			#else
				[unroll]
				for(int i = 0; i < 32; ++i)
				{
					float3 sampleDir = lightDir + side * discSamples32[i].x + up * discSamples32[i].y;
					occlusion += gShadowCubeTex.SampleCmp(gShadowCubeSampler, sampleDir, receiverDepth - shadowBias);
				}
				
				occlusion /= 32;
			#endif
			
			return occlusion;
		}
		
		float4 fsmain(VStoFS input, uint sampleIdx : SV_SampleIndex) : SV_Target0
		{
			float2 ndcPos = input.clipSpacePos.xy / input.clipSpacePos.w;
			uint2 pixelPos = NDCToScreen(ndcPos);
		
			// Get depth & calculate world position
			#if MSAA_COUNT > 1
			float deviceZ = gDepthBufferTex.Load(pixelPos, sampleIdx).r;
			#else		
			float deviceZ = gDepthBufferTex.Load(int3(pixelPos, 0)).r;
			#endif
			
			float depth = convertFromDeviceZ(deviceZ);
			float3 worldPos = NDCToWorld(ndcPos, depth);
		
			float occlusion = cubemapPCF(worldPos, gLightPosAndRadius.xyz, gLightPosAndRadius.w);
			occlusion *= gFadePercent;
			
			// Encode to get better precision in the blacks, similar to gamma correction but cheaper to execute
			return sqrt(occlusion);
		}
	};
};
