//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"

namespace b3d::render
{
	class FrameGraph;
	class RayTracingGeometry;

	/** @addtogroup Renderer
	 *  @{
	 */

	/** Instance of a RayTracingGeometry in a RayTracingScene. */
	struct RayTracingGeometryInstance
	{
		RayTracingGeometryInstance() = default;

		/**
		 * @param	geometry	Geometry the instance places in the scene.
		 * @param	transform	Transform from the space of the geometry to the space of the scene.
		 * @param	instanceId	Value shaders read as the instance id. 24 bits.
		 */
		RayTracingGeometryInstance(const TShared<RayTracingGeometry>& geometry, const Matrix4& transform, u32 instanceId = 0)
			: Geometry(geometry), Transform(transform), InstanceId(instanceId)
		{ }

		TShared<RayTracingGeometry> Geometry; /**< Geometry the instance places in the scene. Instances without one are left out. */
		Matrix4 Transform = Matrix4::kIdentity; /**< Transform from the space of the geometry to the space of the scene. Affine. */

		/**
		 * Value shaders read as the instance id. Indexes data of the instance's own, such as a table of the layouts of its geometries. Stays
		 * the same while the instance index shaders read changes as instances are added and removed. 24 bits.
		 */
		u32 InstanceId = 0;

		/**
		 * Added to the shader binding table record index of hits on the instance: the first record of the hit group the instance's
		 * material uses. 24 bits.
		 */
		u32 HitGroupIndexOffset = 0;

		u8 Mask = 0xFF; /**< Rays only hit the instance if this and their instance mask have a bit in common. */
		GpuRayTracingInstanceFlags Flags; /**< Flags that change how rays interact with the instance, such as ForceNonOpaque for alpha tested ones. */
	};

	/**
	 * Top level acceleration structure of a set of geometry instances. Build() adds frame graph passes that build what changed: the
	 * bottom level structures of geometries that aren't built, and the top level structure if any instance or geometry changed. In a
	 * frame where nothing changed it adds no passes and allocates nothing.
	 *
	 * @note	Render thread.
	 */
	class B3D_EXPORT RayTracingScene
	{
	public:
		/**
		 * @param	device	Device to create the structures on. Must have RSC_ACCELERATION_STRUCTURES.
		 * @param	name	Name of the top level structure, for debugging purposes.
		 */
		RayTracingScene(GpuDevice& device, const StringView& name);

		RayTracingScene(const RayTracingScene&) = delete;
		RayTracingScene& operator=(const RayTracingScene&) = delete;

		/** Adds @p instance to the scene, and returns a slot that identifies it until RemoveInstance(). */
		u32 AddInstance(const RayTracingGeometryInstance& instance);

		/** Replaces the instance in @p slot with @p instance. */
		void UpdateInstance(u32 slot, const RayTracingGeometryInstance& instance);

		/** Moves the instance in @p slot to @p transform. */
		void SetInstanceTransform(u32 slot, const Matrix4& transform);

		/** Removes the instance in @p slot. The slot may be returned by a later AddInstance(). */
		void RemoveInstance(u32 slot);

		/** Returns the number of instances in the scene. */
		u32 GetInstanceCount() const { return mInstanceCount; }

		/**
		 * Adds passes to @p graph that build the bottom level structures of the instances' geometries that aren't built, in one pass,
		 * and, if anything changed since the last build, the top level structure, in a second pass that exports it. Later passes and later
		 * work can then trace rays against GetAccelerationStructure() without declaring it. The scene and the geometries of its instances
		 * must stay alive until the graph executes, and the scene must not be built again before then.
		 */
		void Build(FrameGraph& graph);

		/**
		 * Returns the top level structure, or null before the first Build(). It is replaced when the scene outgrows it, so read it after
		 * Build(). Bindless on devices with GpuRayTracingCapabilities::SupportsBindlessAccelerationStructures.
		 */
		const TShared<GpuAccelerationStructure>& GetAccelerationStructure() const { return mAccelerationStructure; }

	private:
		/** Instance in a slot, or a free slot. */
		struct Slot
		{
			RayTracingGeometryInstance Instance;
			bool IsUsed = false;
			u32 NextFreeSlot = ~0u; /**< Next slot of the free list, if the slot is free. */
		};

		GpuDevice& mDevice;
		String mName;

		Vector<Slot> mSlots;
		u32 mFirstFreeSlot = ~0u;
		u32 mInstanceCount = 0;
		bool mIsDirty = true;

		TShared<GpuAccelerationStructure> mAccelerationStructure;
		u32 mCapacity = 0; /**< Number of instances the top level structure is sized for. */
		u32 mBuildScratchSize = 0; /**< Scratch memory a build of the top level structure with mCapacity instances needs, in bytes. */

		/** Bottom level structures of the last top level build. Its execute function reads them, so they live in the scene. */
		Vector<TShared<GpuAccelerationStructure>> mBottomLevels;
	};

	/** @} */
} // namespace b3d::render
