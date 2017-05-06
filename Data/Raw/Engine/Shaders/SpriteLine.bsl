Parameters =
{
	mat4x4 	worldTransform;
	float	invViewportWidth;
	float	invViewportHeight;

	color	tint;
};

Technique =
{
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};
		
		DepthRead = false;
		DepthWrite = false;
		
		Multisample = false; // This controls line rendering algorithm
		AALine = true;
		
		Common = 
		{
			struct VStoFS
			{
				float4 position : SV_POSITION;
			};
		};
		
		Vertex =
		{
			cbuffer VertParams
			{
				float invViewportWidth;
				float invViewportHeight;
				float4x4 worldTransform;
			};
			
			struct VertexInput
			{
				float2 position : POSITION;
			};			
			
			VStoFS main(VertexInput input)
			{
				float4 tfrmdPos = mul(worldTransform, float4(input.position, 0, 1));
				
				float tfrmdX = -1.0f + (tfrmdPos.x * invViewportWidth);
				float tfrmdY = 1.0f - (tfrmdPos.y * invViewportHeight);

				VStoFS output;
				output.position = float4(tfrmdX, tfrmdY, 0, 1);

				return output;
			}
		};
		
		Fragment =
		{
			cbuffer VertParams
			{
				float4 tint;
			};
			
			float4 main(VStoFS input) : SV_Target
			{
				return tint;
			}
		};
	};
};