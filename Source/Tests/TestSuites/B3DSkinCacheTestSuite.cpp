//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSkinCacheTestSuite.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "Animation/B3DMorphShapes.h"
#include "Animation/B3DSkeleton.h"
#include "CoreObject/B3DRenderThread.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DSubMesh.h"
#include "GpuBackend/B3DVertexData.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DTexture.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Mesh/B3DMesh.h"
#include "Mesh/B3DMeshData.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererMeshData.h"
#include "Renderer/B3DRendererUtility.h"
#include "SkinCache/B3DSkinCache.h"
#include "Threading/B3DAsyncOp.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	constexpr u32 kRingCount = 10;
	constexpr u32 kRingVertexCount = 13;
	constexpr u32 kVertexCount = kRingCount * kRingVertexCount; // Two full groups of the deformation shader and a partial one
	constexpr u32 kIndexCount = (kRingCount - 1) * kRingVertexCount * 6;
	constexpr u32 kBoneCount = 3;
	constexpr u32 kMorphShapeCount = 2;
	constexpr float kRingRadius = 0.25f;

	/** Texture coordinates of every vertex of the test mesh. */
	const Vector2 kUv(0.25f, 0.75f);

	/**
	 * Largest difference between a component of a deformed normal or tangent, stored in 8 bits, and the exact value: half a step of
	 * 2 / 255, and a little for floating point differences.
	 */
	constexpr float kNormalTolerance = 1.0f / 255.0f + 1e-4f;

	/** Largest difference between a component of a deformed position and the reference. */
	constexpr float kPositionTolerance = 1e-5f;

	/** Size of the render target the tests draw the deformed mesh into, in pixels. */
	constexpr u32 kTargetSize = 16;

	/** Vertex of the test mesh after deformation. */
	struct DeformedVertex
	{
		DeformedVertex() = default;

		Vector3 Position;
		Vector3 Normal;
		Vector3 Tangent;
		u8 NormalW = 0; /**< Fourth byte of the packed normal. */
		u8 TangentW = 0; /**< Fourth byte of the packed tangent: the handedness. */
	};

	/** Returns a component of a normal or tangent stored in 8 bits as N * 0.5 + 0.5, in [-1, 1]. */
	float UnpackComponent(u8 value)
	{
		return (value / 255.0f) * 2.0f - 1.0f;
	}

	/**
	 * Returns the vertices of the test mesh: rings of radius kRingRadius around the Y axis from Y = 0 to 1, with outward normals, tangents
	 * around the axis whose handedness alternates, and kUv. Rings take one of four sets of up to four bone weights, each of which adds up
	 * to exactly 1.
	 */
	TShared<MeshData> CreateTestMeshData()
	{
		const VertexLayout layout = (VertexLayout)((i32)VertexLayout::Position | (i32)VertexLayout::Normal | (i32)VertexLayout::Tangent | (i32)VertexLayout::UV0 |
			(i32)VertexLayout::BoneWeights);
		const TShared<RendererMeshData> rendererMeshData = RendererMeshData::Create(kVertexCount, kIndexCount, layout);

		Vector<Vector3> positions;
		Vector<Vector3> normals;
		Vector<Vector4> tangents;
		Vector<Vector2> uvs;
		Vector<BoneWeight> boneWeights;
		for(u32 ringIndex = 0; ringIndex < kRingCount; ringIndex++)
		{
			const float height = ringIndex / (float)(kRingCount - 1);

			BoneWeight boneWeight;
			boneWeight.Index0 = boneWeight.Index1 = boneWeight.Index2 = boneWeight.Index3 = 0;
			boneWeight.Weight0 = boneWeight.Weight1 = boneWeight.Weight2 = boneWeight.Weight3 = 0.0f;
			switch(ringIndex % 4)
			{
			case 0:
				boneWeight.Weight0 = 1.0f;
				break;
			case 1:
				boneWeight.Index1 = 1;
				boneWeight.Weight0 = 0.75f;
				boneWeight.Weight1 = 0.25f;
				break;
			case 2:
				boneWeight.Index0 = 1;
				boneWeight.Index2 = 2;
				boneWeight.Index3 = 2;
				boneWeight.Weight0 = 0.5f;
				boneWeight.Weight1 = 0.25f;
				boneWeight.Weight2 = 0.125f;
				boneWeight.Weight3 = 0.125f;
				break;
			default:
				boneWeight.Index0 = 2;
				boneWeight.Index1 = 1;
				boneWeight.Weight0 = 0.25f;
				boneWeight.Weight1 = 0.75f;
				break;
			}

			for(u32 ringVertexIndex = 0; ringVertexIndex < kRingVertexCount; ringVertexIndex++)
			{
				const float angle = Math::kTwoPi * ringVertexIndex / (float)kRingVertexCount;
				const float cosine = std::cos(angle);
				const float sine = std::sin(angle);

				positions.push_back(Vector3(kRingRadius * cosine, height, kRingRadius * sine));
				normals.push_back(Vector3(cosine, 0.0f, sine));
				tangents.push_back(Vector4(-sine, 0.0f, cosine, ringVertexIndex % 2 == 0 ? 1.0f : -1.0f));
				uvs.push_back(kUv);
				boneWeights.push_back(boneWeight);
			}
		}

		Vector<u32> indices;
		for(u32 ringIndex = 0; ringIndex < kRingCount - 1; ringIndex++)
		{
			for(u32 ringVertexIndex = 0; ringVertexIndex < kRingVertexCount; ringVertexIndex++)
			{
				const u32 nextRingVertexIndex = (ringVertexIndex + 1) % kRingVertexCount;
				const u32 lowerLeft = ringIndex * kRingVertexCount + ringVertexIndex;
				const u32 lowerRight = ringIndex * kRingVertexCount + nextRingVertexIndex;
				const u32 upperLeft = lowerLeft + kRingVertexCount;
				const u32 upperRight = lowerRight + kRingVertexCount;

				for(u32 index : { lowerLeft, upperLeft, lowerRight, lowerRight, upperLeft, upperRight })
					indices.push_back(index);
			}
		}

		rendererMeshData->SetPositions(positions.data(), (u32)(positions.size() * sizeof(Vector3)));
		rendererMeshData->SetNormals(normals.data(), (u32)(normals.size() * sizeof(Vector3)));
		rendererMeshData->SetTangents(tangents.data(), (u32)(tangents.size() * sizeof(Vector4)));
		rendererMeshData->SetUV0(uvs.data(), (u32)(uvs.size() * sizeof(Vector2)));
		rendererMeshData->SetBoneWeights(boneWeights.data(), (u32)(boneWeights.size() * sizeof(BoneWeight)));
		rendererMeshData->SetIndices(indices.data(), (u32)(indices.size() * sizeof(u32)));

		return rendererMeshData->GetData();
	}

	/**
	 * Creates the render thread mesh of @p meshData, with a skeleton of kBoneCount bones, and if @p hasMorphShapes is true, a channel of
	 * kMorphShapeCount morph shapes. Waits until the GPU holds its contents.
	 */
	TShared<render::Mesh> CreateTestMesh(GpuDevice& device, const TShared<MeshData>& meshData, bool hasMorphShapes)
	{
		BoneInformation bones[kBoneCount];
		for(u32 boneIndex = 0; boneIndex < kBoneCount; boneIndex++)
		{
			bones[boneIndex].Name = "Bone" + ToString(boneIndex);
			bones[boneIndex].Parent = boneIndex == 0 ? (u32)-1 : boneIndex - 1;
			bones[boneIndex].LocalTfrm = Transform::kIdentity;
			bones[boneIndex].InvBindPose = Matrix4::kIdentity;
		}

		MeshCreateInformation meshInformation;
		meshInformation.SubMeshes = { SubMesh(0, kIndexCount, DOT_TRIANGLE_LIST) };
		meshInformation.Skeleton = Skeleton::Create(bones, kBoneCount);

		if(hasMorphShapes)
		{
			const Vector<MorphVertex> firstShapeVertices = { MorphVertex(Vector3(0.1f, 0.0f, 0.0f), Vector3::kZero, 0) };
			const Vector<MorphVertex> secondShapeVertices = { MorphVertex(Vector3(0.0f, 0.1f, 0.0f), Vector3::kZero, 1) };
			const TShared<MorphChannel> channel = MorphChannel::Create("Channel", { MorphShape::Create("First", 0.5f, firstShapeVertices),
				MorphShape::Create("Second", 1.0f, secondShapeVertices) });

			meshInformation.MorphShapes = MorphShapes::Create({ channel }, kVertexCount);
		}

		const TShared<render::Mesh> mesh = render::Mesh::Create(meshData, meshInformation);

		// The mesh's initial contents are written through the renderer's context
		GetRenderer()->GetGpuContext().SubmitTransferCommandBuffers();
		device.WaitUntilIdle();

		return mesh;
	}

	/** Returns transforms of the bones of the test mesh that rotate, translate and scale each bone differently, by an amount of @p amount. */
	Vector<Matrix4> CreatePose(float amount)
	{
		Vector<Matrix4> pose;
		pose.push_back(Matrix4::TRS(Vector3(0.1f * amount, 0.0f, -0.05f * amount), Quaternion(Vector3::kUnitY, Radian(0.4f * amount)), Vector3::kOne));
		pose.push_back(Matrix4::TRS(Vector3(0.0f, 0.2f * amount, 0.1f), Quaternion(Vector3::kUnitZ, Radian(0.6f * amount)), Vector3(1.25f, 1.25f, 1.25f)));
		pose.push_back(Matrix4::TRS(Vector3(-0.3f * amount, 0.1f, 0.0f), Quaternion(Vector3::kUnitX, Radian(-0.5f * amount)), Vector3(0.8f, 1.1f, 1.0f)));

		return pose;
	}

	/**
	 * Returns the vertices of @p meshData deformed by @p pose, computed like the deformation shader does: the bone transforms blended by
	 * the vertex's weights, applied to the position and to the normal and tangent unpacked from 8 bits, which are then normalized.
	 */
	Vector<DeformedVertex> DeformOnCpu(const MeshData& meshData, TArrayView<const Matrix4> pose)
	{
		const u32 stride = meshData.GetVertexDescription()->GetVertexStride(0);
		const u8* const positionData = meshData.GetElementData(VES_POSITION);
		const u8* const normalData = meshData.GetElementData(VES_NORMAL);
		const u8* const tangentData = meshData.GetElementData(VES_TANGENT);
		const u8* const blendIndexData = meshData.GetElementData(VES_BLEND_INDICES);
		const u8* const blendWeightData = meshData.GetElementData(VES_BLEND_WEIGHTS);

		Vector<DeformedVertex> vertices(kVertexCount);
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
		{
			const u32 vertexOffset = vertexIndex * stride;

			Vector3 position;
			std::memcpy(&position, positionData + vertexOffset, sizeof(Vector3));

			float blendWeights[4];
			std::memcpy(blendWeights, blendWeightData + vertexOffset, sizeof(blendWeights));

			const u8* const blendIndices = blendIndexData + vertexOffset;
			const u8* const normalBytes = normalData + vertexOffset;
			const u8* const tangentBytes = tangentData + vertexOffset;

			float blendMatrix[3][4] = {};
			for(u32 influenceIndex = 0; influenceIndex < 4; influenceIndex++)
			{
				const Matrix4& boneTransform = pose[blendIndices[influenceIndex]];
				for(u32 row = 0; row < 3; row++)
				{
					for(u32 column = 0; column < 4; column++)
						blendMatrix[row][column] += blendWeights[influenceIndex] * boneTransform[row][column];
				}
			}

			const Vector3 normal(UnpackComponent(normalBytes[0]), UnpackComponent(normalBytes[1]), UnpackComponent(normalBytes[2]));
			const Vector3 tangent(UnpackComponent(tangentBytes[0]), UnpackComponent(tangentBytes[1]), UnpackComponent(tangentBytes[2]));

			DeformedVertex& vertex = vertices[vertexIndex];
			for(u32 row = 0; row < 3; row++)
			{
				const float* const blendRow = blendMatrix[row];
				vertex.Position[row] = blendRow[0] * position.X + blendRow[1] * position.Y + blendRow[2] * position.Z + blendRow[3];
				vertex.Normal[row] = blendRow[0] * normal.X + blendRow[1] * normal.Y + blendRow[2] * normal.Z;
				vertex.Tangent[row] = blendRow[0] * tangent.X + blendRow[1] * tangent.Y + blendRow[2] * tangent.Z;
			}

			vertex.Normal = Vector3::Normalize(vertex.Normal);
			vertex.Tangent = Vector3::Normalize(vertex.Tangent);
			vertex.NormalW = normalBytes[3];
			vertex.TangentW = tangentBytes[3];
		}

		return vertices;
	}

	/** Returns the buffer @p vertexData binds to the stream of the element with @p semantic and @p semanticIndex, or null if it has none. */
	TShared<render::GpuBuffer> FindStreamBuffer(const VertexData& vertexData, VertexElementSemantic semantic, u16 semanticIndex)
	{
		for(const VertexElement& element : vertexData.VertexDescription->GetElements())
		{
			if(element.GetSemantic() == semantic && element.GetSemanticIndex() == semanticIndex)
				return vertexData.GetBuffer(element.GetStreamIndex());
		}

		return nullptr;
	}

	/** Reads the positions of the test mesh's vertices from @p buffer, a buffer of deformed positions. */
	Vector<Vector3> ReadPositions(GpuWorkContext& context, const TShared<render::GpuBuffer>& buffer)
	{
		Vector<Vector3> positions(kVertexCount);
		GpuBufferUtility::Read(context, buffer, 0, kVertexCount * sizeof(Vector3), positions.data());

		return positions;
	}

	/** Reads the deformed vertices @p vertexData binds as POSITION0, NORMAL0 and TANGENT0. */
	Vector<DeformedVertex> ReadDeformedVertices(GpuWorkContext& context, const VertexData& vertexData)
	{
		const Vector<Vector3> positions = ReadPositions(context, FindStreamBuffer(vertexData, VES_POSITION, 0));

		Vector<u8> normalsTangents(kVertexCount * 8);
		GpuBufferUtility::Read(context, FindStreamBuffer(vertexData, VES_NORMAL, 0), 0, (u32)normalsTangents.size(), normalsTangents.data());

		Vector<DeformedVertex> vertices(kVertexCount);
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
		{
			const u8* const normalBytes = &normalsTangents[vertexIndex * 8];
			const u8* const tangentBytes = normalBytes + 4;

			DeformedVertex& vertex = vertices[vertexIndex];
			vertex.Position = positions[vertexIndex];
			vertex.Normal = Vector3(UnpackComponent(normalBytes[0]), UnpackComponent(normalBytes[1]), UnpackComponent(normalBytes[2]));
			vertex.Tangent = Vector3(UnpackComponent(tangentBytes[0]), UnpackComponent(tangentBytes[1]), UnpackComponent(tangentBytes[2]));
			vertex.NormalW = normalBytes[3];
			vertex.TangentW = tangentBytes[3];
		}

		return vertices;
	}

	/** Returns true if every component of @p actual is within @p tolerance of @p expected. */
	bool IsWithin(const Vector3& actual, const Vector3& expected, float tolerance)
	{
		return std::abs(actual.X - expected.X) <= tolerance && std::abs(actual.Y - expected.Y) <= tolerance && std::abs(actual.Z - expected.Z) <= tolerance;
	}

	/**
	 * Returns a description of the first vertex of @p actual that differs from @p expected: a position by more than @p positionTolerance,
	 * a normal or tangent by more than kNormalTolerance, or another fourth byte of the normal or tangent. Returns an empty string if every
	 * vertex matches.
	 */
	String FindMismatch(const Vector<DeformedVertex>& expected, const Vector<DeformedVertex>& actual, float positionTolerance)
	{
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
		{
			const DeformedVertex& expectedVertex = expected[vertexIndex];
			const DeformedVertex& actualVertex = actual[vertexIndex];
			if(IsWithin(actualVertex.Position, expectedVertex.Position, positionTolerance) && IsWithin(actualVertex.Normal, expectedVertex.Normal, kNormalTolerance) &&
				IsWithin(actualVertex.Tangent, expectedVertex.Tangent, kNormalTolerance) && actualVertex.NormalW == expectedVertex.NormalW &&
				actualVertex.TangentW == expectedVertex.TangentW)
			{
				continue;
			}

			// The message is formatted in two parts, since a format takes at most 20 arguments
			const String positionAndNormal = StringUtility::Format("Vertex {0}: position ({1}, {2}, {3}), expected ({4}, {5}, {6}); normal ({7}, {8}, {9}), expected ({10}, "
				"{11}, {12}); ", vertexIndex, actualVertex.Position.X, actualVertex.Position.Y, actualVertex.Position.Z, expectedVertex.Position.X, expectedVertex.Position.Y,
				expectedVertex.Position.Z, actualVertex.Normal.X, actualVertex.Normal.Y, actualVertex.Normal.Z, expectedVertex.Normal.X, expectedVertex.Normal.Y,
				expectedVertex.Normal.Z);

			return positionAndNormal + StringUtility::Format("tangent ({0}, {1}, {2}), expected ({3}, {4}, {5}); fourth bytes {6} and {7}, expected {8} and {9}.",
				actualVertex.Tangent.X, actualVertex.Tangent.Y, actualVertex.Tangent.Z, expectedVertex.Tangent.X, expectedVertex.Tangent.Y, expectedVertex.Tangent.Z,
				(u32)actualVertex.NormalW, (u32)actualVertex.TangentW, (u32)expectedVertex.NormalW, (u32)expectedVertex.TangentW);
		}

		return String();
	}

	/** Returns a description of the first position of @p actual that differs from the position of @p expected by more than kPositionTolerance, or an empty string. */
	String FindPositionMismatch(const Vector<DeformedVertex>& expected, const Vector<Vector3>& actual)
	{
		for(u32 vertexIndex = 0; vertexIndex < kVertexCount; vertexIndex++)
		{
			const Vector3& expectedPosition = expected[vertexIndex].Position;
			if(!IsWithin(actual[vertexIndex], expectedPosition, kPositionTolerance))
			{
				return StringUtility::Format("Vertex {0}: position ({1}, {2}, {3}), expected ({4}, {5}, {6}).", vertexIndex, actual[vertexIndex].X, actual[vertexIndex].Y,
					actual[vertexIndex].Z, expectedPosition.X, expectedPosition.Y, expectedPosition.Z);
			}
		}

		return String();
	}

	/**
	 * Adds a raster pass to @p graph that draws the first sub-mesh of @p mesh with each of @p vertexDatas into a new kTargetSize x
	 * kTargetSize target, with @p pipeline (see CreateTestSkinCacheDrawPipeline()), and returns the target.
	 */
	TShared<render::Texture> AddDrawPass(FrameGraph& graph, const TShared<GpuGraphicsPipelineState>& pipeline, const Vector<TShared<VertexData>>& vertexDatas,
		const TShared<render::Mesh>& mesh)
	{
		TextureCreateInformation targetInformation;
		targetInformation.Name = "Skin cache draw target";
		targetInformation.Width = kTargetSize;
		targetInformation.Height = kTargetSize;
		targetInformation.Format = PF_RGBA8;
		targetInformation.Usage = TextureUsageFlag::RenderTarget;
		targetInformation.ClearColor = Color::kZero;

		const TShared<render::Texture> target = graph.CreateTexture("Skin cache draw target", targetInformation);

		FrameGraphRasterPass pass;
		pass.SetColorAttachment(0, target, FrameGraphAttachmentLoad::Clear);

		const SubMesh subMesh = mesh->GetProperties().SubMeshes[0];
		const TShared<render::GpuBuffer> indexBuffer = mesh->GetIndexBuffer();
		graph.AddRasterPass("Draw deformed meshes", std::move(pass), [pipeline, vertexDatas, indexBuffer, subMesh](FrameGraphPassContext& passContext)
		{
			render::GpuCommandBuffer& commandBuffer = passContext.GetCommandBuffer();
			commandBuffer.SetGpuGraphicsPipelineState(pipeline);
			commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

			for(const TShared<VertexData>& vertexData : vertexDatas)
				GetRendererUtility().Draw(commandBuffer, *vertexData, indexBuffer, subMesh);
		});

		return target;
	}

	/** Updates @p cache in a graph of its own, executes it and waits for the GPU. Returns true if the update added a pass. */
	bool UpdateAndExecute(TestSuite& suite, SkinCache& cache, GpuWorkContext& context, GpuDevice& device)
	{
		bool hasPasses = false;
		{
			FrameGraph graph(context, "Skin cache test");
			hasPasses = cache.Update(graph);

			graph.Compile();
			B3D_TEST_ASSERT_EXTERNAL_MSG(suite, graph.GetStatistics().PassCount == (hasPasses ? 1u : 0u) && graph.GetStatistics().CulledPassCount == 0,
				StringUtility::Format("The skin cache update added {0} passes, of which {1} were culled.", graph.GetStatistics().PassCount, graph.GetStatistics().CulledPassCount))

			graph.Execute();
		}

		device.WaitUntilIdle();
		device.EndFrame();

		return hasPasses;
	}
} // namespace

SkinCacheTestSuite::SkinCacheTestSuite()
	: TestSuite("SkinCacheTestSuite")
{
	B3D_ADD_TEST(SkinCacheTestSuite::TestBindPose)
	B3D_ADD_TEST(SkinCacheTestSuite::TestSkinning)
	B3D_ADD_TEST(SkinCacheTestSuite::TestPreviousPositions)
	B3D_ADD_TEST(SkinCacheTestSuite::TestChangeDetection)
	B3D_ADD_TEST(SkinCacheTestSuite::TestVertexLayout)
	B3D_ADD_TEST(SkinCacheTestSuite::TestFrameGraph)
}

void SkinCacheTestSuite::TestBindPose()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		const TShared<MeshData> meshData = CreateTestMeshData();
		const TShared<render::Mesh> mesh = CreateTestMesh(*device, meshData, false);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		SkinCache cache(*device, "Test skin cache");
		const TShared<SkinCacheEntry> entry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::None, "Bind pose"));
		B3D_TEST_ASSERT(entry != nullptr)
		if(entry == nullptr)
			return;

		// Nothing is deformed or allocated before the first update
		B3D_TEST_ASSERT(cache.GetEntryCount() == 1 && entry->GetMesh() == mesh && !entry->GetFlags().IsSet(SkinCacheEntryFlag::PreviousPositions))
		B3D_TEST_ASSERT(!entry->HasDeformedVertices() && entry->GetVertexData() == nullptr && entry->GetRayTracingGeometry() == nullptr)
		B3D_TEST_ASSERT(entry->GetMemorySize() == 0 && cache.GetMemorySize() == 0)

		// The entry starts in the bind pose, which the first update deforms it to
		B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device))
		B3D_TEST_ASSERT(entry->HasDeformedVertices() && entry->GetVertexData() != nullptr)
		if(entry->GetVertexData() == nullptr)
			return;

		// Identity transforms give the mesh's positions exactly, since every vertex's weights add up to exactly 1
		const Vector<Matrix4> bindPose(kBoneCount, Matrix4::kIdentity);
		const String mismatch = FindMismatch(DeformOnCpu(*meshData, bindPose), ReadDeformedVertices(*context, *entry->GetVertexData()), 0.0f);
		B3D_TEST_ASSERT_MSG(mismatch.empty(), mismatch)

		// The bind pose has no previous positions, and no ray tracing geometry
		B3D_TEST_ASSERT(FindStreamBuffer(*entry->GetVertexData(), VES_POSITION, 1) == nullptr && entry->GetRayTracingGeometry() == nullptr)

		const u32 expectedMemorySize = kVertexCount * (12 + 8);
		B3D_TEST_ASSERT_MSG(entry->GetMemorySize() >= expectedMemorySize && cache.GetMemorySize() == entry->GetMemorySize(),
			StringUtility::Format("Memory size {0}, expected at least {1}.", entry->GetMemorySize(), expectedMemorySize))

		// The same pose again changes nothing
		entry->SetPose(bindPose);
		B3D_TEST_ASSERT(!UpdateAndExecute(*this, cache, *context, *device))
	}, "SkinCacheTestSuite::TestBindPose", true);
}

void SkinCacheTestSuite::TestSkinning()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		const TShared<MeshData> meshData = CreateTestMeshData();
		const TShared<render::Mesh> mesh = CreateTestMesh(*device, meshData, false);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		SkinCache cache(*device, "Test skin cache");
		const TShared<SkinCacheEntry> entry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::None, "Skinning"));
		B3D_TEST_ASSERT(entry != nullptr)
		if(entry == nullptr)
			return;

		// Each pose is deformed into the same buffers, from the mesh's vertices
		for(float amount : { 1.0f, -0.7f })
		{
			const Vector<Matrix4> pose = CreatePose(amount);
			entry->SetPose(pose);
			B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device))
			B3D_TEST_ASSERT(entry->GetVertexData() != nullptr)
			if(entry->GetVertexData() == nullptr)
				return;

			const String mismatch = FindMismatch(DeformOnCpu(*meshData, pose), ReadDeformedVertices(*context, *entry->GetVertexData()), kPositionTolerance);
			B3D_TEST_ASSERT_MSG(mismatch.empty(), StringUtility::Format("Pose {0}: {1}", amount, mismatch))
		}
	}, "SkinCacheTestSuite::TestSkinning", true);
}

void SkinCacheTestSuite::TestPreviousPositions()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		const TShared<MeshData> meshData = CreateTestMeshData();
		const TShared<render::Mesh> mesh = CreateTestMesh(*device, meshData, false);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		SkinCache cache(*device, "Test skin cache");
		const TShared<SkinCacheEntry> entry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::PreviousPositions, "Previous positions"));
		B3D_TEST_ASSERT(entry != nullptr && entry->GetFlags().IsSet(SkinCacheEntryFlag::PreviousPositions))
		if(entry == nullptr)
			return;

		TShared<VertexDescription> vertexDescription;
		TShared<VertexData> previousVertexData;

		// Updates the cache, with @p pose set first unless it is null, and checks that the entry's vertex data binds @p currentBuffer as
		// POSITION0 and @p previousBuffer as POSITION1, and that they hold @p currentPose and @p previousPose. A null buffer stands for any
		// buffer other than those named. Returns the buffers the vertex data binds as POSITION0 and POSITION1.
		const auto fnCheckUpdate = [&](const char* step, const Vector<Matrix4>* pose, bool expectsPasses, const Vector<Matrix4>& currentPose,
			const Vector<Matrix4>& previousPose, const TShared<render::GpuBuffer>& currentBuffer, const TShared<render::GpuBuffer>& previousBuffer)
		{
			if(pose != nullptr)
				entry->SetPose(*pose);

			B3D_TEST_ASSERT_MSG(UpdateAndExecute(*this, cache, *context, *device) == expectsPasses, StringUtility::Format("{0}: unexpected update result.", step))

			const TShared<VertexData>& vertexData = entry->GetVertexData();
			B3D_TEST_ASSERT_MSG(vertexData != nullptr, StringUtility::Format("{0}: no vertex data.", step))
			if(vertexData == nullptr)
				return std::make_pair(TShared<render::GpuBuffer>(), TShared<render::GpuBuffer>());

			// Every vertex data of the entry shares one description, and the entry returns another one after each deformation
			if(vertexDescription == nullptr)
				vertexDescription = vertexData->VertexDescription;

			B3D_TEST_ASSERT_MSG(vertexData->VertexDescription == vertexDescription, StringUtility::Format("{0}: the vertex description changed.", step))
			B3D_TEST_ASSERT_MSG(!expectsPasses || vertexData != previousVertexData, StringUtility::Format("{0}: the vertex data didn't change.", step))
			previousVertexData = vertexData;

			const TShared<render::GpuBuffer> boundCurrent = FindStreamBuffer(*vertexData, VES_POSITION, 0);
			const TShared<render::GpuBuffer> boundPrevious = FindStreamBuffer(*vertexData, VES_POSITION, 1);
			B3D_TEST_ASSERT_MSG(boundCurrent != nullptr && boundPrevious != nullptr, StringUtility::Format("{0}: POSITION0 or POSITION1 isn't bound.", step))
			if(boundCurrent == nullptr || boundPrevious == nullptr)
				return std::make_pair(boundCurrent, boundPrevious);

			const auto fnIsExpectedBuffer = [&currentBuffer, &previousBuffer](const TShared<render::GpuBuffer>& actual, const TShared<render::GpuBuffer>& expected)
			{
				if(expected != nullptr)
					return actual == expected;

				return actual != currentBuffer && actual != previousBuffer;
			};

			B3D_TEST_ASSERT_MSG(fnIsExpectedBuffer(boundCurrent, currentBuffer), StringUtility::Format("{0}: POSITION0 is bound to another buffer.", step))
			B3D_TEST_ASSERT_MSG(fnIsExpectedBuffer(boundPrevious, previousBuffer), StringUtility::Format("{0}: POSITION1 is bound to another buffer.", step))

			const String currentMismatch = FindMismatch(DeformOnCpu(*meshData, currentPose), ReadDeformedVertices(*context, *vertexData), kPositionTolerance);
			B3D_TEST_ASSERT_MSG(currentMismatch.empty(), StringUtility::Format("{0}, current vertices: {1}", step, currentMismatch))

			const String previousMismatch = FindPositionMismatch(DeformOnCpu(*meshData, previousPose), ReadPositions(*context, boundPrevious));
			B3D_TEST_ASSERT_MSG(previousMismatch.empty(), StringUtility::Format("{0}, previous positions: {1}", step, previousMismatch))

			return std::make_pair(boundCurrent, boundPrevious);
		};

		const Vector<Matrix4> firstPose = CreatePose(0.5f);
		const Vector<Matrix4> secondPose = CreatePose(1.0f);
		const Vector<Matrix4> thirdPose = CreatePose(-0.5f);
		const Vector<Matrix4> fourthPose = CreatePose(0.25f);

		// The first deformation has no previous pose, so its one buffer is bound twice
		entry->SetPose(firstPose);
		B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device) && entry->GetVertexData() != nullptr)
		if(entry->GetVertexData() == nullptr)
			return;

		const TShared<render::GpuBuffer> firstBuffer = FindStreamBuffer(*entry->GetVertexData(), VES_POSITION, 0);
		B3D_TEST_ASSERT(firstBuffer != nullptr && FindStreamBuffer(*entry->GetVertexData(), VES_POSITION, 1) == firstBuffer)
		B3D_TEST_ASSERT(entry->GetMemorySize() == firstBuffer->GetTotalSize() + FindStreamBuffer(*entry->GetVertexData(), VES_NORMAL, 0)->GetTotalSize())
		vertexDescription = entry->GetVertexData()->VertexDescription;
		previousVertexData = entry->GetVertexData();

		const String firstMismatch = FindMismatch(DeformOnCpu(*meshData, firstPose), ReadDeformedVertices(*context, *entry->GetVertexData()), kPositionTolerance);
		B3D_TEST_ASSERT_MSG(firstMismatch.empty(), StringUtility::Format("First pose: {0}", firstMismatch))

		// The second deformation writes a second buffer, which becomes the current one
		const TShared<render::GpuBuffer> secondBuffer = fnCheckUpdate("Second pose", &secondPose, true, secondPose, firstPose, nullptr, firstBuffer).first;
		B3D_TEST_ASSERT(secondBuffer != nullptr && entry->GetMemorySize() == 2 * firstBuffer->GetTotalSize() + kVertexCount * 8)

		// The third swaps the roles of the two buffers again
		fnCheckUpdate("Third pose", &thirdPose, true, thirdPose, secondPose, firstBuffer, secondBuffer);

		// An update without a new pose leaves the vertices where they are, with no motion
		fnCheckUpdate("No new pose", nullptr, false, thirdPose, thirdPose, firstBuffer, firstBuffer);

		// The next pose moves from what was on screen last: the third pose
		fnCheckUpdate("Fourth pose", &fourthPose, true, fourthPose, thirdPose, secondBuffer, firstBuffer);
	}, "SkinCacheTestSuite::TestPreviousPositions", true);
}

void SkinCacheTestSuite::TestChangeDetection()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		const TShared<MeshData> meshData = CreateTestMeshData();
		const TShared<render::Mesh> mesh = CreateTestMesh(*device, meshData, true);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		SkinCache cache(*device, "Test skin cache");
		const TShared<SkinCacheEntry> entry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::None, "Change detection"));
		B3D_TEST_ASSERT(entry != nullptr)
		if(entry == nullptr)
			return;

		// The bind pose set before the first update is the pose the entry starts in, which is deformed once
		const Vector<Matrix4> bindPose(kBoneCount, Matrix4::kIdentity);
		const Vector<float> bindPoseWeights(kMorphShapeCount, 0.0f);
		entry->SetPose(bindPose, bindPoseWeights);
		B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device))
		B3D_TEST_ASSERT(!UpdateAndExecute(*this, cache, *context, *device))

		// The same pose twice adds one pass
		const Vector<Matrix4> pose = CreatePose(1.0f);
		Vector<float> weights = { 0.5f, 0.0f };
		entry->SetPose(pose, weights);
		B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device))

		entry->SetPose(pose, weights);
		B3D_TEST_ASSERT(!UpdateAndExecute(*this, cache, *context, *device))

		// A changed morph shape weight alone is a new pose
		weights[1] = 0.25f;
		entry->SetPose(pose, weights);
		B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device))

		// Poses set several times before an update are deformed once, to the last one
		entry->SetPose(CreatePose(0.5f), weights);
		entry->SetPose(pose, weights);
		entry->SetPose(CreatePose(0.75f), weights);
		B3D_TEST_ASSERT(UpdateAndExecute(*this, cache, *context, *device))
		B3D_TEST_ASSERT(!UpdateAndExecute(*this, cache, *context, *device))

		// Poses with another number of bones or weights are rejected
		{
			LoggingScope logs(*this);
			logs.ExpectError("Skin cache entry 'Change detection' needs a pose of 3 bone transforms and 2 morph shape weights, not 2 and 2.");
			logs.ExpectError("Skin cache entry 'Change detection' needs a pose of 3 bone transforms and 2 morph shape weights, not 3 and 0.");

			entry->SetPose(TArrayView<const Matrix4>(pose.data(), 2), weights);
			entry->SetPose(pose);
		}

		B3D_TEST_ASSERT(!UpdateAndExecute(*this, cache, *context, *device))

		// A mesh without a skeleton gets no entry, nor does one whose vertices the cache can't read
		const TShared<render::Mesh> staticMesh = render::Mesh::Create(meshData, MeshCreateInformation());
		B3D_TEST_ASSERT(cache.CreateEntry(SkinCacheEntryCreateInformation(staticMesh)) == nullptr)

		TInlineArray<VertexElement, 8> floatNormalElements;
		floatNormalElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
		floatNormalElements.Add(VertexElement(VET_FLOAT3, VES_NORMAL));
		floatNormalElements.Add(VertexElement(VET_UBYTE4_NORM, VES_TANGENT));
		floatNormalElements.Add(VertexElement(VET_UBYTE4, VES_BLEND_INDICES));
		floatNormalElements.Add(VertexElement(VET_FLOAT4, VES_BLEND_WEIGHTS));

		MeshCreateInformation floatNormalInformation;
		floatNormalInformation.VertexCount = 3;
		floatNormalInformation.IndexCount = 3;
		floatNormalInformation.VertexDescription = B3DMakeShared<VertexDescription>(floatNormalElements);
		floatNormalInformation.SubMeshes = { SubMesh(0, 3, DOT_TRIANGLE_LIST) };
		floatNormalInformation.Skeleton = mesh->GetSkeleton();
		floatNormalInformation.Flags = MeshFlag::Static;
		const TShared<render::Mesh> floatNormalMesh = render::Mesh::Create(floatNormalInformation);

		{
			LoggingScope logs(*this);
			logs.ExpectWarning("can't deform mesh 'Float normals'");
			B3D_TEST_ASSERT(cache.CreateEntry(SkinCacheEntryCreateInformation(floatNormalMesh, SkinCacheEntryFlag::None, "Float normals")) == nullptr)
		}

		B3D_TEST_ASSERT(cache.GetEntryCount() == 1)
	}, "SkinCacheTestSuite::TestChangeDetection", true);
}

void SkinCacheTestSuite::TestVertexLayout()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuGraphicsPipelineState> pipeline = CreateTestSkinCacheDrawPipeline(*device);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		const TShared<MeshData> meshData = CreateTestMeshData();
		const TShared<render::Mesh> mesh = CreateTestMesh(*device, meshData, false);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		SkinCache cache(*device, "Test skin cache");
		const TShared<SkinCacheEntry> entry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::PreviousPositions, "Vertex layout"));
		B3D_TEST_ASSERT(entry != nullptr)
		if(entry == nullptr)
			return;

		// Every bone moves the mesh by 0.5 along X, out of the area the bind pose covers. The graph that deforms the mesh draws it right
		// after, from the buffers the deformation exported.
		const Vector<Matrix4> pose(kBoneCount, Matrix4::Translation(Vector3(0.5f, 0.0f, 0.0f)));
		entry->SetPose(pose);

		TAsyncOp<TShared<PixelData>> targetRead;
		{
			FrameGraph graph(*context, "Skin cache vertex layout");
			B3D_TEST_ASSERT(cache.Update(graph) && entry->GetVertexData() != nullptr)
			if(entry->GetVertexData() == nullptr)
				return;

			const TShared<render::Texture> target = AddDrawPass(graph, pipeline, { entry->GetVertexData() }, mesh);
			targetRead = TextureUtility::ReadAsync(graph, target);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().PassCount == 3 && graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		device->WaitUntilIdle();
		device->EndFrame();

		// The mesh's stream keeps its stride and its texture coordinates, with the deformed attributes as padding. The entry's streams
		// follow it.
		const VertexDescription& meshDescription = *mesh->GetVertexDescription();
		const VertexDescription& description = *entry->GetVertexData()->VertexDescription;
		u32 paddingCount = 0;
		for(const VertexElement& element : description.GetElements())
		{
			if(element.GetSemantic() == VES_PADDING)
				paddingCount++;
		}

		B3D_TEST_ASSERT(paddingCount == 3 && description.GetVertexStride(0) == meshDescription.GetVertexStride(0) && meshDescription.GetVertexStride(0) == 48)
		B3D_TEST_ASSERT(description.HasElement(VES_TEXCOORD, 0, 0) && description.HasElement(VES_BLEND_WEIGHTS, 0, 0) && !description.HasElement(VES_POSITION, 0, 0))
		B3D_TEST_ASSERT(description.GetElementOffsetFromStream(VES_TEXCOORD, 0, 0) == meshDescription.GetElementOffsetFromStream(VES_TEXCOORD, 0, 0))
		B3D_TEST_ASSERT(description.HasElement(VES_POSITION, 0, 1) && description.HasElement(VES_NORMAL, 0, 2) && description.HasElement(VES_TANGENT, 0, 2) &&
			description.HasElement(VES_POSITION, 1, 3))
		B3D_TEST_ASSERT(description.GetVertexStride(1) == 12 && description.GetVertexStride(2) == 8 && description.GetVertexStride(3) == 12)

		// A vertex shader's inputs resolve to the entry's streams for the deformed attributes and to the mesh's for the others
		TInlineArray<VertexElement, 8> shaderInputElements;
		shaderInputElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
		shaderInputElements.Add(VertexElement(VET_FLOAT3, VES_NORMAL));
		shaderInputElements.Add(VertexElement(VET_FLOAT4, VES_TANGENT));
		shaderInputElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));
		const VertexDescription shaderInputDescription(shaderInputElements);

		GpuVertexInputLayout layout;
		layout.Resolve(description, shaderInputDescription);
		B3D_TEST_ASSERT(layout.Attributes.Size() == 4 && layout.NullStreamIndex == GpuVertexInputLayout::kNoNullStream && layout.StreamCount == 3)
		if(layout.Attributes.Size() == 4 && layout.StreamCount == 3)
		{
			B3D_TEST_ASSERT(layout.Attributes[0].StreamIndex == 1 && layout.Attributes[1].StreamIndex == 2 && layout.Attributes[2].StreamIndex == 2 &&
				layout.Attributes[3].StreamIndex == 0)
			B3D_TEST_ASSERT(layout.StreamStrides[0] == 48 && layout.StreamStrides[1] == 12 && layout.StreamStrides[2] == 8)
		}

		// The deformed mesh covers the right of the target, where the bind pose covers nothing, and leaves the middle empty, which the bind
		// pose covers
		B3D_TEST_ASSERT_MSG(targetRead.HasCompleted() && targetRead.GetReturnValue() != nullptr, "The draw target wasn't read.")
		if(!targetRead.HasCompleted() || targetRead.GetReturnValue() == nullptr)
			return;

		const TShared<PixelData>& pixels = targetRead.GetReturnValue();
		const auto fnGetPixel = [&pixels](u32 x, u32 y)
		{
			const u8* const pixel = pixels->GetData() + y * pixels->GetRowPitch() + x * 4;
			return Vector4(pixel[0], pixel[1], pixel[2], pixel[3]);
		};

		const Vector4 deformedPixel = fnGetPixel(14, kTargetSize / 2);
		const Vector4 bindPosePixel = fnGetPixel(6, kTargetSize / 2);
		const String pixelMessage = StringUtility::Format("Pixel (14, {0}) is ({1}, {2}, {3}, {4}), pixel (6, {0}) is ({5}, {6}, {7}, {8}).", kTargetSize / 2,
			deformedPixel.X, deformedPixel.Y, deformedPixel.Z, deformedPixel.W, bindPosePixel.X, bindPosePixel.Y, bindPosePixel.Z, bindPosePixel.W);

		// Texture coordinates come from the mesh's stream, and the normals and tangents the entry's buffers hold have unit length
		B3D_TEST_ASSERT_MSG(deformedPixel.X == 255.0f && std::abs(deformedPixel.Y - kUv.X * 255.0f) <= 1.0f && std::abs(deformedPixel.Z - kUv.Y * 255.0f) <= 1.0f &&
			deformedPixel.W >= 240.0f, pixelMessage)
		B3D_TEST_ASSERT_MSG(bindPosePixel == Vector4(0.0f, 0.0f, 0.0f, 0.0f), pixelMessage)
	}, "SkinCacheTestSuite::TestVertexLayout", true);
}

void SkinCacheTestSuite::TestFrameGraph()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuGraphicsPipelineState> pipeline = CreateTestSkinCacheDrawPipeline(*device);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		const TShared<MeshData> meshData = CreateTestMeshData();
		const TShared<render::Mesh> mesh = CreateTestMesh(*device, meshData, false);
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		SkinCache cache(*device, "Test skin cache");
		const TShared<SkinCacheEntry> entry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::None, "Single buffer"));
		const TShared<SkinCacheEntry> swappingEntry = cache.CreateEntry(SkinCacheEntryCreateInformation(mesh, SkinCacheEntryFlag::PreviousPositions, "Swapping buffers"));
		B3D_TEST_ASSERT(entry != nullptr && swappingEntry != nullptr && cache.GetEntryCount() == 2)
		if(entry == nullptr || swappingEntry == nullptr)
			return;

		const Vector<Matrix4> firstPose = CreatePose(1.0f);
		const Vector<Matrix4> secondPose = CreatePose(-0.5f);

		// Both entries are deformed by one pass, which nothing in the graph reads. The exports keep it from being culled.
		entry->SetPose(firstPose);
		swappingEntry->SetPose(firstPose);
		{
			FrameGraph graph(*context, "Skin cache deformation");
			B3D_TEST_ASSERT(cache.Update(graph))

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().PassCount == 1 && !graph.IsPassCulled(0))
			graph.Execute();
		}

		// A later graph draws the exported buffers at rest without declaring them, then deforms both entries again
		TAsyncOp<TShared<PixelData>> firstRead;
		{
			FrameGraph graph(*context, "Skin cache draw");
			const TShared<render::Texture> target = AddDrawPass(graph, pipeline, { entry->GetVertexData(), swappingEntry->GetVertexData() }, mesh);
			firstRead = TextureUtility::ReadAsync(graph, target);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		device->EndFrame();

		// A deformation after the draws overwrites the single buffer the draw read, and writes the second buffer of the other entry. The
		// same graph draws the new vertices after the deformation exported them.
		entry->SetPose(secondPose);
		swappingEntry->SetPose(secondPose);
		TAsyncOp<TShared<PixelData>> secondRead;
		{
			FrameGraph graph(*context, "Skin cache deformation and draw");
			B3D_TEST_ASSERT(cache.Update(graph))

			const TShared<render::Texture> target = AddDrawPass(graph, pipeline, { entry->GetVertexData(), swappingEntry->GetVertexData() }, mesh);
			secondRead = TextureUtility::ReadAsync(graph, target);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().PassCount == 3 && graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		device->WaitUntilIdle();
		device->EndFrame();

		B3D_TEST_ASSERT(firstRead.HasCompleted() && firstRead.GetReturnValue() != nullptr && secondRead.HasCompleted() && secondRead.GetReturnValue() != nullptr)

		const Vector<DeformedVertex> expectedVertices = DeformOnCpu(*meshData, secondPose);
		for(const TShared<SkinCacheEntry>& deformedEntry : { entry, swappingEntry })
		{
			const String mismatch = FindMismatch(expectedVertices, ReadDeformedVertices(*context, *deformedEntry->GetVertexData()), kPositionTolerance);
			B3D_TEST_ASSERT_MSG(mismatch.empty(), mismatch)
		}

		const String previousMismatch = FindPositionMismatch(DeformOnCpu(*meshData, firstPose), ReadPositions(*context,
			FindStreamBuffer(*swappingEntry->GetVertexData(), VES_POSITION, 1)));
		B3D_TEST_ASSERT_MSG(previousMismatch.empty(), StringUtility::Format("Previous positions: {0}", previousMismatch))
	}, "SkinCacheTestSuite::TestFrameGraph", true);
}
