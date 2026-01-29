#if SOFT
	#define VIEW_DEPTH
#endif

#include "$ENGINE$\ParticleVertex.bslinc"
#include "$ENGINE$\DepthInput.bslinc"

options
{
	transparent = true;
};

shader Surface
{
	mixin ParticleVertex;
	mixin DepthInput;
	
	variations
	{
		SOFT = { false, true };
	};

	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
		};
	};	
	
	depth
	{
		write = false;
	};
	
	code
	{
		[alias(gTexture)]
		SamplerState gSampler;	

		Texture2D gTexture = white;
	
		cbuffer Params
		{
			[hideInInspector]
			float gInvDepthRange = 1.0f;
		};
	
		float4 fsmain(in VStoFS input, in float4 screenPos : SV_Position) : SV_Target0
		{
			float4 output = gTexture.Sample(gSampler, input.uv0) * input.color;
		
			#if SOFT
				float particleDepth = input.depth;
				float sceneDepth = -convertFromDeviceZ(gDepthBufferTex.Load(int3(screenPos.xy, 0)).r);
				
				float diff = sceneDepth - particleDepth;
				float fade = saturate(diff * gInvDepthRange);
			
				output.a *= fade;
			#endif
		
			return output;
		}
	};
};
