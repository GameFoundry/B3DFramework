#include "$ENGINE$\DeferredBasePass.bslinc"

Technique : base("Surface") =
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
				SurfaceData surfaceData;
				surfaceData.albedo = float4(0.05f, 0.05f, 0.05f, 1.0f);
				surfaceData.worldNormal.xyz = input.tangentToWorldZ;
				
				encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				return float4(surfaceData.albedo.rgb, 1.0f); 
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
			layout(location = 1) in vec3 tangentToWorldZ;
			layout(location = 2) in vec4 tangentToWorldX;	
		
			layout(location = 0) out vec4[3] fragColor;
		
			void main()
			{
				SurfaceData surfaceData;
				surfaceData.albedo = vec4(0.05f, 0.05f, 0.05f, 1.0f);
				surfaceData.worldNormal.xyz = tangentToWorldZ;
				
				encodeGBuffer(surfaceData, fragColor[1], fragColor[2]);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				fragColor[0] = vec4(surfaceData.albedo.rgb, 1.0f); 
			}	
		};
	};
};

#include "$ENGINE$\Surface.bslinc"