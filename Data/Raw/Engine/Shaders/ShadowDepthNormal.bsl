#include "$ENGINE$\ShadowDepthBase.bslinc"

mixin ShadowDepth
{ 
	code
	{
		float4 fsmain(ShadowVStoFS input, float outDepth : SV_Depth)
		{
			outDepth = saturate(-input.shadowPos * gInvDepthRange + gDepthBias);
			return 0;
		}
	};
};