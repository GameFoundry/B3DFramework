//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RayTracing/B3DRayTracingScene.h"

#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "RayTracing/B3DRayTracingGeometry.h"
#include "Utility/B3DBitwise.h"

namespace b3d::render
{
	namespace
	{
		/** Smallest number of instances a top level structure is sized for. */
		constexpr u32 kMinimumCapacity = 16;

		/** Returns a structured buffer of @p size bytes that acceleration structure builds can use as scratch memory, created by @p graph. */
		TShared<GpuBuffer> CreateScratchBuffer(FrameGraph& graph, const char* name, u32 size)
		{
			return graph.CreateBuffer(name, GpuBufferCreateInformation::CreateStructuredStorage(4, std::max(1u, Math::DivideAndRoundUp(size, 4u)), GpuBufferFlag::GpuWrite));
		}
	} // namespace

	RayTracingScene::RayTracingScene(GpuDevice& device, const StringView& name)
		: mDevice(device), mName(name)
	{ }

	u32 RayTracingScene::AddInstance(const RayTracingGeometryInstance& instance)
	{
		u32 slotIndex = mFirstFreeSlot;
		if(slotIndex != ~0u)
			mFirstFreeSlot = mSlots[slotIndex].NextFreeSlot;
		else
		{
			slotIndex = (u32)mSlots.size();
			mSlots.emplace_back();
		}

		Slot& slot = mSlots[slotIndex];
		slot.Instance = instance;
		slot.IsUsed = true;
		slot.NextFreeSlot = ~0u;

		mInstanceCount++;
		mIsDirty = true;
		return slotIndex;
	}

	void RayTracingScene::UpdateInstance(u32 slot, const RayTracingGeometryInstance& instance)
	{
		if(!B3D_ENSURE_LOG(slot < (u32)mSlots.size() && mSlots[slot].IsUsed, "Ray tracing scene '{0}' has no instance in slot {1}.", mName, slot))
			return;

		mSlots[slot].Instance = instance;
		mIsDirty = true;
	}

	void RayTracingScene::SetInstanceTransform(u32 slot, const Matrix4& transform)
	{
		if(!B3D_ENSURE_LOG(slot < (u32)mSlots.size() && mSlots[slot].IsUsed, "Ray tracing scene '{0}' has no instance in slot {1}.", mName, slot))
			return;

		mSlots[slot].Instance.Transform = transform;
		mIsDirty = true;
	}

	void RayTracingScene::RemoveInstance(u32 slot)
	{
		if(!B3D_ENSURE_LOG(slot < (u32)mSlots.size() && mSlots[slot].IsUsed, "Ray tracing scene '{0}' has no instance in slot {1}.", mName, slot))
			return;

		// The free slot drops its geometry, which may be the last reference to it
		Slot& freeSlot = mSlots[slot];
		freeSlot.Instance = RayTracingGeometryInstance();
		freeSlot.IsUsed = false;
		freeSlot.NextFreeSlot = mFirstFreeSlot;
		mFirstFreeSlot = slot;

		mInstanceCount--;
		mIsDirty = true;
	}

	void RayTracingScene::Build(FrameGraph& graph)
	{
		const GpuRayTracingCapabilities& rayTracing = mDevice.GetCapabilities().RayTracing;
		FrameAllocatorScope frameScope;

		// The bottom level structures of the geometries that aren't built go in one pass, each at its own offset of one scratch buffer.
		// Marking a geometry built right away keeps instances that share it from adding it twice.
		FrameVector<RayTracingGeometry*> pendingGeometries;
		for(const Slot& slot : mSlots)
		{
			RayTracingGeometry* geometry = slot.Instance.Geometry.get();
			if(!slot.IsUsed || geometry == nullptr || geometry->IsBuilt())
				continue;

			pendingGeometries.push_back(geometry);
			geometry->MarkBuilt();
		}

		if(!pendingGeometries.empty())
		{
			FrameVector<u32> scratchOffsets;
			u32 scratchSize = 0;
			for(RayTracingGeometry* geometry : pendingGeometries)
			{
				const u32 scratchOffset = Bitwise::AlignUp(scratchSize, rayTracing.ScratchOffsetAlignment);
				scratchOffsets.push_back(scratchOffset);
				scratchSize = scratchOffset + geometry->GetBuildScratchSize();
			}

			const TShared<GpuBuffer> scratch = CreateScratchBuffer(graph, "Ray tracing geometry scratch", scratchSize);

			FrameGraphAccelerationStructurePass pass;
			Vector<GpuAccelerationStructureBuild> builds;
			builds.reserve(pendingGeometries.size());
			for(u32 geometryIndex = 0; geometryIndex < (u32)pendingGeometries.size(); geometryIndex++)
			{
				const RayTracingGeometry& geometry = *pendingGeometries[geometryIndex];
				const TShared<GpuAccelerationStructure>& bottomLevel = geometry.GetAccelerationStructure();
				graph.ImportAccelerationStructure(bottomLevel);

				builds.push_back(GpuAccelerationStructureBuild(geometry.GetBuildInputs(), bottomLevel, scratch, scratchOffsets[geometryIndex]));
				pass.AddBuild(builds.back());
				pass.Export(bottomLevel);
			}

			graph.AddAccelerationStructurePass("Build ray tracing geometries", std::move(pass), [builds = std::move(builds)](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().BuildAccelerationStructures(builds);
			});

			// The top level structure must see the new contents of the bottom levels
			mIsDirty = true;
		}

		if(!mIsDirty && mAccelerationStructure != nullptr)
			return;

		// Instances are packed densely, so the instance index rays report changes as instances come and go, and the instance id doesn't
		FrameVector<GpuRayTracingInstance> instances;
		instances.reserve(mInstanceCount);
		mBottomLevels.clear();
		for(const Slot& slot : mSlots)
		{
			const RayTracingGeometryInstance& geometryInstance = slot.Instance;
			if(!slot.IsUsed || geometryInstance.Geometry == nullptr)
				continue;

			const TShared<GpuAccelerationStructure>& bottomLevel = geometryInstance.Geometry->GetAccelerationStructure();

			GpuRayTracingInstance instance(bottomLevel.get(), geometryInstance.Transform, geometryInstance.InstanceId);
			instance.HitGroupIndexOffset = geometryInstance.HitGroupIndexOffset;
			instance.Mask = geometryInstance.Mask;
			instance.Flags = geometryInstance.Flags;
			instances.push_back(instance);

			mBottomLevels.push_back(bottomLevel);
		}

		// Instances that share a geometry share its bottom level structure, which the build needs only once
		std::sort(mBottomLevels.begin(), mBottomLevels.end());
		mBottomLevels.erase(std::unique(mBottomLevels.begin(), mBottomLevels.end()), mBottomLevels.end());

		// The instance buffer has room for the capacity, so the size query of a larger structure can use it
		const u32 instanceCount = (u32)instances.size();
		const bool needsAccelerationStructure = mAccelerationStructure == nullptr || instanceCount > mCapacity;
		const u32 capacity = needsAccelerationStructure ? std::max(std::max(instanceCount, mCapacity * 2), kMinimumCapacity) : mCapacity;

		const TShared<GpuBuffer> instanceBuffer = graph.GetWorkContext().CreateScratchGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(rayTracing.InstanceSize,
			capacity, GpuBufferFlag::AccelerationStructureInput, GpuBufferMemory::Mapped));
		if(!B3D_ENSURE_LOG(instanceBuffer != nullptr, "Ray tracing scene '{0}' couldn't create its instance buffer.", mName))
			return;

		instanceBuffer->SetName("Ray tracing scene instances");
		if(instanceCount > 0)
		{
			const GpuBufferMappedScope mapping = instanceBuffer->Map(GpuMapOption::Write);
			if(!B3D_ENSURE_LOG(mapping.IsValid(), "Ray tracing scene '{0}' couldn't map its instance buffer.", mName))
				return;

			if(!mDevice.WriteRayTracingInstances(instances, mapping.GetMappedMemory()))
				return;
		}

		if(needsAccelerationStructure)
		{
			GpuAccelerationStructureSizes sizes;
			if(!mDevice.GetAccelerationStructureSizes(GpuAccelerationStructureInputs(instanceBuffer, capacity), sizes))
				return;

			const GpuAccelerationStructureFlags flags = rayTracing.SupportsBindlessAccelerationStructures ? GpuAccelerationStructureFlag::Bindless : GpuAccelerationStructureFlag::None;
			const TShared<GpuAccelerationStructure> accelerationStructure = mDevice.CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
				GpuAccelerationStructureType::TopLevel, sizes.AccelerationStructureSize, flags, mName));
			if(accelerationStructure == nullptr)
				return;

			mAccelerationStructure = accelerationStructure;
			mCapacity = capacity;
			mBuildScratchSize = sizes.BuildScratchSize;
		}

		graph.ImportAccelerationStructure(mAccelerationStructure);

		// A build with fewer instances than the capacity needs no more scratch memory than one with the capacity
		GpuAccelerationStructureBuild build(GpuAccelerationStructureInputs(instanceBuffer, instanceCount), mAccelerationStructure,
			CreateScratchBuffer(graph, "Ray tracing scene scratch", mBuildScratchSize));
		build.BottomLevels = mBottomLevels;

		FrameGraphAccelerationStructurePass pass;
		pass.AddBuild(build);
		pass.Export(mAccelerationStructure);

		graph.AddAccelerationStructurePass("Build ray tracing scene", std::move(pass), [build](FrameGraphPassContext& passContext)
		{
			passContext.GetCommandBuffer().BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1));
		});

		mIsDirty = false;
	}
} // namespace b3d::render
