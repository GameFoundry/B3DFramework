#include "$ENGINE$\DeferredBasePass.bslinc"

Parameters =
{
	Sampler2D 	gAlbedoSamp : alias("gAlbedoTex");
	Sampler2D 	gNormalSamp : alias("gNormalTex");
	
	Texture2D 	gAlbedoTex;
	Texture2D	gNormalTex = "normal";
};

Technique : base("Surface") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			SamplerState gAlbedoSamp : register(s0);
			SamplerState gNormalSamp : register(s1);
			
			Texture2D gAlbedoTex : register(t0);
			Texture2D gNormalTex : register(t1);
			
			float4 main(
				in VStoFS input, 
				out float4 OutGBufferA : SV_Target1,
				out float4 OutGBufferB : SV_Target2) : SV_Target0
			{
				float3 normal = normalize(gNormalTex.Sample(gNormalSamp, input.uv0) * 2.0f - float3(1, 1, 1));
				float3 worldNormal = calcWorldNormal(input, normal);
			
				GBufferData gbufferData;
				gbufferData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
				gbufferData.worldNormal.xyz = worldNormal;
				
				encodeGBuffer(gbufferData, OutGBufferA, OutGBufferB);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				return float4(gbufferData.albedo.rgb, 1.0f) * 0.01f; 
			}	
		};
	};
};

Technique : base("Surface") =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			uniform sampler2D gAlbedoTex;
			uniform sampler2D gNormalTex;
			
			void main()
			{
				vec3 normal = normalize(texture2D(gNormalTex, uv0).xyz * 2.0f - vec3(1, 1, 1));
				vec3 worldNormal = calcWorldNormal(tangentToWorldZ, tangentToWorldX, normal);
			
				GBufferData gbufferData;
				gbufferData.albedo = texture2D(gAlbedoTex, uv0);
				gbufferData.worldNormal.xyz = worldNormal;
				
				encodeGBuffer(gbufferData, gl_FragData[1], gl_FragData[2]);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				gl_FragData[0] = vec4(gbufferData.albedo.rgb, 1.0f) * 0.01f; 
			}	
		};
	};
};

#include "$ENGINE$\Surface.bslinc"