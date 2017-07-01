#include "$ENGINE$\PPBase.bslinc"

technique PPBuildHiZ
{
	mixin PPBase;

	code
	{	
		SamplerState gDepthSamp;
		Texture2D gDepthTex;
		
		float fsmain(VStoFS input) : SV_Target0
		{
			float4 depth = gDepthTex.Gather(gDepthSamp, input.uv0);
			return min(min(depth.x, depth.y), min(depth.z, depth.w));
		}	
	};
};