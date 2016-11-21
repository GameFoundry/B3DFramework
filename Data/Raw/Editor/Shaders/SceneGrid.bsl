Priority = -10000;

Parameters =
{
	mat4x4		matViewProj;
	float4		worldCameraPos;
	color		gridColor;
	float3		gridPlaneNormal;
	float		gridSpacing;
	float		gridBorderWidth;
	float		gridFadeOutStart;
	float		gridFadeOutEnd;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthWrite = false;
		Cull = NOCULL;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};

		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				out float4 oPosition : SV_Position,
				out float3 oWorldPos : TEXCOORD0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oWorldPos = inPos;
			}
		};
		
		Fragment =
		{
			float4 worldCameraPos;
			float gridSpacing;
			float gridBorderWidth;
			float4 gridColor;
			float gridFadeOutStart;
			float gridFadeOutEnd;
			float3 gridPlaneNormal; // Must be one of the basis vectors

			float3 sampleGrid(float3 position)
			{
				float3 count = round(position / gridSpacing);
				
				// Adding grid plane normal ensures that the remainer for the unused axis is not zero, otherwise
				// the sampled value would always be 1.
				float3 remainder = abs(position - count * gridSpacing) + gridPlaneNormal;

				float3 derivX = ddx(position);
				float3 derivY = ddy(position);
				float3 gradientLength = float3(length(float2(derivX.x, derivY.x)), 
					length(float2(derivX.y, derivY.y)), length(float2(derivX.z, derivY.z)));

				return 1.0f - smoothstep(gridBorderWidth, gridBorderWidth + gradientLength, remainder);
			}

			float4 main(in float4 inPos : SV_Position, in float3 inWorldPos : TEXCOORD0) : SV_Target
			{
				float3 sampValue = sampleGrid(inWorldPos);

				float4 outColor = gridColor;
				outColor.a = max(max(sampValue.x, sampValue.y), sampValue.z) * 
					smoothstep(gridFadeOutEnd, gridFadeOutStart, length(inWorldPos - worldCameraPos));
				
				return outColor;
			}
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		DepthWrite = false;
		Cull = NOCULL;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};

		Vertex =
		{
			layout(location = 0) in vec3 bs_position;
			layout(location = 0) out vec3 worldPos;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
		
			layout(binding = 0) uniform VertUBO
			{
				mat4 matViewProj;
			};
			
			void main()
			{
				vec4 outPos = matViewProj * vec4(bs_position.xyz, 1);
				worldPos = bs_position;
				gl_Position = outPos;
			}
		};
		
		Fragment =
		{
			layout(location = 0) in vec3 worldPos;
			layout(location = 0) out vec4 fragColor;
		
			layout(binding = 1) uniform FragUBO
			{
				vec4 worldCameraPos;
				float gridSpacing;
				float gridBorderWidth;
				vec4 gridColor;
				float gridFadeOutStart;
				float gridFadeOutEnd;
				vec3 gridPlaneNormal; // Must be one of the basis vectors
			};
		
			vec3 sampleGrid(vec3 position)
			{
				vec3 count = round(position / vec3(gridSpacing, gridSpacing, gridSpacing));
				
				// Adding grid plane normal ensures that the remainer for the unused axis is not zero, otherwise
				// the sampled value would always be 1.
				vec3 remainder = abs(position - count * vec3(gridSpacing, gridSpacing, gridSpacing)) + gridPlaneNormal;

				vec3 derivX = dFdx(position);
				vec3 derivY = dFdy(position);
				vec3 gradientLength = vec3(length(vec2(derivX.x, derivY.x)), length(vec2(derivX.y, derivY.y)), length(vec2(derivX.z, derivY.z)));

				vec3 vecBorderWidth = vec3(gridBorderWidth, gridBorderWidth, gridBorderWidth);
				return 1.0f - smoothstep(vecBorderWidth, vecBorderWidth + gradientLength, remainder);
			}

			void main()
			{
				vec3 sampValue = sampleGrid(worldPos);

				vec4 outColor = gridColor;
				outColor.a = max(max(sampValue.x, sampValue.y), sampValue.z) * 
					smoothstep(gridFadeOutEnd, gridFadeOutStart, length(worldPos - worldCameraPos.xyz));
				
				fragColor = outColor;
			}
		};
	};
};

