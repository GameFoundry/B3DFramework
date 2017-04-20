#include "$ENGINE$\PerCameraData.bslinc"

Parameters =
{
	SamplerCUBE 	gSkySamp : alias("gSkyTex");
	TextureCUBE  	gSkyTex;
};

Technique : inherits("PerCameraData") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = CW;
		CompareFunc = LTE;
		DepthWrite = false;
		
		Vertex =
		{
			void main(
				in float3 inPos : POSITION,
				out float4 oPosition : SV_Position,
				out float3 oDir : TEXCOORD0)
			{
				float4 pos = mul(gMatViewProj, float4(inPos.xyz + gViewOrigin, 1));
			
				// Set Z = W so that final depth is 1.0f and it renders behind everything else
				oPosition = pos.xyww;
				oDir = inPos;
			}
		};
		
		Fragment =
		{
			TextureCube gSkyTex : register(t0);
			SamplerState gSkySamp : register(s0);
		
			cbuffer Params : register(b0)
			{
				float4 gClearColor;
			}
		
			float4 main(
				in float4 inPos : SV_Position, 
				in float3 dir : TEXCOORD0) : SV_Target
			{
				#ifdef SOLID_COLOR
					return gClearColor;
				#else
					return gSkyTex.SampleLevel(gSkySamp, dir, 0);
				#endif
				}
		};	
	};
};