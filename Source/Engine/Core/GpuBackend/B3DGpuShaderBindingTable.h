//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	struct GpuRayTracingCapabilities;

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

	/** Records of one kind in the memory of a shader binding table. All values are in bytes. Zeros if the table has no records of the kind. */
	struct GpuShaderBindingTableRegion
	{
		u32 Offset = 0; /**< Offset of the first record from the start of the table. A multiple of GpuRayTracingCapabilities::ShaderTableAlignment. */
		u32 Stride = 0; /**< Distance between two records. */
		u32 Size = 0; /**< Stride times the number of records. */
	};

	/** Descriptor structure used for initializing a shader binding table. */
	struct GpuShaderBindingTableCreateInformation : GpuShaderBindingTableInformation
	{
		GpuShaderBindingTableCreateInformation() = default;

		GpuShaderBindingTableCreateInformation(const GpuShaderBindingTableInformation& other)
			: GpuShaderBindingTableInformation(other)
		{}
	};

	/**
	 * Layout of a shader binding table whose records are the identifiers of their shader groups, the kind of table Vulkan and D3D12 read.
	 * Backends with such tables keep one next to the memory of the records.
	 */
	class B3D_EXPORT GpuShaderBindingTableIdentifierLayout
	{
	public:
		/**
		 * Computes where the records go from @p capabilities.
		 *
		 * @param	information		Records of the table, which the device validated.
		 * @param	capabilities	Ray tracing capabilities of the device that creates the table.
		 */
		GpuShaderBindingTableIdentifierLayout(const GpuShaderBindingTableInformation& information, const GpuRayTracingCapabilities& capabilities);

		/** Returns the number of bytes the records of the table take. */
		u32 GetSize() const { return mSize; }

		/**
		 * Returns where the ray generation records are. Their stride is a multiple of the table alignment, so each record can be passed as a
		 * region of its own.
		 */
		const GpuShaderBindingTableRegion& GetRayGenerationRegion() const { return mRayGenerationRegion; }

		/** Returns where the miss records are. */
		const GpuShaderBindingTableRegion& GetMissRegion() const { return mMissRegion; }

		/** Returns where the hit group records are. */
		const GpuShaderBindingTableRegion& GetHitGroupRegion() const { return mHitGroupRegion; }

		/** Returns where the callable records are. */
		const GpuShaderBindingTableRegion& GetCallableRegion() const { return mCallableRegion; }

		/**
		 * Writes every record into @p destination, which has GetSize() bytes. A record is the identifier of its shader group, and an empty
		 * hit group record is zeros. Groups are numbered ray generation programs first, then miss programs, hit groups and callable programs.
		 *
		 * @param	information				Records of the table, the same the layout was computed from.
		 * @param	destination				Memory of the table.
		 * @param	fnGetGroupIdentifier	Returns the identifier of a shader group by its number, GpuRayTracingCapabilities::ShaderGroupHandleSize
		 *									bytes.
		 */
		void WriteRecords(const GpuShaderBindingTableInformation& information, u8* destination, const Function<TArrayView<const u8>(u32)>& fnGetGroupIdentifier) const;

	private:
		u32 mSize = 0;
		u32 mShaderGroupHandleSize = 0;
		GpuShaderBindingTableRegion mRayGenerationRegion;
		GpuShaderBindingTableRegion mMissRegion;
		GpuShaderBindingTableRegion mHitGroupRegion;
		GpuShaderBindingTableRegion mCallableRegion;
	};

	namespace render
	{
		/**
		 * Records that tell a ray tracing dispatch which programs of a pipeline run for each ray. Created by
		 * GpuDevice::CreateGpuShaderBindingTable(). Backends that support ray tracing pipelines derive from it, and own the memory of the
		 * records in a layout of their own.
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
			{}

			GpuShaderBindingTableInformation mInformation;
		};
	} // namespace render

	/** @} */
} // namespace b3d
