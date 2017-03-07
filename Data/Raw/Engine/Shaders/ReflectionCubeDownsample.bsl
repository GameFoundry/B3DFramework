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

Technique
 : inherits("PPBase")
 : inherits("ReflectionCubemapCommon") =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			in VStoFS
			{
				layout(location = 0) vec2 uv0;
			} FSInput;		
		
			layout(location = 0) out vec4 fragColor;
		
			layout(binding = 0) uniform Input
			{
				int gCubeFace;
			};
			
			layout(binding = 1) uniform samplerCube gInputTex;
			
			void main()
			{
				vec2 scaledUV = FSInput.uv0 * 2.0f - 1.0f;
				vec3 dir = getDirFromCubeFace(gCubeFace, scaledUV);

				fragColor = texture(gInputTex, dir);
			}	
		};
	};
};