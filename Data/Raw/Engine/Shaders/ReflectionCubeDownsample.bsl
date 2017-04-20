#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"

Parameters =
{
	int			gCubeFace;
	SamplerCUBE	gInputSamp : alias("gInputTex");
	TextureCUBE gInputTex;
};

Blocks =
{
	Block Input;
};

Technique 
 : inherits("PPBase")
 : inherits("ReflectionCubemapCommon") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			cbuffer Input
			{
				int gCubeFace;
			}	
		
			SamplerState gInputSamp;
			TextureCube gInputTex;

			float4 main(VStoFS input) : SV_Target0
			{
				float2 scaledUV = input.uv0 * 2.0f - 1.0f;
				float3 dir = getDirFromCubeFace(gCubeFace, scaledUV);
				
				return gInputTex.Sample(gInputSamp, dir);
			}	
		};
	};
};