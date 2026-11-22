//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RayTracing/B3DRayTracingScene.h"

#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "RayTracing/B3DRayTracingGeometry.h"

namespace b3d::render
{
	namespace
	{
		/** Smallest number of instances a top level structure is sized for. */
		constexpr u32 kMinimumCapacity = 16;
	} // namespace

	RayTracingScene::RayTracingScene(GpuDevice& device, const StringView& name)
		: mDevice(device), mName(name), mInstanceSize(device.GetCapabilities().RayTracing.InstanceSize)
	{ }

	RayTracingScene::~RayTracingScene()
	{
		for(RayTracingGeometry* geometry : mGeometries)
			geometry->NotifyRemovedFromScene(*this);
	}

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
		PackInstance(slotIndex);

		mInstanceCount++;
		mIsDirty = true;
		return slotIndex;
	}

	void RayTracingScene::UpdateInstance(u32 slot, const RayTracingGeometryInstance& instance)
	{
		if(!B3D_ENSURE_LOG(slot < (u32)mSlots.size() && mSlots[slot].IsUsed, "Ray tracing scene '{0}' has no instance in slot {1}.", mName, slot))
			return;

		UnpackInstance(slot);
		mSlots[slot].Instance = instance;
		PackInstance(slot);
		mIsDirty = true;
	}

	void RayTracingScene::SetInstanceTransform(u32 slot, const Matrix4& transform)
	{
		if(!B3D_ENSURE_LOG(slot < (u32)mSlots.size() && mSlots[slot].IsUsed, "Ray tracing scene '{0}' has no instance in slot {1}.", mName, slot))
			return;

		Slot& usedSlot = mSlots[slot];
		usedSlot.Instance.Transform = transform;
		if(usedSlot.PackedIndex != ~0u && !TryWriteInstance(usedSlot.Instance, &mPackedInstances[(size_t)usedSlot.PackedIndex * mInstanceSize]))
			UnpackInstance(slot);

		mIsDirty = true;
	}

	void RayTracingScene::RemoveInstance(u32 slot)
	{
		if(!B3D_ENSURE_LOG(slot < (u32)mSlots.size() && mSlots[slot].IsUsed, "Ray tracing scene '{0}' has no instance in slot {1}.", mName, slot))
			return;

		UnpackInstance(slot);

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

		// The geometries that still aren't built go in one pass. Sorting drops the ones the list holds more than once. Their build marks
		// the scene dirty, as it does every scene with instances of them.
		if(!mPendingGeometries.empty())
		{
			FrameAllocatorScope frameScope;
			FrameVector<RayTracingGeometry*> geometries;
			for(RayTracingGeometry* geometry : mPendingGeometries)
			{
				if(!geometry->IsBuilt())
					geometries.push_back(geometry);
			}

			mPendingGeometries.clear();
			std::sort(geometries.begin(), geometries.end());
			geometries.erase(std::unique(geometries.begin(), geometries.end()), geometries.end());

			RayTracingGeometry::AddBuildPass(graph, "Build ray tracing geometries", geometries, TArrayView<RayTracingGeometry* const>(), FrameGraphAccelerationStructurePass());
		}

		if(!mIsDirty && mAccelerationStructure != nullptr)
			return;

		// The instance buffer has room for the capacity, so the size query of a larger structure can use it
		const u32 instanceCount = (u32)mPackedInstanceSlots.Size();
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

			std::memcpy(mapping.GetMappedMemory(), mPackedInstances.data(), mPackedInstances.size());
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

		// The build reads its list of bottom level structures when the graph executes, after instances may have come and gone, so it
		// reads a copy, taken only when the list changed
		if(mBottomLevelsChanged)
		{
			mBuiltBottomLevels = mBottomLevels;
			mBottomLevelsChanged = false;
		}

		graph.ImportAccelerationStructure(mAccelerationStructure);

		// A build with fewer instances than the capacity needs no more scratch memory than one with the capacity
		const TShared<GpuBuffer> scratch = graph.CreateBuffer("Ray tracing scene scratch", GpuBufferCreateInformation::CreateStructuredStorage(4,
			std::max(1u, Math::DivideAndRoundUp(mBuildScratchSize, 4u)), GpuBufferFlag::GpuWrite));
		GpuAccelerationStructureBuild build(GpuAccelerationStructureInputs(instanceBuffer, instanceCount), mAccelerationStructure, scratch);
		build.BottomLevels = mBuiltBottomLevels;

		FrameGraphAccelerationStructurePass pass;
		pass.AddBuild(build);
		pass.Export(mAccelerationStructure);

		graph.AddAccelerationStructurePass("Build ray tracing scene", std::move(pass), [build](FrameGraphPassContext& passContext)
		{
			passContext.GetCommandBuffer().BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1));
		});

		mIsDirty = false;
	}

	void RayTracingScene::NotifyGeometryChanged(RayTracingGeometry& geometry)
	{
		if(!geometry.IsBuilt())
			mPendingGeometries.push_back(&geometry);

		mIsDirty = true;
	}

	void RayTracingScene::PackInstance(u32 slotIndex)
	{
		Slot& slot = mSlots[slotIndex];
		RayTracingGeometry* geometry = slot.Instance.Geometry.get();
		if(geometry == nullptr)
			return;

		const u32 packedIndex = (u32)mPackedInstanceSlots.Size();
		mPackedInstances.resize((size_t)(packedIndex + 1) * mInstanceSize);
		if(!TryWriteInstance(slot.Instance, &mPackedInstances[(size_t)packedIndex * mInstanceSize]))
		{
			mPackedInstances.resize((size_t)packedIndex * mInstanceSize);
			return;
		}

		slot.PackedIndex = packedIndex;
		mPackedInstanceSlots.Add(slotIndex);

		// The first instance of a geometry makes the scene listen to it, and queues its build if it isn't built
		GeometryReference& reference = mGeometryReferences[geometry];
		if(reference.InstanceCount++ > 0)
			return;

		reference.Index = (u32)mGeometries.Size();
		mGeometries.Add(geometry);
		mBottomLevels.Add(geometry->GetAccelerationStructure());
		mBottomLevelsChanged = true;

		geometry->NotifyAddedToScene(*this);
		if(!geometry->IsBuilt())
			mPendingGeometries.push_back(geometry);
	}

	void RayTracingScene::UnpackInstance(u32 slotIndex)
	{
		Slot& slot = mSlots[slotIndex];
		const u32 packedIndex = slot.PackedIndex;
		if(packedIndex == ~0u)
			return;

		// The last packed instance takes the place of the removed one. Instances are of the device's size, so their data is moved by hand.
		const u32 lastPackedIndex = (u32)mPackedInstanceSlots.Size() - 1;
		if(packedIndex != lastPackedIndex)
			std::memcpy(&mPackedInstances[(size_t)packedIndex * mInstanceSize], &mPackedInstances[(size_t)lastPackedIndex * mInstanceSize], mInstanceSize);

		mPackedInstances.resize((size_t)lastPackedIndex * mInstanceSize);
		if(mPackedInstanceSlots.SwapAndErase(mPackedInstanceSlots.begin() + packedIndex))
			mSlots[mPackedInstanceSlots[packedIndex]].PackedIndex = packedIndex;

		slot.PackedIndex = ~0u;

		// The last instance of a geometry removes it from the scene, and the last geometry takes its place
		RayTracingGeometry* geometry = slot.Instance.Geometry.get();
		const auto referenceIterator = mGeometryReferences.find(geometry);
		if(--referenceIterator->second.InstanceCount > 0)
			return;

		const u32 geometryIndex = referenceIterator->second.Index;
		mGeometryReferences.erase(referenceIterator);
		mBottomLevels.SwapAndErase(mBottomLevels.begin() + geometryIndex);
		if(mGeometries.SwapAndErase(mGeometries.begin() + geometryIndex))
			mGeometryReferences[mGeometries[geometryIndex]].Index = geometryIndex;

		mBottomLevelsChanged = true;

		geometry->NotifyRemovedFromScene(*this);
		mPendingGeometries.erase(std::remove(mPendingGeometries.begin(), mPendingGeometries.end(), geometry), mPendingGeometries.end());
	}

	bool RayTracingScene::TryWriteInstance(const RayTracingGeometryInstance& instance, u8* destination) const
	{
		GpuRayTracingInstance deviceInstance(instance.Geometry->GetAccelerationStructure().get(), instance.Transform, instance.InstanceId);
		deviceInstance.HitGroupIndexOffset = instance.HitGroupIndexOffset;
		deviceInstance.Mask = instance.Mask;
		deviceInstance.Flags = instance.Flags;

		return mDevice.WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(&deviceInstance, 1), destination);
	}
} // namespace b3d::render
