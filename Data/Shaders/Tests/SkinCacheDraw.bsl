//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader SkinCacheDraw
{
	depth
	{
		read = false;
		write = false;
	};

	raster
	{
		cull = none;
	};

	code
	{
		struct VertexInput
		{
			float3 position : POSITION;
			float3 normal : NORMAL;
			float4 tangent : TANGENT;
			float2 uv0 : TEXCOORD0;
		};

		struct VStoFS
		{
			float4 position : SV_Position;
			float2 uv0 : TEXCOORD0;
			float frameLength : TEXCOORD1;
		};

		// Places mesh space X in [-0.5, 0.5] and Y in [0, 1] over the whole viewport
		VStoFS vsmain(VertexInput input)
		{
			float3 normal = input.normal * 2.0f - 1.0f;
			float tangentSign = input.tangent.w * 2.0f - 1.0f;

			VStoFS output;
			output.position = float4(input.position.x * 2.0f, input.position.y * 2.0f - 1.0f, 0.5f, 1.0f);
			output.uv0 = input.uv0;
			output.frameLength = saturate(dot(normal, normal)) * abs(tangentSign);

			return output;
		}

		// Red marks the drawn surface, green and blue hold its texture coordinates, and alpha is 1 where the normal and the tangent's
		// handedness have unit length
		float4 fsmain(VStoFS input) : SV_Target0
		{
			return float4(1.0f, input.uv0.x, input.uv0.y, input.frameLength);
		}
	};
};
