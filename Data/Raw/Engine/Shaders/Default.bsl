#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\Surface.bslinc"

Technique : base("Surface") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			void main(
				in VStoFS input, 
				out float4 OutGBufferA : SV_Target0,
				out float4 OutGBufferB : SV_Target1,
				out float2 OutGBufferC : SV_Target2)
			{
				SurfaceData surfaceData;
				surfaceData.albedo = float4(0.05f, 0.05f, 0.05f, 1.0f);
				surfaceData.worldNormal.xyz = input.tangentToWorldZ;
				surfaceData.roughness = 1.0f;
				surfaceData.metalness = 0.0f;
				
				encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC);
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
			layout(location = 0) in vec2 uv0;
			layout(location = 2) in vec3 tangentToWorldZ;
			layout(location = 3) in vec4 tangentToWorldX;	
		
			layout(location = 0) out vec4[3] fragColor;
		
			void main()
			{
				SurfaceData surfaceData;
				surfaceData.albedo = vec4(0.05f, 0.05f, 0.05f, 1.0f);
				surfaceData.worldNormal.xyz = tangentToWorldZ;
				surfaceData.roughness = 1.0f;
				surfaceData.metalness = 0.0f;
				
				encodeGBuffer(surfaceData, fragColor[0], fragColor[1], fragColor[2]);
			}	
		};
	};
};