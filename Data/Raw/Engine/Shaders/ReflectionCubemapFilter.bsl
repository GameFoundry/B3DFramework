#include "$ENGINE$\PPBase.bslinc"

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

Technique : inherits("PPBase") =
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
				// Note: This is a rough approximation rather than something physically correct.
				// For a more correct version we should sample along the specular lobe and weight
				// the contributions. But even that wouldn't be fully correct as specular lobe
				// shape changes according to view/normal due to occlusion. So instead we just 
				// approximate everything.
			
				float2 scaledUV = input.uv0 * 2.0f - 1.0f;
				
				float3 dir;
				if(gCubeFace == 0)
					dir = float3(1.0f, -scaledUV.y, -scaledUV.x);
				else if(gCubeFace == 1)
					dir = float3(-1.0f, -scaledUV.y, scaledUV.x);
				else if(gCubeFace == 2)
					dir = float3(scaledUV.x, 1.0f, scaledUV.y);
				else if(gCubeFace == 3)
					dir = float3(scaledUV.x, -1.0f, -scaledUV.y);
				else if(gCubeFace == 4)
					dir = float3(scaledUV.x, -scaledUV.y, 1.0f);
				else
					dir = float3(-scaledUV.x, -scaledUV.y, -1.0f);
				
				return gInputTex.Sample(gInputSamp, dir);
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
				int gCubeFace;
			};
			
			layout(binding = 1) uniform samplerCube gInputTex;
			
			void main()
			{
				// Note: This is a rough approximation rather than something physically correct.
				// For a more correct version we should sample along the specular lobe and weight
				// the contributions. But even that wouldn't be fully correct as specular lobe
				// shape changes according to view/normal due to occlusion. So instead we just 
				// approximate everything.
			
				vec2 scaledUV = FSInput.uv0 * 2.0f - 1.0f;
				
				vec3 dir;
				if(gCubeFace == 0)
					dir = vec3(1.0f, -scaledUV.y, -scaledUV.x);
				else if(gCubeFace == 1)
					dir = vec3(-1.0f, -scaledUV.y, scaledUV.x);
				else if(gCubeFace == 2)
					dir = vec3(scaledUV.x, 1.0f, scaledUV.y);
				else if(gCubeFace == 3)
					dir = vec3(scaledUV.x, -1.0f, -scaledUV.y);
				else if(gCubeFace == 4)
					dir = vec3(scaledUV.x, -scaledUV.y, 1.0f);
				else
					dir = vec3(-scaledUV.x, -scaledUV.y, -1.0f);
				
				fragColor = texture(gInputTex, dir);
			}	
		};
	};
};