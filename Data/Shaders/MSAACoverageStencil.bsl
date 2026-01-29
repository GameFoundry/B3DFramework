#include "$ENGINE$\PPBase.bslinc"

shader MSAACoverageStencil
{
	mixin PPBase;

	stencil
	{
		enabled = true;
		reference = 0x80;
		front = { replace, replace, replace, always };
		writemask = 0x80;
	};		
	
	code
	{	
		Texture2D gMSAACoverage;
	
		float fsmain(VStoFS input) : SV_Target0
		{
			int2 pixelPos = (int2)input.uv0.xy;
			float coverage = gMSAACoverage.Load(int3(pixelPos, 0));
			
			// Note: Consider checking 2x2 pixel block and only discard if none require per-sample
			// evaluation. This should speed up HiStencil.
			if(coverage < 0.5f)
				discard;
		
			return 0.0f;			
		}	
	};
};
