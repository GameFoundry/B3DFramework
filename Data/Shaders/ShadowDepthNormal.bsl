#define USES_PS
#include "$ENGINE$\ShadowDepthBase.bslinc"

shader ShadowDepth
{ 
	mixin ShadowDepthBase;

	code
	{
		void fsmain(ShadowVStoFS input, out float outDepth : SV_Depth)
		{
			// Shadow position in clip space, plus bias, scaled to [0, 1] range
			outDepth = saturate(input.shadowPos * gInvDepthRange + gDepthBias);
		}
	};
};
