//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererMeshDataTestSuite.h"
#include "GpuBackend/B3DSubMesh.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Image/B3DColor.h"
#include "Image/B3DPixelUtility.h"
#include "Math/B3DRandom.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Mesh/B3DMesh.h"
#include "Mesh/B3DMeshData.h"
#include "Particles/B3DParticleEmitter.h"
#include "Particles/B3DParticleModule.h"
#include "Physics/B3DPhysicsMesh.h"
#include "Private/Particles/B3DParticleSet.h"
#include "Renderer/B3DRendererMeshData.h"

using namespace b3d;

namespace
{
	constexpr u32 kVertexCount = 8;
	constexpr u32 kIndexCount = 36;

	/** Largest difference between a normal or tangent component and its value after packing into 8 bits. */
	constexpr float kPackedNormalTolerance = 1.0f / 127.0f;

	/** How the attributes of a test mesh are placed in vertex streams. */
	enum class StreamLayout
	{
		/** Every attribute in stream 0. */
		Interleaved,

		/** Positions in stream 0; normals, tangents and blend data in stream 1; texture coordinates and colors in stream 2. */
		Split
	};

	/** Format of the normals and tangents of a test mesh. */
	enum class NormalFormat
	{
		/** Packed into 8 bits per component, as renderer mesh data stores them. */
		Packed,

		/** 32-bit floats. */
		Float
	};

	/** Attributes of the cube at the origin with sides of length one, one vertex per corner. */
	struct CubeData
	{
		CubeData()
		{
			for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
			{
				const Vector3 position((vertexIndex & 1) != 0 ? 0.5f : -0.5f, (vertexIndex & 2) != 0 ? 0.5f : -0.5f, (vertexIndex & 4) != 0 ? 0.5f : -0.5f);
				const Vector3 tangent = Vector3::Normalize(Vector3(-position.Z, 0.0f, position.X));

				Positions[vertexIndex] = position;
				Normals[vertexIndex] = Vector3::Normalize(position);
				Tangents[vertexIndex] = Vector4(tangent.X, tangent.Y, tangent.Z, (vertexIndex & 1) != 0 ? 1.0f : -1.0f);
				Uv0[vertexIndex] = Vector2(position.X + 0.5f, position.Y + 0.5f);
				Uv1[vertexIndex] = Vector2(0.25f * (float)vertexIndex, 0.125f * (float)vertexIndex);
				Colors[vertexIndex] = Color((float)(vertexIndex * 32) / 255.0f, (float)(255 - vertexIndex * 32) / 255.0f, 128.0f / 255.0f, 1.0f);
				PixelUtility::PackColor(Colors[vertexIndex], PF_RGBA8, &PackedColors[vertexIndex]);

				BoneWeight& boneWeight = BoneWeights[vertexIndex];
				boneWeight.Index0 = (int)(vertexIndex % 3);
				boneWeight.Index1 = (int)((vertexIndex + 1) % 3);
				boneWeight.Index2 = 3;
				boneWeight.Index3 = 4;
				boneWeight.Weight0 = 0.5f;
				boneWeight.Weight1 = 0.25f;
				boneWeight.Weight2 = 0.125f;
				boneWeight.Weight3 = 0.125f;

				BlendIndices[vertexIndex] = (u32)boneWeight.Index0 | ((u32)boneWeight.Index1 << 8) | ((u32)boneWeight.Index2 << 16) | ((u32)boneWeight.Index3 << 24);
				BlendWeights[vertexIndex] = Vector4(boneWeight.Weight0, boneWeight.Weight1, boneWeight.Weight2, boneWeight.Weight3);
			}

			// Two triangles for each side: the four corners whose coordinate on the side's axis is the side's
			u32 index = 0;
			for(u32 axis = 0; axis < 3; axis++)
			{
				const u32 axisBit = 1 << axis;
				const u32 firstBit = 1 << ((axis + 1) % 3);
				const u32 secondBit = 1 << ((axis + 2) % 3);

				for(u32 side = 0; side < 2; side++)
				{
					const u32 corner = side != 0 ? axisBit : 0;

					Indices[index++] = corner;
					Indices[index++] = corner | firstBit;
					Indices[index++] = corner | firstBit | secondBit;
					Indices[index++] = corner;
					Indices[index++] = corner | firstBit | secondBit;
					Indices[index++] = corner | secondBit;
				}
			}
		}

		Vector3 Positions[kVertexCount];
		Vector3 Normals[kVertexCount];
		Vector4 Tangents[kVertexCount];
		Vector2 Uv0[kVertexCount];
		Vector2 Uv1[kVertexCount];
		Color Colors[kVertexCount];
		u32 PackedColors[kVertexCount];
		BoneWeight BoneWeights[kVertexCount];
		u32 BlendIndices[kVertexCount];
		Vector4 BlendWeights[kVertexCount];
		u32 Indices[kIndexCount];
	};

	/** Returns the stream @p layout places the attribute with @p semantic in. */
	u32 GetStreamIndex(StreamLayout layout, VertexElementSemantic semantic)
	{
		if(layout == StreamLayout::Interleaved)
			return 0;

		switch(semantic)
		{
		case VES_POSITION:
			return 0;
		case VES_NORMAL:
		case VES_TANGENT:
		case VES_BLEND_INDICES:
		case VES_BLEND_WEIGHTS:
			return 1;
		default:
			return 2;
		}
	}

	/**
	 * Creates the description of a test mesh with the attributes in @p attributes (a combination of VertexLayout flags), in the streams
	 * of @p layout, with normals and tangents in @p normalFormat.
	 */
	TShared<VertexDescription> CreateVertexDescription(StreamLayout layout, u32 attributes, NormalFormat normalFormat)
	{
		const bool packedNormals = normalFormat == NormalFormat::Packed;

		TInlineArray<VertexElement, 8> elements;
		if((attributes & (u32)VertexLayout::Position) != 0)
			elements.Add(VertexElement(VET_FLOAT3, VES_POSITION, 0, GetStreamIndex(layout, VES_POSITION)));

		if((attributes & (u32)VertexLayout::Normal) != 0)
			elements.Add(VertexElement(packedNormals ? VET_UBYTE4_NORM : VET_FLOAT3, VES_NORMAL, 0, GetStreamIndex(layout, VES_NORMAL)));

		if((attributes & (u32)VertexLayout::Tangent) != 0)
			elements.Add(VertexElement(packedNormals ? VET_UBYTE4_NORM : VET_FLOAT4, VES_TANGENT, 0, GetStreamIndex(layout, VES_TANGENT)));

		if((attributes & (u32)VertexLayout::BoneWeights) != 0)
		{
			elements.Add(VertexElement(VET_UBYTE4, VES_BLEND_INDICES, 0, GetStreamIndex(layout, VES_BLEND_INDICES)));
			elements.Add(VertexElement(VET_FLOAT4, VES_BLEND_WEIGHTS, 0, GetStreamIndex(layout, VES_BLEND_WEIGHTS)));
		}

		if((attributes & (u32)VertexLayout::UV0) != 0)
			elements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 0, GetStreamIndex(layout, VES_TEXCOORD)));

		if((attributes & (u32)VertexLayout::UV1) != 0)
			elements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD, 1, GetStreamIndex(layout, VES_TEXCOORD)));

		if((attributes & (u32)VertexLayout::Color) != 0)
			elements.Add(VertexElement(VET_COLOR, VES_COLOR, 0, GetStreamIndex(layout, VES_COLOR)));

		return B3DMakeShared<VertexDescription>(elements);
	}

	/** Every attribute renderer mesh data supports. */
	constexpr u32 kAllAttributes = (u32)VertexLayout::Position | (u32)VertexLayout::Normal | (u32)VertexLayout::Tangent | (u32)VertexLayout::UV0 |
		(u32)VertexLayout::UV1 | (u32)VertexLayout::Color | (u32)VertexLayout::BoneWeights;

	/**
	 * Creates mesh data of the cube with @p vertexDescription, writing each attribute it declares straight into its stream, and the cube's
	 * indices. Normals and tangents are written as floats; the description must declare them as floats, if at all. Every position is
	 * moved by @p offset.
	 */
	TShared<MeshData> CreateCubeMeshData(const CubeData& cube, const TShared<VertexDescription>& vertexDescription, const Vector3& offset)
	{
		TShared<MeshData> meshData = MeshData::Create(kVertexCount, kIndexCount, vertexDescription);

		Vector3 positions[kVertexCount];
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
			positions[vertexIndex] = cube.Positions[vertexIndex] + offset;

		const auto fnWriteElement = [&meshData, &vertexDescription](VertexElementSemantic semantic, u32 semanticIndex, const void* data, u32 size)
		{
			const VertexElement* element = vertexDescription->FindElement(semantic, semanticIndex);
			if(element != nullptr)
				meshData->SetVertexData(semantic, const_cast<void*>(data), size, semanticIndex, element->GetStreamIndex());
		};

		fnWriteElement(VES_POSITION, 0, positions, sizeof(positions));
		fnWriteElement(VES_NORMAL, 0, cube.Normals, sizeof(cube.Normals));
		fnWriteElement(VES_TANGENT, 0, cube.Tangents, sizeof(cube.Tangents));
		fnWriteElement(VES_BLEND_INDICES, 0, cube.BlendIndices, sizeof(cube.BlendIndices));
		fnWriteElement(VES_BLEND_WEIGHTS, 0, cube.BlendWeights, sizeof(cube.BlendWeights));
		fnWriteElement(VES_TEXCOORD, 0, cube.Uv0, sizeof(cube.Uv0));
		fnWriteElement(VES_TEXCOORD, 1, cube.Uv1, sizeof(cube.Uv1));
		fnWriteElement(VES_COLOR, 0, cube.PackedColors, sizeof(cube.PackedColors));

		memcpy(meshData->GetIndices32(), cube.Indices, sizeof(cube.Indices));
		return meshData;
	}

	/** Checks that every component of @p a is within @p tolerance of the same component of @p b. */
	bool IsNear(const Vector3& a, const Vector3& b, float tolerance)
	{
		return Math::Abs(a.X - b.X) <= tolerance && Math::Abs(a.Y - b.Y) <= tolerance && Math::Abs(a.Z - b.Z) <= tolerance;
	}

	/** Checks that every component of @p a is within @p tolerance of the same component of @p b. */
	bool IsNear(const Vector4& a, const Vector4& b, float tolerance)
	{
		return Math::Abs(a.X - b.X) <= tolerance && Math::Abs(a.Y - b.Y) <= tolerance && Math::Abs(a.Z - b.Z) <= tolerance &&
			Math::Abs(a.W - b.W) <= tolerance;
	}

	/**
	 * Checks that @p a and @p b differ by at most one step of 8 bits per channel. Storing a color in 8 bits per channel and reading it
	 * back can round to the next step.
	 */
	bool IsNear(const Color& a, const Color& b)
	{
		constexpr float kTolerance = 1.01f / 255.0f;
		return Math::Abs(a.R - b.R) <= kTolerance && Math::Abs(a.G - b.G) <= kTolerance && Math::Abs(a.B - b.B) <= kTolerance &&
			Math::Abs(a.A - b.A) <= kTolerance;
	}

	/** Checks that @p a and @p b have the same bone indices and weights. */
	bool IsEqual(const BoneWeight& a, const BoneWeight& b)
	{
		return a.Index0 == b.Index0 && a.Index1 == b.Index1 && a.Index2 == b.Index2 && a.Index3 == b.Index3 && a.Weight0 == b.Weight0 &&
			a.Weight1 == b.Weight1 && a.Weight2 == b.Weight2 && a.Weight3 == b.Weight3;
	}

	/**
	 * Checks that the positions of @p meshData are the cube's corners: each of them is within @p tolerance of a corner, and each corner is
	 * within @p tolerance of one of them.
	 */
	bool HasCubeCorners(const CubeData& cube, MeshData& meshData, float tolerance)
	{
		const u32 vertexCount = meshData.GetVertexCount();
		Vector<Vector3> positions(vertexCount, Vector3::kZero);
		meshData.GetVertexData(VES_POSITION, positions.data(), vertexCount * sizeof(Vector3));

		for(const Vector3& position : positions)
		{
			bool isCorner = false;
			for(const Vector3& corner : cube.Positions)
				isCorner = isCorner || IsNear(position, corner, tolerance);

			if(!isCorner)
				return false;
		}

		for(const Vector3& corner : cube.Positions)
		{
			bool isPresent = false;
			for(const Vector3& position : positions)
				isPresent = isPresent || IsNear(position, corner, tolerance);

			if(!isPresent)
				return false;
		}

		return true;
	}
} // namespace

RendererMeshDataTestSuite::RendererMeshDataTestSuite()
	: TestSuite("RendererMeshDataTestSuite")
{
	B3D_ADD_TEST(RendererMeshDataTestSuite::TestVertexLayout)
	B3D_ADD_TEST(RendererMeshDataTestSuite::TestAccessors)
	B3D_ADD_TEST(RendererMeshDataTestSuite::TestConvert)
	B3D_ADD_TEST(RendererMeshDataTestSuite::TestCombine)
	B3D_ADD_TEST(RendererMeshDataTestSuite::TestPhysicsMesh)
	B3D_ADD_TEST(RendererMeshDataTestSuite::TestParticleEmitter)
}

void RendererMeshDataTestSuite::TestVertexLayout()
{
	// Stream of each attribute, or -1 if the layout doesn't have it
	struct ExpectedStreams
	{
		ExpectedStreams(u32 layout, i32 position, i32 tangentBasis, i32 surface, u32 streamCount)
			: Layout(layout), Position(position), TangentBasis(tangentBasis), Surface(surface), StreamCount(streamCount)
		{}

		u32 Layout;
		i32 Position;
		i32 TangentBasis; /**< Normals, tangents, blend indices and blend weights. */
		i32 Surface; /**< Texture coordinates and colors. */
		u32 StreamCount;
	};

	const ExpectedStreams cases[] = {
		ExpectedStreams(kAllAttributes, 0, 1, 2, 3),
		ExpectedStreams((u32)VertexLayout::PNTU, 0, 1, 2, 3),
		ExpectedStreams((u32)VertexLayout::Position | (u32)VertexLayout::BoneWeights | (u32)VertexLayout::UV0, 0, 1, 2, 3),
		ExpectedStreams((u32)VertexLayout::PN, 0, 1, -1, 2),
		ExpectedStreams((u32)VertexLayout::PCU, 0, -1, 1, 2),
		ExpectedStreams((u32)VertexLayout::Position, 0, -1, -1, 1),
	};

	for(const ExpectedStreams& expected : cases)
	{
		const TShared<VertexDescription> description = RendererMeshData::VertexLayoutVertexDesc((VertexLayout)expected.Layout);

		// Every stream up to the last holds an element
		bool streamUsed[3] = { false, false, false };
		bool streamsMatch = true;
		for(const VertexElement& element : description->GetElements())
		{
			streamsMatch = streamsMatch && element.GetStreamIndex() < expected.StreamCount;
			if(element.GetStreamIndex() < 3)
				streamUsed[element.GetStreamIndex()] = true;
		}

		for(u32 streamIndex = 0; streamIndex < expected.StreamCount; streamIndex++)
			streamsMatch = streamsMatch && streamUsed[streamIndex];

		for(const VertexElement& element : description->GetElements())
		{
			i32 expectedStream = expected.Surface;
			if(element.GetSemantic() == VES_POSITION)
				expectedStream = expected.Position;
			else if(element.GetSemantic() == VES_NORMAL || element.GetSemantic() == VES_TANGENT || element.GetSemantic() == VES_BLEND_INDICES ||
				element.GetSemantic() == VES_BLEND_WEIGHTS)
			{
				expectedStream = expected.TangentBasis;
			}

			streamsMatch = streamsMatch && (i32)element.GetStreamIndex() == expectedStream;
		}

		B3D_TEST_ASSERT_MSG(streamsMatch, StringUtility::Format("Vertex layout {0} doesn't place its attributes in the expected streams.", expected.Layout))
	}

	// Strides of the full layout: position; normal, tangent and blend data; two sets of texture coordinates and a color
	const TShared<VertexDescription> fullDescription = RendererMeshData::VertexLayoutVertexDesc((VertexLayout)kAllAttributes);
	B3D_TEST_ASSERT(fullDescription->GetVertexStride(0) == 12 && fullDescription->GetVertexStride(1) == 28 && fullDescription->GetVertexStride(2) == 20)
}

void RendererMeshDataTestSuite::TestAccessors()
{
	CubeData cube;

	for(StreamLayout layout : { StreamLayout::Interleaved, StreamLayout::Split })
	{
		const TShared<VertexDescription> vertexDescription = CreateVertexDescription(layout, kAllAttributes, NormalFormat::Packed);
		const TShared<RendererMeshData> meshData = RendererMeshData::Create(MeshData::Create(kVertexCount, kIndexCount, vertexDescription));

		meshData->SetPositions(cube.Positions, sizeof(cube.Positions));
		meshData->SetNormals(cube.Normals, sizeof(cube.Normals));
		meshData->SetTangents(cube.Tangents, sizeof(cube.Tangents));
		meshData->SetUV0(cube.Uv0, sizeof(cube.Uv0));
		meshData->SetUV1(cube.Uv1, sizeof(cube.Uv1));
		meshData->SetColors(cube.Colors, sizeof(cube.Colors));
		meshData->SetBoneWeights(cube.BoneWeights, sizeof(cube.BoneWeights));
		meshData->SetIndices(cube.Indices, sizeof(cube.Indices));

		Vector<Vector3> positions(kVertexCount, Vector3::kZero);
		Vector<Vector3> normals(kVertexCount, Vector3::kZero);
		Vector<Vector4> tangents(kVertexCount, Vector4::kZero);
		Vector<Vector2> uv0(kVertexCount, Vector2::kZero);
		Vector<Vector2> uv1(kVertexCount, Vector2::kZero);
		Vector<Color> colors(kVertexCount, Color::kBlack);
		Vector<BoneWeight> boneWeights(kVertexCount, BoneWeight());
		Vector<u32> indices(kIndexCount, 0);

		meshData->GetPositions(positions.data(), kVertexCount * sizeof(Vector3));
		meshData->GetNormals(normals.data(), kVertexCount * sizeof(Vector3));
		meshData->GetTangents(tangents.data(), kVertexCount * sizeof(Vector4));
		meshData->GetUV0(uv0.data(), kVertexCount * sizeof(Vector2));
		meshData->GetUV1(uv1.data(), kVertexCount * sizeof(Vector2));
		meshData->GetColors(colors.data(), kVertexCount * sizeof(Color));
		meshData->GetBoneWeights(boneWeights.data(), kVertexCount * sizeof(BoneWeight));
		meshData->GetIndices(indices.data(), kIndexCount * sizeof(u32));

		bool positionsEqual = true;
		bool normalsNear = true;
		bool tangentsNear = true;
		bool uvsEqual = true;
		bool colorsEqual = true;
		bool boneWeightsEqual = true;
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
		{
			positionsEqual = positionsEqual && positions[vertexIndex] == cube.Positions[vertexIndex];
			normalsNear = normalsNear && IsNear(normals[vertexIndex], cube.Normals[vertexIndex], kPackedNormalTolerance);
			tangentsNear = tangentsNear && IsNear(tangents[vertexIndex], cube.Tangents[vertexIndex], kPackedNormalTolerance) &&
				(tangents[vertexIndex].W > 0.0f) == (cube.Tangents[vertexIndex].W > 0.0f);
			uvsEqual = uvsEqual && uv0[vertexIndex] == cube.Uv0[vertexIndex] && uv1[vertexIndex] == cube.Uv1[vertexIndex];
			colorsEqual = colorsEqual && IsNear(colors[vertexIndex], cube.Colors[vertexIndex]);
			boneWeightsEqual = boneWeightsEqual && IsEqual(boneWeights[vertexIndex], cube.BoneWeights[vertexIndex]);
		}

		B3D_TEST_ASSERT_MSG(positionsEqual, "Positions differ after a round trip through the accessors.")
		B3D_TEST_ASSERT_MSG(normalsNear, "Normals differ by more than the packing tolerance after a round trip through the accessors.")
		B3D_TEST_ASSERT_MSG(tangentsNear, "Tangents differ by more than the packing tolerance, or lost their handedness, after a round trip through the accessors.")
		B3D_TEST_ASSERT_MSG(uvsEqual, "Texture coordinates differ after a round trip through the accessors.")
		B3D_TEST_ASSERT_MSG(colorsEqual, "Colors differ by more than one 8-bit step after a round trip through the accessors.")
		B3D_TEST_ASSERT_MSG(boneWeightsEqual, "Bone weights differ after a round trip through the accessors.")
		B3D_TEST_ASSERT_MSG(memcmp(indices.data(), cube.Indices, sizeof(cube.Indices)) == 0, "Indices differ after a round trip through the accessors.")

		// The setters wrote each attribute into the stream the layout declares it in
		const TShared<MeshData> vertexData = meshData->GetData();

		Vector<Vector3> streamPositions(kVertexCount, Vector3::kZero);
		Vector<Vector4> streamWeights(kVertexCount, Vector4::kZero);
		Vector<Vector2> streamUv1(kVertexCount, Vector2::kZero);
		vertexData->GetVertexData(VES_POSITION, streamPositions.data(), kVertexCount * sizeof(Vector3), 0, GetStreamIndex(layout, VES_POSITION));
		vertexData->GetVertexData(VES_BLEND_WEIGHTS, streamWeights.data(), kVertexCount * sizeof(Vector4), 0, GetStreamIndex(layout, VES_BLEND_WEIGHTS));
		vertexData->GetVertexData(VES_TEXCOORD, streamUv1.data(), kVertexCount * sizeof(Vector2), 1, GetStreamIndex(layout, VES_TEXCOORD));

		bool streamsMatch = true;
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
		{
			streamsMatch = streamsMatch && streamPositions[vertexIndex] == cube.Positions[vertexIndex] &&
				streamWeights[vertexIndex] == cube.BlendWeights[vertexIndex] && streamUv1[vertexIndex] == cube.Uv1[vertexIndex];
		}

		B3D_TEST_ASSERT_MSG(streamsMatch, "An attribute written through the accessors is not in the stream its layout declares it in.")

		// Colors written as packed values read back as the same colors
		const TShared<RendererMeshData> packedColorMeshData = RendererMeshData::Create(MeshData::Create(kVertexCount, kIndexCount, vertexDescription));
		packedColorMeshData->SetColors(cube.PackedColors, sizeof(cube.PackedColors));

		Vector<Color> packedColors(kVertexCount, Color::kBlack);
		packedColorMeshData->GetColors(packedColors.data(), kVertexCount * sizeof(Color));

		bool packedColorsEqual = true;
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
			packedColorsEqual = packedColorsEqual && IsNear(packedColors[vertexIndex], cube.Colors[vertexIndex]);

		B3D_TEST_ASSERT_MSG(packedColorsEqual, "Colors written as packed values differ by more than one 8-bit step when read back.")
	}
}

void RendererMeshDataTestSuite::TestConvert()
{
	CubeData cube;

	const TShared<VertexDescription> vertexDescription = CreateVertexDescription(StreamLayout::Split, kAllAttributes, NormalFormat::Float);
	const TShared<MeshData> converted = RendererMeshData::Convert(CreateCubeMeshData(cube, vertexDescription, Vector3::kZero));
	B3D_TEST_ASSERT(converted != nullptr && converted->GetVertexCount() == kVertexCount && converted->GetIndexCount() == kIndexCount)

	const TShared<VertexDescription> convertedDescription = converted->GetVertexDescription();
	const VertexElement* normalElement = convertedDescription->FindElement(VES_NORMAL);
	const VertexElement* tangentElement = convertedDescription->FindElement(VES_TANGENT);
	B3D_TEST_ASSERT(normalElement != nullptr && normalElement->GetType() == VET_UBYTE4_NORM)
	B3D_TEST_ASSERT(tangentElement != nullptr && tangentElement->GetType() == VET_UBYTE4_NORM)

	const TShared<RendererMeshData> meshData = RendererMeshData::Create(converted);

	Vector<Vector3> positions(kVertexCount, Vector3::kZero);
	Vector<Vector3> normals(kVertexCount, Vector3::kZero);
	Vector<Vector4> tangents(kVertexCount, Vector4::kZero);
	Vector<Vector2> uv0(kVertexCount, Vector2::kZero);
	Vector<Vector2> uv1(kVertexCount, Vector2::kZero);
	Vector<Color> colors(kVertexCount, Color::kBlack);
	Vector<BoneWeight> boneWeights(kVertexCount, BoneWeight());
	Vector<u32> indices(kIndexCount, 0);

	meshData->GetPositions(positions.data(), kVertexCount * sizeof(Vector3));
	meshData->GetNormals(normals.data(), kVertexCount * sizeof(Vector3));
	meshData->GetTangents(tangents.data(), kVertexCount * sizeof(Vector4));
	meshData->GetUV0(uv0.data(), kVertexCount * sizeof(Vector2));
	meshData->GetUV1(uv1.data(), kVertexCount * sizeof(Vector2));
	meshData->GetColors(colors.data(), kVertexCount * sizeof(Color));
	meshData->GetBoneWeights(boneWeights.data(), kVertexCount * sizeof(BoneWeight));
	meshData->GetIndices(indices.data(), kIndexCount * sizeof(u32));

	bool allMatch = true;
	for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
	{
		allMatch = allMatch && positions[vertexIndex] == cube.Positions[vertexIndex] &&
			IsNear(normals[vertexIndex], cube.Normals[vertexIndex], kPackedNormalTolerance) &&
			IsNear(tangents[vertexIndex], cube.Tangents[vertexIndex], kPackedNormalTolerance) && uv0[vertexIndex] == cube.Uv0[vertexIndex] &&
			uv1[vertexIndex] == cube.Uv1[vertexIndex] && IsNear(colors[vertexIndex], cube.Colors[vertexIndex]) &&
			IsEqual(boneWeights[vertexIndex], cube.BoneWeights[vertexIndex]);
	}

	B3D_TEST_ASSERT_MSG(allMatch, "Converted split mesh data doesn't hold the source attributes.")
	B3D_TEST_ASSERT_MSG(memcmp(indices.data(), cube.Indices, sizeof(cube.Indices)) == 0, "Converted split mesh data doesn't hold the source indices.")
}

void RendererMeshDataTestSuite::TestCombine()
{
	CubeData cube;
	const Vector3 secondOffset(2.0f, 0.0f, 0.0f);

	const u32 attributesWithoutColors = kAllAttributes & ~(u32)VertexLayout::Color;
	const TShared<MeshData> first = CreateCubeMeshData(cube, CreateVertexDescription(StreamLayout::Split, kAllAttributes, NormalFormat::Float), Vector3::kZero);
	const TShared<MeshData> second = CreateCubeMeshData(cube, CreateVertexDescription(StreamLayout::Split, attributesWithoutColors, NormalFormat::Float), secondOffset);

	const Vector<Vector<SubMesh>> sourceSubMeshes = { { SubMesh(0, kIndexCount, DOT_TRIANGLE_LIST) }, { SubMesh(0, kIndexCount, DOT_TRIANGLE_LIST) } };
	Vector<SubMesh> subMeshes;
	const TShared<MeshData> combined = MeshData::Combine({ first, second }, sourceSubMeshes, subMeshes);

	B3D_TEST_ASSERT(combined->GetVertexCount() == kVertexCount * 2 && combined->GetIndexCount() == kIndexCount * 2)
	B3D_TEST_ASSERT(subMeshes.size() == 2 && subMeshes[0].IndexOffset == 0 && subMeshes[1].IndexOffset == kIndexCount)

	const TShared<VertexDescription> combinedDescription = combined->GetVertexDescription();
	const VertexElementSemantic semantics[] = { VES_POSITION, VES_NORMAL, VES_TANGENT, VES_BLEND_INDICES, VES_BLEND_WEIGHTS, VES_TEXCOORD, VES_COLOR };

	bool streamsMatch = combinedDescription->GetElementCount() == 8;
	for(VertexElementSemantic semantic : semantics)
	{
		const VertexElement* element = combinedDescription->FindElement(semantic);
		streamsMatch = streamsMatch && element != nullptr && element->GetStreamIndex() == GetStreamIndex(StreamLayout::Split, semantic);
	}

	B3D_TEST_ASSERT_MSG(streamsMatch, "Combined split mesh data doesn't declare each attribute once, in its source stream.")

	Vector<Vector3> positions(kVertexCount * 2, Vector3::kZero);
	Vector<Vector2> uv1(kVertexCount * 2, Vector2::kZero);
	Vector<u32> colors(kVertexCount * 2, 1);
	combined->GetVertexData(VES_POSITION, positions.data(), kVertexCount * 2 * sizeof(Vector3), 0, 0);
	combined->GetVertexData(VES_TEXCOORD, uv1.data(), kVertexCount * 2 * sizeof(Vector2), 1, 2);
	combined->GetVertexData(VES_COLOR, colors.data(), kVertexCount * 2 * sizeof(u32), 0, 2);

	bool verticesMatch = true;
	for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
	{
		verticesMatch = verticesMatch && positions[vertexIndex] == cube.Positions[vertexIndex] &&
			positions[kVertexCount + vertexIndex] == cube.Positions[vertexIndex] + secondOffset && uv1[vertexIndex] == cube.Uv1[vertexIndex] &&
			uv1[kVertexCount + vertexIndex] == cube.Uv1[vertexIndex] && colors[vertexIndex] == cube.PackedColors[vertexIndex] &&
			colors[kVertexCount + vertexIndex] == 0;
	}

	B3D_TEST_ASSERT_MSG(verticesMatch, "Combined split mesh data doesn't hold the vertices of both meshes, with zero colors for the second.")

	const u32* indices = combined->GetIndices32();
	bool indicesMatch = true;
	for(u32 index = 0; index < kIndexCount; index++)
		indicesMatch = indicesMatch && indices[index] == cube.Indices[index] && indices[kIndexCount + index] == cube.Indices[index] + kVertexCount;

	B3D_TEST_ASSERT_MSG(indicesMatch, "Combined mesh data doesn't hold the indices of both meshes, the second offset by the first's vertices.")

	// An attribute in different streams can't be combined
	{
		LoggingScope logs(*this);
		logs.ExpectError("are in different streams");

		const TShared<MeshData> interleaved = CreateCubeMeshData(cube, CreateVertexDescription(StreamLayout::Interleaved, kAllAttributes, NormalFormat::Float), Vector3::kZero);

		Vector<SubMesh> mixedSubMeshes;
		MeshData::Combine({ first, interleaved }, sourceSubMeshes, mixedSubMeshes);
	}
}

void RendererMeshDataTestSuite::TestPhysicsMesh()
{
	CubeData cube;

	const TShared<VertexDescription> vertexDescription = CreateVertexDescription(StreamLayout::Split, kAllAttributes, NormalFormat::Float);
	const TShared<MeshData> meshData = CreateCubeMeshData(cube, vertexDescription, Vector3::kZero);

	for(PhysicsMeshType type : { PhysicsMeshType::Triangle, PhysicsMeshType::Convex })
	{
		const HPhysicsMesh physicsMesh = PhysicsMesh::Create(meshData, type);
		B3D_TEST_ASSERT(physicsMesh.IsLoaded())

		const TShared<MeshData> cooked = physicsMesh->GetMeshData();
		B3D_TEST_ASSERT(cooked != nullptr && cooked->GetVertexCount() > 0)
		B3D_TEST_ASSERT_MSG(HasCubeCorners(cube, *cooked, 1e-5f), "A physics mesh cooked from split mesh data doesn't have the source positions.")
	}
}

void RendererMeshDataTestSuite::TestParticleEmitter()
{
	CubeData cube;

	const TShared<VertexDescription> vertexDescription = CreateVertexDescription(StreamLayout::Split, kAllAttributes, NormalFormat::Packed);
	const TShared<RendererMeshData> meshData = RendererMeshData::Create(MeshData::Create(kVertexCount, kIndexCount, vertexDescription));
	meshData->SetPositions(cube.Positions, sizeof(cube.Positions));
	meshData->SetNormals(cube.Normals, sizeof(cube.Normals));
	meshData->SetIndices(cube.Indices, sizeof(cube.Indices));

	ParticleStaticMeshShapeSettings settings;
	settings.Type = ParticleEmitterMeshType::Vertex;
	settings.Sequential = true;
	settings.Mesh = Mesh::Create(meshData->GetData(), MeshFlag::Static | MeshFlag::KeepCPUCopy);

	const TShared<ParticleEmitterStaticMeshShape> shape = ParticleEmitterStaticMeshShape::Create(settings);

	ParticleSet particles(kVertexCount);
	Random random(1);
	ParticleSystemState state;
	const u32 firstParticle = shape->SpawnInternal(random, particles, kVertexCount, state);

	B3D_TEST_ASSERT(particles.GetParticleCount() == firstParticle + kVertexCount)

	const ParticleSetData& particleData = particles.GetParticles();
	bool particlesMatch = true;
	for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
	{
		particlesMatch = particlesMatch && particleData.Position[firstParticle + vertexIndex] == cube.Positions[vertexIndex] &&
			IsNear(particleData.Velocity[firstParticle + vertexIndex], cube.Normals[vertexIndex], kPackedNormalTolerance);
	}

	B3D_TEST_ASSERT_MSG(particlesMatch, "Particles emitted from the vertices of a split mesh don't have its positions and normals.")
}
