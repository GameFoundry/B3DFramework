#include "$ENGINE$/GpuParticleTileVertex.bslinc"
#include "$ENGINE$/PerCameraData.bslinc"
#include "$ENGINE$/PerObjectData.bslinc"

shader GpuParticleSimulate
{
	variations
	{
		DEPTH_COLLISIONS = { 0, 1, 2 };
	};

	mixin GpuParticleTileVertex;
	
	#if DEPTH_COLLISIONS
		mixin PerCameraData;
		mixin PerObjectData;
	#endif
	
	code
	{
		Texture2D gPosAndTimeTex;
		[alias(gPosAndTimeTex)]
		SamplerState gPosAndTimeSampler
		{
            Filter = MIN_MAG_MIP_POINT;
        };

		Texture2D gVelocityTex;
		[alias(gVelocityTex)]
		SamplerState gVelocitySampler
		{
            Filter = MIN_MAG_MIP_POINT;
        };
		
		Texture3D gVectorFieldTex;
		[alias(gVectorFieldTex)]
		SamplerState gVectorFieldSampler
		{
			AddressU = CLAMP;
			AddressV = CLAMP;
			AddressW = CLAMP;
		};
		
		#if DEPTH_COLLISIONS
		Texture2D gDepthTex;
		Texture2D gNormalsTex;
		
		[alias(gDepthTex)]
		SamplerState gDepthSampler
		{
            Filter = MIN_MAG_MIP_POINT;
        };
		
		[alias(gNormalsTex)]
		SamplerState gNormalsSampler;
		
		Texture2D gSizeRotationTex;
		[alias(gSizeRotationTex)]
		SamplerState gSizeRotationSampler
		{
            Filter = MIN_MAG_MIP_POINT;
        };
		
		Texture2D gCurvesTex;
		
		[alias(gCurvesTex)]
		SamplerState gCurvesSampler;
		
		#endif
		
		cbuffer VectorFieldParams
		{
			float3 gFieldBounds;
			float gFieldIntensity;
			float3 gFieldTiling;
			float gFieldTightness;
			float4x4 gWorldToField;
			float3x3 gFieldToWorld;
		};
		
		cbuffer Params
		{
			uint gNumVectorFields;
			uint gNumIterations;
			float gDT;
			float gDrag;
			float3 gAcceleration;
		};
		
		#if DEPTH_COLLISIONS
		cbuffer DepthCollisionParams
		{
			float gCollisionRange;
			float gRestitution;
			float gDampening;
			float gCollisionRadiusScale;
			
			float2 gSizeScaleCurveOffset;
			float2 gSizeScaleCurveScale;
		};
		
		void integrateWithDepthCollisions(
			float3 inPosition, float3 inVelocity, 
			out float3 outPosition, out float3 outVelocity,
			float dt, 
			float3 acceleration, float radius)
		{
			// Integrate as normal
			float3 deltaPosPerSec = inVelocity + acceleration * 0.5f;
			float3 deltaPos = deltaPosPerSec * dt;
			
			outPosition = inPosition + deltaPos;
			outVelocity = inVelocity + acceleration;
			
			// Determine potential collision point (edge of the particle sphere in the direction of travel)
			float3 dirOffset = normalize(deltaPos) * radius;
			float3 offsetPointWrld = inPosition + dirOffset;
			float4 offsetPointClip = mul(gMatViewProj, float4(offsetPointWrld, 1.0f));
			float2 offsetPointNdc = offsetPointClip.xy / offsetPointClip.w;
			
			// Potential collision point out of view
			if(any(abs(offsetPointNdc.xy) > float2(1.0f, 1.0f)))
				return;
				
			float2 offsetPointUV = NDCToUV(offsetPointNdc);
			float sampledDepth = convertFromDeviceZ(gDepthTex.Sample(gDepthSampler, offsetPointUV).r);
			
			// Potential collision point too far away from sampled depth
			if(abs(-offsetPointClip.w - sampledDepth) >= gCollisionRange)
				return;
				
			float3 hitPointWrld = NDCToWorld(offsetPointNdc.xy, sampledDepth);
			float3 hitPointNormal = gNormalsTex.Sample(gNormalsSampler, offsetPointUV).xyz * 2.0f - 1.0f;
			
			float4 hitPlane = float4(hitPointNormal, dot(hitPointNormal, hitPointWrld));
			float speedToPlane = dot(hitPlane.xyz, deltaPos);
			
			// Moving away from the collision plane
			if(speedToPlane >= 0.0f)
				return;
			
			// Check if capsule (sweeped sphere) intersects the plane
			float distToOldPos = dot(inPosition, hitPlane.xyz) - hitPlane.w;
			float distToNewPos = dot(outPosition, hitPlane.xyz) - hitPlane.w;
			
			if(distToOldPos >= -radius && distToNewPos <= radius)
			{
				// Separate velocity into perpendicular and tangent velocity
				// Note: This is using mid-velocity for both position and velocity integration. Velocity integration should
				// should use the full acceleration value, but we'll keep it imprecise to avoid another calculation.
				float3 perpVelocity = dot(deltaPosPerSec, hitPlane.xyz) * hitPlane.xyz;
				float3 tanVelocity = deltaPosPerSec - perpVelocity;
				
				outVelocity = (1.0f - gDampening) * tanVelocity - gRestitution * perpVelocity;
				
				float t = (distToOldPos - radius) / -speedToPlane;
				outPosition = inPosition + deltaPos * t + outVelocity * (1.0f - t) * dt;
			}
		}		
		
		#endif
		
		float3 evaluateVectorField(float3 pos, float scale, out float3 velocity, out float tightness)
		{
			if(gNumVectorFields == 0)
			{
				velocity = 0.0f;
				tightness = 0.0f;
				return 0.0f;
			}
		
			float3 uv = mul(gWorldToField, float4(pos, 1.0f)).xyz;
			uv -= floor(uv * gFieldTiling);
			
			float3 smp = gVectorFieldTex.Sample(gVectorFieldSampler, uv);
			smp = mul(gFieldToWorld, smp);
			
			velocity = gFieldIntensity * gFieldTightness * smp;
			tightness = gFieldTightness;
			return gFieldIntensity * smp;
		}
	
		void fsmain(VStoFS input, 
			out float4 outPosAndTime : SV_Target0, 
			out float4 outVelocityAndTimeScale : SV_Target1)
		{
			float4 posAndTime = gPosAndTimeTex.Sample(gPosAndTimeSampler, input.uv0);
			float4 velocityAndTimeScale = gVelocityTex.Sample(gVelocitySampler, input.uv0);
			
			float3 position = posAndTime.xyz;
			float time = posAndTime.w;
			float3 velocity = velocityAndTimeScale.xyz;
			float timeScale = velocityAndTimeScale.w;
			
			for(uint i = 0; i < gNumIterations; i++)
			{
				time += gDT * timeScale;
			
				// Constant acceleration
				float3 totalForce = gAcceleration;
			
				// Evaluate vector field
				float3 fieldVelocity;
				float fieldTightness;
				totalForce += evaluateVectorField(posAndTime.xyz, 1.0f, fieldVelocity, fieldTightness);
				
				velocity = lerp(velocity, fieldVelocity, fieldTightness);
				
				// Drag
				totalForce -= gDrag * velocity;
				
				// Integrate
				float3 acceleration = totalForce * gDT;
				
#if DEPTH_COLLISIONS
				float2 size = gSizeRotationTex.Sample(gSizeRotationSampler, input.uv0).xy;
			
				float2 sizeScaleCurveUV = gSizeScaleCurveOffset + time * gSizeScaleCurveScale;
				float2 sizeScale = gCurvesTex.Sample(gCurvesSampler, sizeScaleCurveUV, 0).xy;
			
				// TODO - Apply world transform scale
				size *= 0.5f * sizeScale;
				float collisionRadius = min(size.x, size.y) * gCollisionRadiusScale;

			#if DEPTH_COLLISIONS == 2
				position = mul(gMatWorld, float4(position, 1.0f)).xyz;
				velocity = mul(gMatWorld, float4(velocity, 0.0f)).xyz;			
			#endif
				
				float3 outPosition;
				float3 outVelocity;
				integrateWithDepthCollisions(
					position, velocity, 
					outPosition, outVelocity,
					gDT, 
					acceleration, collisionRadius);
					
			#if DEPTH_COLLISIONS == 2
				position = mul(gMatInvWorld, float4(outPosition, 1.0f)).xyz;
				velocity = mul(gMatInvWorld, float4(outVelocity, 0.0f)).xyz;
			#else
				position = outPosition;
				velocity = outVelocity;
			#endif
#else
				position += (velocity + acceleration * 0.5f) * gDT;
				velocity += acceleration;
#endif
			}

			outPosAndTime.xyz = position;
			outPosAndTime.w = time;
			outVelocityAndTimeScale.xyz = velocity;
			outVelocityAndTimeScale.w = timeScale;			
		}	
	};
};
