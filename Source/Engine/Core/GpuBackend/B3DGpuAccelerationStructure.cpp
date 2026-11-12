//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuAccelerationStructure.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d::render
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	namespace
	{
		/** Checks that @p buffer can be read by acceleration structure builds as @p usage, and logs the reason if it can't. */
		bool ValidateInputBuffer(const GpuBuffer& buffer, GpuBufferType expectedType, const char* usage)
		{
			const GpuBufferInformation& information = buffer.GetInformation();
			if(!B3D_ENSURE_LOG(information.Type == expectedType || information.Type == GpuBufferType::StructuredStorage,
				"The {0} buffer '{1}' of an acceleration structure has the wrong type.", usage, buffer.GetName()))
			{
				return false;
			}

			return B3D_ENSURE_LOG(information.Flags.IsSet(GpuBufferFlag::AccelerationStructureInput),
				"The {0} buffer '{1}' of an acceleration structure was not created with GpuBufferFlag::AccelerationStructureInput.", usage, buffer.GetName());
		}

		/** Checks one geometry of a bottom level acceleration structure, and logs the reason if it is invalid. */
		bool ValidateGeometry(const GpuRayTracingGeometry& geometry, u32 geometryIndex, u64& inOutPrimitiveCount)
		{
			if(!B3D_ENSURE_LOG(geometry.VertexBuffer != nullptr, "Geometry {0} of an acceleration structure has no vertex buffer.", geometryIndex))
				return false;

			if(!ValidateInputBuffer(*geometry.VertexBuffer, GpuBufferType::Vertex, "vertex"))
				return false;

			if(!B3D_ENSURE_LOG(geometry.VertexType == VET_FLOAT3, "Geometry {0} of an acceleration structure must have VET_FLOAT3 vertex positions.", geometryIndex))
				return false;

			static constexpr u32 kPositionSize = 3 * sizeof(float);
			if(!B3D_ENSURE_LOG(geometry.VertexOffset % 4 == 0 && geometry.VertexStride % 4 == 0 && geometry.VertexStride >= kPositionSize,
				"Geometry {0} of an acceleration structure has vertex offset {1} and stride {2}. Both must be multiples of 4, and the stride at least {3}.",
				geometryIndex, geometry.VertexOffset, geometry.VertexStride, kPositionSize))
			{
				return false;
			}

			if(!B3D_ENSURE_LOG(geometry.VertexCount > 0, "Geometry {0} of an acceleration structure has no vertices.", geometryIndex))
				return false;

			const u64 vertexEnd = (u64)geometry.VertexOffset + (u64)(geometry.VertexCount - 1) * geometry.VertexStride + kPositionSize;
			if(!B3D_ENSURE_LOG(vertexEnd <= geometry.VertexBuffer->GetTotalSize(), "The vertices of geometry {0} of an acceleration structure end past the end of buffer '{1}'.",
				geometryIndex, geometry.VertexBuffer->GetName()))
			{
				return false;
			}

			if(geometry.IndexBuffer == nullptr)
			{
				if(!B3D_ENSURE_LOG(geometry.VertexCount % 3 == 0, "Geometry {0} of an acceleration structure has no index buffer, so its vertex count must be a multiple of 3.", geometryIndex))
					return false;

				inOutPrimitiveCount += geometry.VertexCount / 3;
				return true;
			}

			if(!ValidateInputBuffer(*geometry.IndexBuffer, GpuBufferType::Index, "index"))
				return false;

			const u32 indexSize = b3d::GpuBuffer::GetIndexSize(geometry.IndexType);
			if(!B3D_ENSURE_LOG(geometry.IndexOffset % indexSize == 0, "Geometry {0} of an acceleration structure has index offset {1}, which must be a multiple of the index size.",
				geometryIndex, geometry.IndexOffset))
			{
				return false;
			}

			if(!B3D_ENSURE_LOG(geometry.IndexCount > 0 && geometry.IndexCount % 3 == 0, "Geometry {0} of an acceleration structure has {1} indices, which must be a non-zero multiple of 3.",
				geometryIndex, geometry.IndexCount))
			{
				return false;
			}

			const u64 indexEnd = (u64)geometry.IndexOffset + (u64)geometry.IndexCount * indexSize;
			if(!B3D_ENSURE_LOG(indexEnd <= geometry.IndexBuffer->GetTotalSize(), "The indices of geometry {0} of an acceleration structure end past the end of buffer '{1}'.",
				geometryIndex, geometry.IndexBuffer->GetName()))
			{
				return false;
			}

			inOutPrimitiveCount += geometry.IndexCount / 3;
			return true;
		}
	} // namespace
#endif

	GpuAccelerationStructure::GpuAccelerationStructure(GpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation, IGpuResource& gpuResource)
		: mDevice(device), mGpuResource(&gpuResource), mType(createInformation.Type), mSize(createInformation.Size), mFlags(createInformation.Flags)
		, mName(createInformation.Name)
	{ }

	GpuAccelerationStructure::~GpuAccelerationStructure()
	{
		// Before the native resource is destroyed, since the slot refers to it
		mBindlessSlots.Unregister(*mGpuResource);

		UnpinBottomLevels();

		// Freed once the GPU work and the pins that still use it complete
		mGpuResource->Destroy();
	}

	void GpuAccelerationStructure::Initialize()
	{
		if(mFlags.IsSet(GpuAccelerationStructureFlag::Bindless))
			mBindlessSlots.Register(mDevice, *mGpuResource, GpuBindlessResourceView(*this));
	}

	void GpuAccelerationStructure::SetBottomLevels(TArrayView<const TShared<GpuAccelerationStructure>> bottomLevels)
	{
		// Pinned before the previous set is released, so structures in both sets stay alive
		TArray<IGpuResource*> pinnedBottomLevels;
		pinnedBottomLevels.Reserve((u32)bottomLevels.size());
		for(const TShared<GpuAccelerationStructure>& bottomLevel : bottomLevels)
		{
			if(bottomLevel == nullptr)
				continue;

			mDevice.PinResource(*bottomLevel->mGpuResource);
			pinnedBottomLevels.Add(bottomLevel->mGpuResource);
		}

		UnpinBottomLevels();
		mPinnedBottomLevels = std::move(pinnedBottomLevels);
	}

	void GpuAccelerationStructure::UnpinBottomLevels()
	{
		for(IGpuResource* nativeResource : mPinnedBottomLevels)
			mDevice.UnpinResource(*nativeResource);

		mPinnedBottomLevels.Clear();
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	bool GpuAccelerationStructure::ValidateInputs(const GpuDevice& device, const GpuAccelerationStructureInputs& inputs)
	{
		const GpuRayTracingCapabilities& capabilities = device.GetCapabilities().RayTracing;

		const GpuAccelerationStructureBuildFlags preferences = GpuAccelerationStructureBuildFlag::PreferFastTrace | GpuAccelerationStructureBuildFlag::PreferFastBuild;
		if(!B3D_ENSURE_LOG((inputs.Flags & preferences) != preferences, "An acceleration structure cannot prefer both a fast trace and a fast build."))
			return false;

		if(inputs.Type == GpuAccelerationStructureType::BottomLevel)
		{
			if(!B3D_ENSURE_LOG(inputs.InstanceBuffer == nullptr && inputs.InstanceCount == 0, "A bottom level acceleration structure cannot have instances."))
				return false;

			if(!B3D_ENSURE_LOG(!inputs.Geometries.IsEmpty() && inputs.Geometries.Size() <= capabilities.MaximumGeometryCount,
				"A bottom level acceleration structure has {0} geometries. It must have at least one and at most {1}.", inputs.Geometries.Size(), capabilities.MaximumGeometryCount))
			{
				return false;
			}

			u64 primitiveCount = 0;
			for(u32 geometryIndex = 0; geometryIndex < (u32)inputs.Geometries.Size(); geometryIndex++)
			{
				if(!ValidateGeometry(inputs.Geometries[geometryIndex], geometryIndex, primitiveCount))
					return false;
			}

			return B3D_ENSURE_LOG(primitiveCount <= capabilities.MaximumPrimitiveCount, "A bottom level acceleration structure has {0} triangles, more than the {1} the device supports.",
				primitiveCount, capabilities.MaximumPrimitiveCount);
		}

		if(!B3D_ENSURE_LOG(inputs.Geometries.IsEmpty(), "A top level acceleration structure cannot have geometries."))
			return false;

		if(!B3D_ENSURE_LOG(inputs.InstanceCount <= capabilities.MaximumInstanceCount, "A top level acceleration structure has {0} instances, more than the {1} the device supports.",
			inputs.InstanceCount, capabilities.MaximumInstanceCount))
		{
			return false;
		}

		if(inputs.InstanceCount == 0 && inputs.InstanceBuffer == nullptr)
			return true;

		if(!B3D_ENSURE_LOG(inputs.InstanceBuffer != nullptr, "A top level acceleration structure with instances has no instance buffer."))
			return false;

		if(!ValidateInputBuffer(*inputs.InstanceBuffer, GpuBufferType::StructuredStorage, "instance"))
			return false;

		if(!B3D_ENSURE_LOG(inputs.InstanceOffset % capabilities.InstanceSize == 0, "A top level acceleration structure has instance offset {0}, which must be a multiple of {1}.",
			inputs.InstanceOffset, capabilities.InstanceSize))
		{
			return false;
		}

		const u64 instanceEnd = (u64)inputs.InstanceOffset + (u64)inputs.InstanceCount * capabilities.InstanceSize;
		return B3D_ENSURE_LOG(instanceEnd <= inputs.InstanceBuffer->GetTotalSize(), "The instances of a top level acceleration structure end past the end of buffer '{0}'.",
			inputs.InstanceBuffer->GetName());
	}
#endif
} // namespace b3d::render
