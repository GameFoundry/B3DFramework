//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Hit group record of a shader binding table that runs no program. A ray that hits geometry with this record ends without running one. */
	inline constexpr u32 kGpuShaderBindingTableEmptyRecord = ~0u;

	/** Records of a shader binding table: which programs of a ray tracing pipeline run for which rays. */
	struct GpuShaderBindingTableInformation
	{
		TShared<GpuRayTracingPipelineState> Pipeline; /**< Pipeline whose programs the records run. */

		/** Per record, index into the pipeline's RayGenerationPrograms. render::GpuCommandBuffer::DispatchRays() picks the record. */
		Vector<u32> RayGenerationRecords;

		/** Per record, index into the pipeline's MissPrograms. The MissShaderIndex of TraceRay() picks the record. */
		Vector<u32> MissRecords;

		/**
		 * Per record, index into the pipeline's HitGroups, or kGpuShaderBindingTableEmptyRecord. A ray that hits geometry runs the record at
		 * GpuRayTracingInstance::HitGroupIndexOffset of the instance, plus RayContributionToHitGroupIndex of TraceRay(), plus GeometryIndex()
		 * times MultiplierForGeometryContributionToHitGroupIndex of TraceRay().
		 */
		Vector<u32> HitGroupRecords;

		/** Per record, index into the pipeline's CallablePrograms. The index CallShader() is called with picks the record. */
		Vector<u32> CallableRecords;
	};

	/** Descriptor structure used for initializing a shader binding table. */
	struct GpuShaderBindingTableCreateInformation : GpuShaderBindingTableInformation
	{
		GpuShaderBindingTableCreateInformation() = default;

		GpuShaderBindingTableCreateInformation(const GpuShaderBindingTableInformation& other)
			: GpuShaderBindingTableInformation(other)
		{}
	};

	namespace render
	{
		/**
		 * Records that tell a ray tracing dispatch which programs of a pipeline run for each ray. Created by
		 * GpuDevice::CreateGpuShaderBindingTable(). Backends that support ray tracing pipelines derive from it, and own the memory of the
		 * records.
		 *
		 * @note	Thread safe (Immutable).
		 */
		class B3D_EXPORT GpuShaderBindingTable
		{
		public:
			virtual ~GpuShaderBindingTable() = default;

			/** Returns the records of the table. */
			const GpuShaderBindingTableInformation& GetInformation() const { return mInformation; }

			/** Returns the pipeline whose programs the records run. */
			const TShared<GpuRayTracingPipelineState>& GetPipeline() const { return mInformation.Pipeline; }

		protected:
			/** @param	createInformation	Records of the table, which the device validated. */
			GpuShaderBindingTable(const GpuShaderBindingTableCreateInformation& createInformation)
				: mInformation(createInformation)
			{ }

			GpuShaderBindingTableInformation mInformation;
		};
	} // namespace render

	/** @} */
} // namespace b3d
