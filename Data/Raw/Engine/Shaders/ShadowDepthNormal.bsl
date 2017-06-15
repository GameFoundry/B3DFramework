#define USES_PS
#include "$ENGINE$\ShadowDepthBase.bslinc"

mixin ShadowDepth
{ 
	code
	{
		float4 fsmain(ShadowVStoFS input, out float outDepth : SV_Depth) : SV_Target0
		{
			// Shadow position in clip space, plus bias, scaled to [0, 1] range
			outDepth = saturate(input.shadowPos * gInvDepthRange + gDepthBias);
			return 0;
		}
	};
};