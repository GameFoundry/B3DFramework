#include "$ENGINE$\DeferredLightPass.bslinc"

Technique 
  : inherits("DeferredLightPass") =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthRead = false;
	
		Common = 
		{
			struct VStoFS
			{
				float4 position : SV_POSITION;
				float2 uv0 : TEXCOORD0;
				float3 screenDir : TEXCOORD1;
			};
		};
	
		Vertex =
		{
			struct VertexInput
			{
				float2 screenPos : POSITION;
				float2 uv0 : TEXCOORD0;
			};
			
			VStoFS main(VertexInput input)
			{
				VStoFS output;
			
				output.position = float4(input.screenPos, 0, 1);
				output.uv0 = input.uv0;
				output.screenDir = mul(gMatInvProj, float4(input.screenPos, 1, 0)).xyz - gViewOrigin.xyz;
			
				return output;
			}			
		};
		
		Fragment = 
		{
			float4 main(VStoFS input) : SV_Target0
			{
				GBufferData gBufferData = getGBufferData(input.uv0);

				if(gBufferData.worldNormal.w > 0.0f)
				{
					float3 cameraDir = normalize(input.screenDir);
					float3 worldPosition = input.screenDir * gBufferData.depth + gViewOrigin;
					
					LightData lightData = getLightData();
					return getLighting(worldPosition, input.uv0, gBufferData, lightData);
				}
				else
					return float4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		};
	};
};

Technique 
	: inherits("DeferredLightPass") =
{
	Language = "GLSL";
	
	Pass =
	{
		DepthRead = false;
	
		Vertex =
		{
			layout(location = 0) in vec2 bs_position;
			layout(location = 1) in vec2 bs_texcoord0;
		
			layout(location = 0) out vec4 position;
			layout(location = 1) out vec2 uv0;
			layout(location = 2) out vec3 screenDir;
		
			out gl_PerVertex
			{
				vec4 gl_Position;
			};	
			
			void main()
			{
				position = vec4(bs_position.x, bs_position.y, 0, 1);
				uv0 = bs_texcoord0;
				screenDir = (gMatInvProj * position).xyz - gViewOrigin.xyz;
			
				gl_Position = position;
			}			
		};
		
		Fragment = 
		{
			layout(location = 0) in vec4 position;
			layout(location = 1) in vec2 uv0;
			layout(location = 2) in vec3 screenDir;
		
			layout(location = 0) out vec4 fragColor;
		
			void main()
			{
				GBufferData gBufferData = getGBufferData(uv0);

				if(gBufferData.worldNormal.w > 0.0f)
				{
					vec3 cameraDir = normalize(screenDir);
					vec3 worldPosition = screenDir * gBufferData.depth + gViewOrigin;
					
					LightData lightData = getLightData();
					fragColor = getLighting(worldPosition, uv0, gBufferData, lightData);
				}
				else
					fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		};
	};
};