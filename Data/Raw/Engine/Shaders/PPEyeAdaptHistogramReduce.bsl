#include "$ENGINE$\PPBase.bslinc"

Parameters =
{
	Texture2D 	gHistogramTex;
	Texture2D 	gEyeAdaptationTex;
};

Blocks =
{
	Block Input;
};

Technique : inherits("PPBase") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			cbuffer Input
			{
				uint gThreadGroupCount;
			}		
		
			Texture2D gHistogramTex;
			Texture2D gEyeAdaptationTex;

			float4 main(VStoFS input) : SV_Target0
			{
				int2 iUV = trunc(input.uv0);
				float4 outputValue = 0.0f;

				// Output texture only has two rows, store histogram on the first
				if(input.uv0.y < 1.0f)
				{
					// TODO - Potentially optimize using bilinear filtering
					for(uint i = 0; i < gThreadGroupCount; i++)
						outputValue += gHistogramTex.Load(int3(iUV.x, i, 0));

					return outputValue / gThreadGroupCount;
				}
				else
				{
					// Store eye adaptation from last frame in the second row of the texture
					return gEyeAdaptationTex.Load(int3(0, 0, 0)).x;
				}
			}	
		};
	};
};

Technique : inherits("PPBase") =
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
				uint gThreadGroupCount;
			};
		
			layout(binding = 1) uniform sampler2D gHistogramTex;
			layout(binding = 2) uniform sampler2D gEyeAdaptationTex;
			
			void main()
			{
				ivec2 iUV = ivec2(trunc(FSInput.uv0));
				vec4 outputValue = vec4(0.0f);

				// Output texture only has two rows, store histogram on the first
				if(FSInput.uv0.y < 1.0f)
				{
					// TODO - Potentially optimize using bilinear filtering
					for(uint i = 0; i < gThreadGroupCount; i++)
						outputValue += texelFetch(gHistogramTex, ivec2(iUV.x, i), 0);

					fragColor = outputValue / gThreadGroupCount;
				}
				else
				{
					// Store eye adaptation from last frame in the second row of the texture
					fragColor = texelFetch(gEyeAdaptationTex, ivec2(0, 0), 0).xxxx;
				}
			}	
		};
	};
};