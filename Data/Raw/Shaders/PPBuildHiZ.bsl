#include "$ENGINE$\PPBase.bslinc"

shader PPBuildHiZ
{
	mixin PPBase;
	
	code
	{	
		#define NO_TEXTURE_VIEWS 0
			
		#ifdef OPENGL 
		#ifdef __VERSION__
			#if __VERSION__ < 430
				#define NO_TEXTURE_VIEWS 1
			#endif
		#endif
		#endif
	
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
			depth.x = gDepthTex.Sample(gDepthSamp, input.uv0, int2(1, 1));
			depth.y = gDepthTex.Sample(gDepthSamp, input.uv0, int2(-1, 1));
			depth.z = gDepthTex.Sample(gDepthSamp, input.uv0, int2(1, -1));
			depth.w = gDepthTex.Sample(gDepthSamp, input.uv0, int2(-1, -1));
#else
			float4 depth = gDepthTex.Gather(gDepthSamp, input.uv0);
#endif
			
			return min(min(depth.x, depth.y), min(depth.z, depth.w));
		}	
	};
};