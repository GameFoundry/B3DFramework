Technique =
{
	Language = "HLSL11";
	
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
		
		Common = 
		{
			struct VStoFS
			{
				float4 position : SV_POSITION;
				float2 screenPos : TEXCOORD1;
				uint lineIdx : BLENDINDICES0;
			};
		};
		
		Vertex =
		{
			float invViewportWidth;
			float invViewportHeight;
			float4x4 worldTransform;

			struct VertexInput
			{
				float2 position : POSITION;
				uint lineIdx : LINEIDX;
			};			
			
			VStoFS main(VertexInput input)
			{
				float4 tfrmdPos = mul(worldTransform, float4(input.position, 0, 1));

				float tfrmdX = -1.0f + (tfrmdPos.x * invViewportWidth);
				float tfrmdY = 1.0f - (tfrmdPos.y * invViewportHeight);

				VStoFS output;
				output.position = float4(tfrmdX, tfrmdY, 0, 1);
				output.screenPos.xy = input.position;
				output.lineIdx = input.lineIdx;
				
				return output;
			}
		};
		
		Fragment =
		{
			StructuredBuffer<float2> linePoints;
			float lineWidth;
			float4 tint;
			
			float getSquaredDistanceToLine(float2 a, float2 b, float2 pt)
			{
				float2 lineDiff = b - a;
				float sqrdLineLength = dot(lineDiff, lineDiff);
				
				float2 pointDiff = pt - a;
				if(sqrdLineLength < 0.0001f) // a == b
					return dot(pointDiff, pointDiff);
				else
				{
					float t = clamp(dot(pointDiff, lineDiff / sqrdLineLength), 0, 1);
					float2 projPoint = a + lineDiff * t;
					float2 toLineDiff = projPoint - pt;
					return dot(toLineDiff, toLineDiff);
				} 
			}

			float4 main(VStoFS input) : SV_Target
			{
				// Get number of lines
				uint numPoints;
				uint dummy;
				linePoints.GetDimensions(numPoints, dummy);
				
				uint numLines = numPoints / 2 - 1;
								
				// Find nearest line
				//// Distance to current line
				int lineIdx = (int)input.lineIdx;
				float2 a = linePoints.Load(lineIdx * 2 + 0);				
				float2 b = linePoints.Load(lineIdx * 2 + 1);
				
				float minSquaredDistance = getSquaredDistanceToLine(a, b, input.screenPos);

				//// Distance to previous line
				int prevLineIdx = max(0, lineIdx - 1);
				a = linePoints.Load(prevLineIdx * 2 + 0);				
				b = linePoints.Load(prevLineIdx * 2 + 1);
				
				minSquaredDistance = min(minSquaredDistance, getSquaredDistanceToLine(a, b, input.screenPos));
				
				//// Distance to next line
				int nextLineIdx = min((int)numPoints - 1, lineIdx + 1);
				a = linePoints.Load(nextLineIdx * 2 + 0);				
				b = linePoints.Load(nextLineIdx * 2 + 1);
				
				minSquaredDistance = min(minSquaredDistance, getSquaredDistanceToLine(a, b, input.screenPos));
				
				// TODO - Use a different filter like Gaussian
				float weight = clamp(minSquaredDistance / (lineWidth * lineWidth), 0, 1);
				return float4(tint.rgb, tint.a * weight);
			}
		};
	};
};

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		// TODO
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		// TODO
	};
};