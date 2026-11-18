//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
shader SkinCacheDeformation
{
	featureset = HighEnd;

	variations
	{
		SKINNED = { true };
	};

	code
	{
		// Mesh vertices: the stream with the positions, and the stream with the normals, tangents and blend data. The same buffer for
		// a mesh with one stream.
		ByteAddressBuffer gSourcePositions;
		ByteAddressBuffer gSourceTangentFrames;

		// Poses of every entry deformed this frame: three rows of each bone's transform, then the morph shape weights
		ByteAddressBuffer gPoses;

		RWByteAddressBuffer gPositions; // float3 per vertex
		RWByteAddressBuffer gNormalsTangents; // Normal, then tangent with its handedness in W, 8-bit unorm per component

		[internal]
		cbuffer Parameters
		{
			uint gVertexCount;
			uint gPositionStride; // in bytes, in gSourcePositions
			uint gPositionOffset; // in bytes, in a vertex of gSourcePositions
			uint gTangentFrameStride; // in bytes, in gSourceTangentFrames
			uint gNormalOffset; // the offsets below are in bytes, in a vertex of gSourceTangentFrames
			uint gTangentOffset;
			uint gBlendIndicesOffset;
			uint gBlendWeightsOffset;
			uint gBoneOffset; // in bytes, in gPoses
			uint gMorphShapeWeightOffset; // in bytes, in gPoses
		}

		float4 UnpackUnorm4(uint value)
		{
			return float4(value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, value >> 24) / 255.0f;
		}

		// Packs XYZ of a vector in [-1, 1] to 8-bit unorm, as V * 0.5 + 0.5, and @p w to the fourth byte
		uint PackUnitVector(float3 value, uint w)
		{
			uint3 bytes = (uint3)round(saturate(value * 0.5f + 0.5f) * 255.0f);
			return bytes.x | (bytes.y << 8) | (bytes.z << 16) | (w << 24);
		}

		#if SKINNED
		float3x4 LoadBoneMatrix(uint boneIndex)
		{
			uint address = gBoneOffset + boneIndex * 48;
			float4 row0 = asfloat(gPoses.Load4(address));
			float4 row1 = asfloat(gPoses.Load4(address + 16));
			float4 row2 = asfloat(gPoses.Load4(address + 32));

			return float3x4(row0, row1, row2);
		}
		#endif

		[numthreads(64, 1, 1)]
		void csmain(uint3 dispatchThreadId : SV_DispatchThreadID)
		{
			uint vertexIndex = dispatchThreadId.x;
			if(vertexIndex >= gVertexCount)
				return;

			uint tangentFrameAddress = vertexIndex * gTangentFrameStride;
			float3 position = asfloat(gSourcePositions.Load3(vertexIndex * gPositionStride + gPositionOffset));
			uint packedNormal = gSourceTangentFrames.Load(tangentFrameAddress + gNormalOffset);
			uint packedTangent = gSourceTangentFrames.Load(tangentFrameAddress + gTangentOffset);

			float3 normal = UnpackUnorm4(packedNormal).xyz * 2.0f - 1.0f;
			float3 tangent = UnpackUnorm4(packedTangent).xyz * 2.0f - 1.0f;

			#if SKINNED
			uint packedBlendIndices = gSourceTangentFrames.Load(tangentFrameAddress + gBlendIndicesOffset);
			float4 blendWeights = asfloat(gSourceTangentFrames.Load4(tangentFrameAddress + gBlendWeightsOffset));

			float3x4 blendMatrix = blendWeights.x * LoadBoneMatrix(packedBlendIndices & 0xFF);
			blendMatrix += blendWeights.y * LoadBoneMatrix((packedBlendIndices >> 8) & 0xFF);
			blendMatrix += blendWeights.z * LoadBoneMatrix((packedBlendIndices >> 16) & 0xFF);
			blendMatrix += blendWeights.w * LoadBoneMatrix(packedBlendIndices >> 24);

			position = mul(blendMatrix, float4(position, 1.0f));
			normal = mul(blendMatrix, float4(normal, 0.0f));
			tangent = mul(blendMatrix, float4(tangent, 0.0f));
			#endif

			normal = normalize(normal);
			tangent = normalize(tangent);

			gPositions.Store3(vertexIndex * 12, asuint(position));
			gNormalsTangents.Store2(vertexIndex * 8, uint2(PackUnitVector(normal, packedNormal >> 24), PackUnitVector(tangent, packedTangent >> 24)));
		}
	};
};
