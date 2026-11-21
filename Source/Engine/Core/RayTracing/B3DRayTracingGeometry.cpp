//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RayTracing/B3DRayTracingGeometry.h"

#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DSubMesh.h"
#include "GpuBackend/B3DVertexData.h"
#include "RayTracing/B3DRayTracingManager.h"
#include "RayTracing/B3DRayTracingScene.h"
#include "Utility/B3DBitwise.h"

namespace b3d::render
{
	namespace
	{
		/** Returns true if @p description has no element with @p semantic and semantic index 0, or its element is of @p type. */
		bool IsOptionalElementOfType(const VertexDescription& description, VertexElementSemantic semantic, VertexElementType type)
		{
			const VertexElement* element = description.FindElement(semantic);
			return element == nullptr || element->GetType() == type;
		}

		/** Describes where @p element of the vertices of @p vertexData lives, or returns an empty stream if @p element is null. */
		RayTracingVertexStream GetVertexStream(const VertexData& vertexData, const VertexElement* element)
		{
			RayTracingVertexStream stream;
			if(element == nullptr)
				return stream;

			const TShared<GpuBuffer> buffer = vertexData.GetBuffer(element->GetStreamIndex());
			if(buffer == nullptr)
				return stream;

			stream.BufferIndex = buffer->GetBindlessIndex();
			stream.Offset = element->GetOffset();
			stream.Stride = vertexData.VertexDescription->GetVertexStride(element->GetStreamIndex());

			return stream;
		}
	} // namespace

	bool RayTracingGeometry::IsSupportedFormat(const VertexDescription& vertexDescription, IndexType indexType)
	{
		if(indexType != kIndexType)
			return false;

		const VertexElement* position = vertexDescription.FindElement(VES_POSITION);
		if(position == nullptr || position->GetType() != kPositionType)
			return false;

		return IsOptionalElementOfType(vertexDescription, VES_NORMAL, kNormalType) && IsOptionalElementOfType(vertexDescription, VES_TANGENT, kTangentType) &&
			IsOptionalElementOfType(vertexDescription, VES_TEXCOORD, kUvType);
	}

	RayTracingGeometryLayout RayTracingGeometry::GetVertexLayout(const VertexData& vertexData, const TShared<GpuBuffer>& defaultAttributes)
	{
		const VertexDescription& description = *vertexData.VertexDescription;

		RayTracingGeometryLayout layout;
		layout.Position = GetVertexStream(vertexData, description.FindElement(VES_POSITION));
		layout.Normal = GetVertexStream(vertexData, description.FindElement(VES_NORMAL));
		layout.Tangent = GetVertexStream(vertexData, description.FindElement(VES_TANGENT));
		layout.Uv = GetVertexStream(vertexData, description.FindElement(VES_TEXCOORD));

		if(defaultAttributes == nullptr)
			return layout;

		const u32 defaultAttributesBufferIndex = defaultAttributes->GetBindlessIndex();
		if(layout.Normal.Stride == 0)
		{
			layout.Normal.BufferIndex = defaultAttributesBufferIndex;
			layout.Normal.Offset = offsetof(RayTracingDefaultAttributes, Normal);
		}

		if(layout.Tangent.Stride == 0)
		{
			layout.Tangent.BufferIndex = defaultAttributesBufferIndex;
			layout.Tangent.Offset = offsetof(RayTracingDefaultAttributes, Tangent);
		}

		if(layout.Uv.Stride == 0)
		{
			layout.Uv.BufferIndex = defaultAttributesBufferIndex;
			layout.Uv.Offset = offsetof(RayTracingDefaultAttributes, Uv);
		}

		return layout;
	}

	void RayTracingGeometry::SetVertexData(const VertexData& vertexData)
	{
		if(!B3D_ENSURE_LOG(mFlags.IsSet(RayTracingGeometryFlag::Dynamic), "Only dynamic ray tracing geometry can replace its vertex buffers."))
			return;

		const bool isSameDescription = vertexData.VertexDescription == mVertexDescription || (vertexData.VertexDescription != nullptr && *vertexData.VertexDescription == *mVertexDescription);
		if(!B3D_ENSURE_LOG(isSameDescription && vertexData.VertexCount == mVertexCount, "Ray tracing geometry needs vertices with the description and the number of vertices it was created with."))
			return;

		const TShared<GpuBuffer> positionBuffer = vertexData.GetBuffer(mVertexDescription->FindElement(VES_POSITION)->GetStreamIndex());
		if(!B3D_ENSURE_LOG(positionBuffer != nullptr, "Ray tracing geometry needs vertices with a buffer bound to the stream of their positions."))
			return;

		// The geometries keep their vertex offsets and strides, as the description is the same, and the layouts their index ranges
		for(GpuRayTracingGeometry& geometry : mGeometries)
			geometry.VertexBuffer = positionBuffer;

		const RayTracingGeometryLayout vertexLayout = GetVertexLayout(vertexData, mDefaultAttributes);
		for(RayTracingGeometryLayout& layout : mLayouts)
		{
			layout.Position = vertexLayout.Position;
			layout.Normal = vertexLayout.Normal;
			layout.Tangent = vertexLayout.Tangent;
			layout.Uv = vertexLayout.Uv;
		}
	}

	void RayTracingGeometry::Build(FrameGraph& graph)
	{
		RayTracingGeometry* geometry = this;
		AddBuildPass(graph, "Build ray tracing geometry", TArrayView<RayTracingGeometry* const>(&geometry, 1), FrameGraphAccelerationStructurePass());
	}

	void RayTracingGeometry::AddBuildPass(FrameGraph& graph, const StringView& name, TArrayView<RayTracingGeometry* const> geometries, FrameGraphAccelerationStructurePass&& pass)
	{
		if(geometries.IsEmpty())
			return;

		FrameAllocatorScope frameScope;
		const u32 scratchOffsetAlignment = graph.GetWorkContext().GetDevice().GetCapabilities().RayTracing.ScratchOffsetAlignment;
		FrameVector<u32> scratchOffsets;
		u32 scratchSize = 0;
		for(const RayTracingGeometry* geometry : geometries)
		{
			const u32 scratchOffset = Bitwise::AlignUp(scratchSize, scratchOffsetAlignment);
			scratchOffsets.push_back(scratchOffset);
			scratchSize = scratchOffset + geometry->mBuildScratchSize;
		}

		const TShared<GpuBuffer> scratch = graph.CreateBuffer("Ray tracing geometry scratch", GpuBufferCreateInformation::CreateStructuredStorage(4, std::max(1u, Math::DivideAndRoundUp(scratchSize, 4u)),
			GpuBufferFlag::GpuWrite));

		Vector<GpuAccelerationStructureBuild> builds;
		builds.reserve((size_t)geometries.Size());
		for(u32 geometryIndex = 0; geometryIndex < (u32)geometries.Size(); geometryIndex++)
		{
			RayTracingGeometry& geometry = *geometries[geometryIndex];
			graph.ImportAccelerationStructure(geometry.mAccelerationStructure);

			builds.push_back(GpuAccelerationStructureBuild(geometry.mInputs, geometry.mAccelerationStructure, scratch, scratchOffsets[geometryIndex]));
			pass.AddBuild(builds.back());
			pass.Export(geometry.mAccelerationStructure);

			// Scenes with instances of the geometry rebuild their top level structure over the new contents
			geometry.mIsBuilt = true;
			geometry.mVersion++;
			for(RayTracingScene* scene : geometry.mScenes)
				scene->NotifyGeometryChanged(geometry);
		}

		graph.AddAccelerationStructurePass(name, std::move(pass), [builds = std::move(builds)](FrameGraphPassContext& passContext)
		{
			passContext.GetCommandBuffer().BuildAccelerationStructures(builds);
		});
	}

	void RayTracingGeometry::MarkOutOfDate()
	{
		if(!mIsBuilt)
			return;

		mIsBuilt = false;

		for(RayTracingScene* scene : mScenes)
			scene->NotifyGeometryChanged(*this);
	}

	void RayTracingGeometry::NotifyAddedToScene(RayTracingScene& scene)
	{
		mScenes.Add(&scene);
	}

	void RayTracingGeometry::NotifyRemovedFromScene(RayTracingScene& scene)
	{
		const auto sceneIterator = std::find(mScenes.begin(), mScenes.end(), &scene);
		if(sceneIterator != mScenes.end())
			mScenes.SwapAndErase(sceneIterator);
	}

	TShared<RayTracingGeometry> RayTracingGeometry::Create(GpuDevice& device, const VertexData& vertexData, const TShared<GpuBuffer>& indexBuffer, TArrayView<const SubMesh> subMeshes, RayTracingGeometryFlags flags)
	{
		if(indexBuffer == nullptr)
			return nullptr;

		const VertexDescription& description = *vertexData.VertexDescription;
		if(!IsSupportedFormat(description, indexBuffer->GetInformation().Index.Type))
		{
			B3D_LOG(Warning, LogRenderer, "Ray tracing geometry needs vertices and indices in the formats RayTracingGeometry lists.");
			return nullptr;
		}

		const VertexElement* position = description.FindElement(VES_POSITION);
		const TShared<GpuBuffer> positionBuffer = vertexData.GetBuffer(position->GetStreamIndex());
		if(positionBuffer == nullptr)
			return nullptr;

		// Attributes the vertices lack point at one value every vertex shares, so shaders read them without checking
		RayTracingGeometryLayout meshLayout = GetVertexLayout(vertexData, nullptr);
		TShared<GpuBuffer> defaultAttributes;
		if(meshLayout.Normal.Stride == 0 || meshLayout.Tangent.Stride == 0 || meshLayout.Uv.Stride == 0)
		{
			defaultAttributes = RayTracingManager::Instance().GetDefaultAttributes(device);
			if(defaultAttributes == nullptr && device.GetBindlessHeaps() != nullptr)
				return nullptr;

			meshLayout = GetVertexLayout(vertexData, defaultAttributes);
		}

		meshLayout.IndexBufferIndex = indexBuffer->GetBindlessIndex();

		const u32 indexSize = b3d::GpuBuffer::GetIndexSize(kIndexType);
		const u32 vertexStride = description.GetVertexStride(position->GetStreamIndex());

		const TShared<RayTracingGeometry> geometry = B3DMakeSharedFromExisting<RayTracingGeometry>(new(B3DAllocate<RayTracingGeometry>()) RayTracingGeometry());
		geometry->mFlags = flags;
		geometry->mVertexDescription = vertexData.VertexDescription;
		geometry->mVertexCount = vertexData.VertexCount;
		geometry->mDefaultAttributes = defaultAttributes;
		for(const SubMesh& subMesh : subMeshes)
		{
			// Each triangle list sub-mesh becomes one geometry, which rays report as its geometry index
			const u32 triangleIndexCount = subMesh.IndexCount / 3 * 3;
			if(subMesh.DrawOp != DOT_TRIANGLE_LIST || triangleIndexCount == 0)
			{
				geometry->mSubMeshGeometryIndices.push_back(~0u);
				continue;
			}

			geometry->mSubMeshGeometryIndices.push_back((u32)geometry->mGeometries.size());

			GpuRayTracingGeometry triangles(positionBuffer, vertexStride, vertexData.VertexCount, indexBuffer, triangleIndexCount, kIndexType);
			triangles.VertexOffset = position->GetOffset();
			triangles.IndexOffset = subMesh.IndexOffset * indexSize;
			geometry->mGeometries.push_back(triangles);

			RayTracingGeometryLayout layout = meshLayout;
			layout.FirstIndex = subMesh.IndexOffset;
			geometry->mLayouts.push_back(layout);
		}

		if(geometry->mGeometries.empty())
			return nullptr;

		// Moving vertices are built often, so they trade ray tracing speed for build speed
		const GpuAccelerationStructureBuildFlags buildFlags = flags.IsSet(RayTracingGeometryFlag::Dynamic) ? GpuAccelerationStructureBuildFlag::PreferFastBuild : GpuAccelerationStructureBuildFlag::PreferFastTrace;
		geometry->mInputs = GpuAccelerationStructureInputs(geometry->mGeometries, buildFlags);

		GpuAccelerationStructureSizes sizes;
		if(!device.GetAccelerationStructureSizes(geometry->mInputs, sizes))
			return nullptr;

		geometry->mBuildScratchSize = sizes.BuildScratchSize;
		geometry->mAccelerationStructure = device.CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(GpuAccelerationStructureType::BottomLevel,
			sizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "Mesh geometry"));

		if(geometry->mAccelerationStructure == nullptr)
			return nullptr;

		return geometry;
	}
} // namespace b3d::render
