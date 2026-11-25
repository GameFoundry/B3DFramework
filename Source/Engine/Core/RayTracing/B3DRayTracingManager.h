//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueries.h"
#include "Utility/B3DConfigVariable.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/**
	 * If enabled, meshes create their ray tracing geometry with render::RayTracingGeometryFlag::AllowCompaction, so its structures are
	 * compacted once they are built and use less memory. Applies to geometry created after the change, and stops the compaction of
	 * geometry created before it. See render::RayTracingManager::Update().
	 */
	extern B3D_EXPORT TConfigVariable<bool> gRenderRayTracingCompaction;
} // namespace b3d

namespace b3d::render
{
	class FrameGraph;
	class RayTracingGeometry;

	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Normal, tangent and texture coordinates that vertices without them read, in the formats of RayTracingGeometry. Layout of the buffer
	 * RayTracingManager::GetDefaultAttributes() returns.
	 */
	struct RayTracingDefaultAttributes
	{
		u32 Normal = 0xFFFF8080; /**< (0, 0, 1). */
		u32 Tangent = 0xFF8080FF; /**< (1, 0, 0), with handedness 1. */
		float Uv[2] = { 0.0f, 0.0f }; /**< (0, 0). */
	};

	static_assert(sizeof(RayTracingDefaultAttributes) == 16, "RayTracingDefaultAttributes must match the layout of its buffer.");

	/** Ray tracing resources that every RayTracingGeometry of a device shares, and the compaction of their structures. */
	class B3D_EXPORT RayTracingManager : public Module<RayTracingManager>
	{
	public:
		/**
		 * Returns a bindless buffer with the normal, tangent and texture coordinates that vertices without them read, creating it on first
		 * use. Its layout is RayTracingDefaultAttributes. Returns null if @p device has no bindless heaps.
		 *
		 * @note	Render thread.
		 */
		const TShared<GpuBuffer>& GetDefaultAttributes(GpuDevice& device);

		/**
		 * Requests the compaction of each of @p geometries that was created with RayTracingGeometryFlag::AllowCompaction and without
		 * RayTracingGeometryFlag::Dynamic, and is built. Adds a pass to @p graph that queries their compacted sizes, so a later Update()
		 * can compact them. A pass of @p graph must have built them. Adds nothing if none of them qualifies. RayTracingScene::Build()
		 * requests the compaction of the geometries it builds.
		 *
		 * @note	Render thread.
		 */
		void RequestCompaction(FrameGraph& graph, TArrayView<RayTracingGeometry* const> geometries);

		/**
		 * Adds a pass to @p graph that compacts the structures of the geometries built on the graph's device whose compacted sizes are
		 * known, so they use less memory. Ray tracing scenes with instances of them rebuild their top level structure in their next
		 * RayTracingScene::Build(). Call once per frame, before the scenes build. Adds nothing in a frame without results, and never waits
		 * for the GPU. Geometry is compacted if its compaction was requested (RequestCompaction()), the size query has resolved, and
		 * gRenderRayTracingCompaction is on.
		 *
		 * @note	Render thread.
		 */
		void Update(FrameGraph& graph);

	protected:
		void OnShutDown() override;

	private:
		/** Compacted size query of one geometry. */
		struct CompactedSizeQuery
		{
			CompactedSizeQuery(const WeakSPtr<RayTracingGeometry>& geometry, u32 version)
				: Geometry(geometry), Version(version)
			{ }

			WeakSPtr<RayTracingGeometry> Geometry;
			u32 Version = 0; /**< RayTracingGeometry::GetVersion() of the build the query reads. */
			GpuQueryId Query; /**< Allocated when the pass records. */
		};

		/** Compacted size queries in one pool. A pass writes several batches when its queries don't fit in one pool. */
		struct CompactedSizeQueryBatch
		{
			GpuDevice* Device = nullptr;
			TShared<GpuQueryPool> QueryPool;
			Vector<CompactedSizeQuery> Queries;

			/**
			 * Set once the pass records the queries. Until then a pool reads as resolved, with no queries or with the results of the batch
			 * that used it before, so the batch is only resolved after this.
			 */
			std::atomic<bool> IsRecorded = false;
		};

		static constexpr u32 kCompactedSizeQueryPoolSize = 64; /**< Number of queries in each pool of compacted size queries. */

		UnorderedMap<GpuDevice*, TShared<GpuBuffer>> mDefaultAttributes;
		Vector<TShared<CompactedSizeQueryBatch>> mCompactedSizeQueryBatches;

		/** Pools of compacted size queries whose batches Update() resolved, for later batches of the same device. */
		UnorderedMap<GpuDevice*, Vector<TShared<GpuQueryPool>>> mFreeCompactedSizeQueryPools;
	};

	/** @} */
} // namespace b3d::render
