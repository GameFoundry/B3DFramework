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

	void RayTracingGeometry::Build(FrameGraph& graph)
	{
		graph.ImportAccelerationStructure(mAccelerationStructure);

		const TShared<GpuBuffer> scratch = graph.CreateBuffer("Ray tracing geometry scratch",
			GpuBufferCreateInformation::CreateStructuredStorage(4, Math::DivideAndRoundUp(mBuildScratchSize, 4u), GpuBufferFlag::GpuWrite));

		const GpuAccelerationStructureBuild build(mInputs, mAccelerationStructure, scratch);

		FrameGraphAccelerationStructurePass pass;
		pass.AddBuild(build);
		pass.Export(mAccelerationStructure);

		graph.AddAccelerationStructurePass("Build ray tracing geometry", std::move(pass), [build](FrameGraphPassContext& passContext)
		{
			passContext.GetCommandBuffer().BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1));
		});

		mIsBuilt = true;
	}

	TShared<RayTracingGeometry> RayTracingGeometry::Create(GpuDevice& device, const VertexData& vertexData, const TShared<GpuBuffer>& indexBuffer,
		TArrayView<const SubMesh> subMeshes)
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

		RayTracingGeometryLayout meshLayout;
		meshLayout.IndexBufferIndex = indexBuffer->GetBindlessIndex();
		meshLayout.Position = GetVertexStream(vertexData, position);
		meshLayout.Normal = GetVertexStream(vertexData, description.FindElement(VES_NORMAL));
		meshLayout.Tangent = GetVertexStream(vertexData, description.FindElement(VES_TANGENT));
		meshLayout.Uv = GetVertexStream(vertexData, description.FindElement(VES_TEXCOORD));

		const TShared<GpuBuffer> positionBuffer = vertexData.GetBuffer(position->GetStreamIndex());
		if(positionBuffer == nullptr)
			return nullptr;

		// Attributes the vertices lack point at one value every vertex shares, so shaders read them without checking
		TShared<GpuBuffer> defaultAttributes;
		if(meshLayout.Normal.Stride == 0 || meshLayout.Tangent.Stride == 0 || meshLayout.Uv.Stride == 0)
		{
			defaultAttributes = RayTracingManager::Instance().GetDefaultAttributes(device);
			if(defaultAttributes == nullptr && device.GetBindlessHeaps() != nullptr)
				return nullptr;
		}

		if(defaultAttributes != nullptr)
		{
			const u32 bufferIndex = defaultAttributes->GetBindlessIndex();
			if(meshLayout.Normal.Stride == 0)
			{
				meshLayout.Normal.BufferIndex = bufferIndex;
				meshLayout.Normal.Offset = offsetof(RayTracingDefaultAttributes, Normal);
			}

			if(meshLayout.Tangent.Stride == 0)
			{
				meshLayout.Tangent.BufferIndex = bufferIndex;
				meshLayout.Tangent.Offset = offsetof(RayTracingDefaultAttributes, Tangent);
			}

			if(meshLayout.Uv.Stride == 0)
			{
				meshLayout.Uv.BufferIndex = bufferIndex;
				meshLayout.Uv.Offset = offsetof(RayTracingDefaultAttributes, Uv);
			}
		}

		const u32 indexSize = b3d::GpuBuffer::GetIndexSize(kIndexType);
		const u32 vertexStride = description.GetVertexStride(position->GetStreamIndex());

		const TShared<RayTracingGeometry> geometry = B3DMakeSharedFromExisting<RayTracingGeometry>(new(B3DAllocate<RayTracingGeometry>()) RayTracingGeometry());
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

		geometry->mInputs = GpuAccelerationStructureInputs(geometry->mGeometries,
			GpuAccelerationStructureBuildFlag::PreferFastTrace);

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
