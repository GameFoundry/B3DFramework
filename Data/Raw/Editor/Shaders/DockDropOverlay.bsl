Parameters =
{
	float	invViewportWidth;
	float	invViewportHeight;
	
	color		tintColor;
	color		highlightColor;
	color		highlightActive;
};

Technique =
{
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthRead = false;
		DepthWrite = false;
		
		Vertex =
		{
			cbuffer Uniforms
			{	
				float invViewportWidth;
				float invViewportHeight;

				float4 tintColor;
				float4 highlightColor;
				float4 highlightActive;
			};

			void main(
			in float2 inPos : POSITION,
			in float4 color : COLOR0,
			out float4 oPosition : SV_Position,
			out float4 oColor : COLOR0)
			{
				float tfrmdX = -1.0f + (inPos.x * invViewportWidth);
				float tfrmdY = 1.0f - (inPos.y * invViewportHeight);

				oPosition = float4(tfrmdX, tfrmdY, 0, 1);

				float4 highlight = highlightActive * color;
				float highlightSum = highlight.x + highlight.y + highlight.z + highlight.a;

				oColor = (1.0f - highlightSum) * tintColor + highlightSum * highlightColor;
			}
		};
		
		Fragment =
		{
			float4 main(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target
			{
				return color;
			}	
		};
	};
};