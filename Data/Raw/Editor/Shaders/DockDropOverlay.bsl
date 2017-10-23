technique DockDropOverlay
{
	blend
	{
		target
		{
			enabled = true;
			color = { srcA, srcIA, add };
		};
	};

	depth
	{
		read = false;
		write = false;
	};
	
	code
	{
		cbuffer Uniforms
		{	
			float invViewportWidth;
			float invViewportHeight;

			[color]	float4 tintColor;
			[color]	float4 highlightColor;
			[color]	float4 highlightActive;
		};

		void vsmain(
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

		float4 fsmain(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target
		{
			return color;
		}	
	};
};
