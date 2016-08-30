#include "$ENGINE$\DeferredBasePass.bslinc"

Technique : base("DiffuseCommon") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			float4 main(
				in VStoFS input, 
				out float4 OutGBufferA : SV_Target1,
				out float4 OutGBufferB : SV_Target2) : SV_Target0
			{
				GBufferData gbufferData;
				gbufferData.albedo = float4(0.05f, 0.05f, 0.05f, 1.0f);
				gbufferData.worldNormal.xyz = input.tangentToWorldZ;
				
				encodeGBuffer(gbufferData, OutGBufferA, OutGBufferB);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				return float4(gbufferData.albedo.rgb, 1.0f); 
			}	
		};
	};
};

Technique 
 : inherits("DeferredBasePass")
 : inherits("DiffuseCommon") =
{
	Language = "HLSL11";
};

Technique 
 : inherits("DeferredBasePassSkinned")
 : inherits("DiffuseCommon") =
{
	Language = "HLSL11";
	Tags = { "Animated" };
};

Technique : base("DiffuseCommon") =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			out vec4[3] fragColor;
		
			void main()
			{
				GBufferData gbufferData;
				gbufferData.albedo = vec4(0.05f, 0.05f, 0.05f, 1.0f);
				gbufferData.worldNormal.xyz = tangentToWorldZ;
				
				encodeGBuffer(gbufferData, fragColor[1], fragColor[2]);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				fragColor[0] = vec4(gbufferData.albedo.rgb, 1.0f); 
			}	
		};
	};
};

Technique 
 : inherits("DeferredBasePass")
 : inherits("DiffuseCommon") =
{
	Language = "GLSL";
};

Technique 
 : inherits("DeferredBasePassSkinned")
 : inherits("DiffuseCommon") =
{
	Language = "GLSL";
	Tags = { "Animated" };
};