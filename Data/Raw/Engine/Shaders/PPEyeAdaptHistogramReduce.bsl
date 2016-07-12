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

Technique =
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

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			// TODO
		};
	};
};