#include "$ENGINE$\ParticleVertex.bslinc"

options
{
	transparent = true;
};

shader Surface
{
	mixin ParticleVertex;

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
	
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			return gTexture.Sample(gSampler, input.uv0);
		}	
	};
};