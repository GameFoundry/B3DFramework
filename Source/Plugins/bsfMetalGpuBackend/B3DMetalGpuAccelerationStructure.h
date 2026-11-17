//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalResource.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Wrapper around a Metal acceleration structure and the memory that holds it, which manages its usage and lifetime. */
		class MetalAccelerationStructure : public MetalResource
		{
		public:
			/**
			 * @param	owner					Manager that takes care of tracking and releasing of this object.
			 * @param	accelerationStructure	Native structure. The wrapper frees it, and its memory, when destroyed.
			 * @param	allocation				Heap memory the structure is placed in, or no memory if the structure isn't placed in a heap.
			 * @param	name					Optional name of the structure, for debugging purposes.
			 */
			MetalAccelerationStructure(MetalResourceManager* owner, id<MTLAccelerationStructure> accelerationStructure, const GpuAllocation& allocation, const StringView& name);
			~MetalAccelerationStructure();

			/** Returns the native structure. */
			id<MTLAccelerationStructure> GetMetalHandle() const { return mAccelerationStructure; }

		private:
			id<MTLAccelerationStructure> mAccelerationStructure = nil;
			GpuAllocation mAllocation;
		};

		/** Metal implementation of an acceleration structure. */
		class MetalGpuAccelerationStructure : public GpuAccelerationStructure
		{
		public:
			/**
			 * @param	device					Device that created the structure.
			 * @param	createInformation		Description of the structure, which the device validated.
			 * @param	accelerationStructure	Native structure, which the base class destroys.
			 */
			MetalGpuAccelerationStructure(MetalGpuDevice& device, const GpuAccelerationStructureCreateInformation& createInformation, MetalAccelerationStructure& accelerationStructure);

			/** Returns the native structure. */
			MetalAccelerationStructure* GetMetalResource() const { return mAccelerationStructure; }

		private:
			MetalAccelerationStructure* mAccelerationStructure;
		};

		/** @} */
	} // namespace render
} // namespace b3d
