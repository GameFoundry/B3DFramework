#include "$ENGINE$\ShadowDepthBase.bslinc"

shader ShadowDepth
{ 
	mixin ShadowDepthBase;

	code
	{
		// Despite the shader name technically we do have a PS, but it doesn’t
		// do anything. It’s here just because macOS OpenGL implementation 
		// complains when using a pipeline without one.
		float4 fsmain(ShadowVStoFS input) : SV_Target0
		{
			return 0;
		}
	};
};
