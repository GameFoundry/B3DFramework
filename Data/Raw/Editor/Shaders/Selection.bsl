Parameters =
{
	mat4x4			matWorldViewProj;
	float4			selColor;
	StructBuffer 	boneMatrices;
};

Technique : base("SelectionBase") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fill = WIRE;
		DepthBias = 0.00001f;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};

		Fragment =
		{
			float4 selColor;

			float4 main(in float4 inPos : SV_Position) : SV_Target
			{
				return selColor;
			}
		};
	};
};

Technique : inherits("SelectionBase") = 
{
	Language = "HLSL11";

	Vertex =
	{
		float4x4 matWorldViewProj;

		void main(
		in float3 inPos : POSITION,
		out float4 oPosition : SV_Position)
		{
			oPosition = mul(matWorldViewProj, float4(inPos.xyz, 1));
		}
	};
};

Technique : inherits("SelectionBase") = 
{
	Language = "HLSL11";
	Tags = { "Animated" };
	
	Vertex =
	{
		struct VertexInput
		{
			float3 position : POSITION;
			uint4 blendIndices : BLENDINDICES;
			float4 blendWeights : BLENDWEIGHT;
			
			#if USE_BLEND_SHAPES
				float3 deltaPosition : POSITION1;
				float4 deltaNormal : NORMAL1;
			#endif
		};	
	
		StructuredBuffer<float4> boneMatrices;
		float4x4 matWorldViewProj;

		float3x4 getBoneMatrix(uint idx)
		{
			float4 row0 = boneMatrices[idx * 3 + 0];
			float4 row1 = boneMatrices[idx * 3 + 1];
			float4 row2 = boneMatrices[idx * 3 + 2];
			
			return float3x4(row0, row1, row2);
		}
		
		float3x4 getBlendMatrix(VertexInput input)
		{
			float3x4 result = input.blendWeights.x * getBoneMatrix(input.blendIndices.x);
			result += input.blendWeights.y * getBoneMatrix(input.blendIndices.y);
			result += input.blendWeights.z * getBoneMatrix(input.blendIndices.z);
			result += input.blendWeights.w * getBoneMatrix(input.blendIndices.w);
			
			return result;
		}		
		
		void main(VertexInput input, out float4 oPosition : SV_Position)
		{
			float3x4 blendMatrix = getBlendMatrix(input);
			float4 position = float4(mul(blendMatrix, float4(input.position, 1.0f)), 1.0f);
			oPosition = mul(matWorldViewProj, position);
		}
	};
};

Technique : base("SelectionBase") =
{
	Language = "GLSL";
	
	Pass =
	{
		Fill = WIRE;
		DepthBias = 0.00001f;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		Fragment =
		{
			uniform vec4 selColor;
			out vec4 fragColor;

			void main()
			{
				fragColor = selColor;
			}
		};
	};
};

Technique : inherits("SelectionBase") = 
{
	Language = "GLSL";

	Vertex =
	{
		uniform mat4 matWorldViewProj;

		in vec3 bs_position;

		out gl_PerVertex
		{
			vec4 gl_Position;
		};
		
		void main()
		{
			gl_Position = matWorldViewProj * vec4(bs_position.xyz, 1);
		}
	};
};

Technique : inherits("SelectionBase") = 
{
	Language = "GLSL";
	Tags = { "Animated" };
	
	Vertex =
	{
		uniform mat4 matWorldViewProj;

		in vec3 bs_position;
	
		in uvec4 bs_blendindices;
		in vec4 bs_blendweights;
			
		#if USE_BLEND_SHAPES
			in vec3 bs_position1;
			in vec4 bs_normal1;
		#endif
		
		uniform samplerBuffer boneMatrices;
		
		out gl_PerVertex
		{
			vec4 gl_Position;
		};
		
		void getBoneMatrix(uint idx, out mat4x3 result)
		{
			mat3x4 temp;
		
			temp[0] = texelFetch(boneMatrices, idx * 3 + 0);
			temp[1] = texelFetch(boneMatrices, idx * 3 + 1);
			temp[2] = texelFetch(boneMatrices, idx * 3 + 2);
			
			result = transpose(temp);				
		}
		
		void getBlendMatrix(out mat4x3 result)
		{
			mat4x3 boneMatrix;
			
			getBoneMatrix(bs_blendindices.x, out boneMatrix);
			result = bs_blendweights.x * boneMatrix;
			
			getBoneMatrix(bs_blendindices.y, out boneMatrix);
			result += bs_blendweights.y * boneMatrix;
			
			getBoneMatrix(bs_blendindices.z, out boneMatrix);
			result += bs_blendweights.z * boneMatrix;
			
			getBoneMatrix(bs_blendindices.w, out boneMatrix);
			result += bs_blendweights.w * boneMatrix;
		}
		
		void main()
		{
			mat3x4 blendMatrix;
			getBlendMatrix(blendMatrix);
			
			vec4 position = vec4(blendMatrix * vec4(bs_position, 1.0f), 1.0f);
			gl_Position = matWorldViewProj * position;
		}
	};
};