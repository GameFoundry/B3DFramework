shader SpriteLine
{
	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			writemask = RGB;
		};
	};	
	
	depth
	{
		read = false;
		write = false;
	};
	
	raster
	{
		multisample = false; // This controls line rendering algorithm
		lineaa = false;
	};
	
	code
	{
		struct VStoFS
		{
			float4 position : SV_POSITION;
		};

		cbuffer GUIParams
		{
			float4x4 gWorldTransform;
			float gInvViewportWidth;
			float gInvViewportHeight;
			float gViewportYFlip;
			float4 gTint;
		}	
		
		struct VertexInput
		{
			float2 position : POSITION;
		};			
		
		VStoFS vsmain(VertexInput input)
		{
			float4 tfrmdPos = mul(gWorldTransform, float4(input.position, 0, 1));
			
			float tfrmdX = -1.0f + (tfrmdPos.x * gInvViewportWidth);
			float tfrmdY = 1.0f - (tfrmdPos.y * gInvViewportHeight);

			VStoFS output;
			output.position = float4(tfrmdX, tfrmdY, 0, 1);

			return output;
		}

		float4 fsmain(VStoFS input) : SV_Target
		{
			return gTint;
		}
	};
};