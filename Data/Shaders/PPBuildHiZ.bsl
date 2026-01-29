#include "$ENGINE$\PPBase.bslinc"

shader PPBuildHiZ
{
	mixin PPBase;
	
	variations
	{
		NO_TEXTURE_VIEWS = { true, false };
	};
	
	code
	{	
		SamplerState gDepthSamp;
		Texture2D gDepthTex;
		
		// Older OpenGL versions don't support views, meaning we need to specify mip level explicitly
#if NO_TEXTURE_VIEWS
		[internal]
		cbuffer Input
		{
			float2 gHalfPixelOffset;
			int gMipLevel;
		}
#endif
		
		float fsmain(VStoFS input) : SV_Target0
		{
#if NO_TEXTURE_VIEWS 	
			float4 depth;
			
			float2 uv = input.uv0 - gHalfPixelOffset;
			
			depth.x = gDepthTex.SampleLevel(gDepthSamp, uv, gMipLevel, int2(0, 0));
			depth.y = gDepthTex.SampleLevel(gDepthSamp, uv, gMipLevel, int2(1, 0));
			depth.z = gDepthTex.SampleLevel(gDepthSamp, uv, gMipLevel, int2(1, 1));
			depth.w = gDepthTex.SampleLevel(gDepthSamp, uv, gMipLevel, int2(0, 1));
#else
			float4 depth = gDepthTex.Gather(gDepthSamp, input.uv0);
#endif
			
			return min(min(depth.x, depth.y), min(depth.z, depth.w));
		}	
	};
};
