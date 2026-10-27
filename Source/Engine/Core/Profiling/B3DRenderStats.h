//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Profiling-Internal
	 *  @{
	 */

	/**	Common object types to track resource statistics for. */
	enum RenderStatResourceType
	{
		RenderStatObject_IndexBuffer,
		RenderStatObject_VertexBuffer,
		RenderStatObject_GpuBuffer,
		RenderStatObject_GpuParamBuffer,
		RenderStatObject_Texture,
		RenderStatObject_GpuProgram,
		RenderStatObject_Query
	};

	/** Statistics tracked by RenderStats. */
	enum class RenderStatistic : u32
	{
		DrawCallCount, /**< Number of draw calls. */
		ComputeCallCount, /**< Number of compute dispatches. */
		RenderTargetChangeCount, /**< Number of times the active render target changed. */
		PresentCount, /**< Number of presents. */
		ClearCount, /**< Number of render target clears, entire or partial. */
		VertexCount, /**< Number of vertices sent to the pipeline. */
		PrimitiveCount, /**< Number of primitives sent to the pipeline. */
		PipelineStateChangeCount, /**< Number of pipeline state binds. */
		GpuParameterBindCount, /**< Number of GPU parameter binds. */
		VertexBufferBindCount, /**< Number of vertex buffer binds. */
		IndexBufferBindCount, /**< Number of index buffer binds. */
		ResourceWriteCount, /**< Number of GPU resource writes. */
		ResourceReadCount, /**< Number of GPU resource reads. */
		ObjectsCreatedCount, /**< Number of GPU objects created. */
		ObjectsDestroyedCount, /**< Number of GPU objects destroyed. */
		Count
	};

	/** Object that stores various render statistics. */
	struct B3D_EXPORT RenderStatsData
	{
		/** Value of each statistic, indexed by RenderStatistic. */
		std::array<u64, (u32)RenderStatistic::Count> Values {};
	};

	/**
	 * Tracks various render system statistics.
	 *
	 * @note	Thread safe, as command buffers may be recorded on several threads at once.
	 */
	class B3D_EXPORT RenderStats : public Module<RenderStats>
	{
	public:
		/** Increments draw call counter indicating how many times were render system API Draw methods called. */
		void IncNumDrawCalls() { Add(RenderStatistic::DrawCallCount, 1); }

		/** Increments compute call counter indicating how many times were compute shaders dispatched. */
		void IncNumComputeCalls() { Add(RenderStatistic::ComputeCallCount, 1); }

		/** Increments render target change counter indicating how many times did the active render target change. */
		void IncNumRenderTargetChanges() { Add(RenderStatistic::RenderTargetChangeCount, 1); }

		/** Increments render target present counter indicating how many times did the buffer swap happen. */
		void IncNumPresents() { Add(RenderStatistic::PresentCount, 1); }

		/**
		 * Increments render target clear counter indicating how many times did the target the cleared, entirely or
		 * partially.
		 */
		void IncNumClears() { Add(RenderStatistic::ClearCount, 1); }

		/** Increments vertex draw counter indicating how many vertices were sent to the pipeline. */
		void AddNumVertices(u32 count) { Add(RenderStatistic::VertexCount, count); }

		/** Increments primitive draw counter indicating how many primitives were sent to the pipeline. */
		void AddNumPrimitives(u32 count) { Add(RenderStatistic::PrimitiveCount, count); }

		/** Increments pipeline state change counter indicating how many times was a pipeline state bound. */
		void IncNumPipelineStateChanges() { Add(RenderStatistic::PipelineStateChangeCount, 1); }

		/** Increments GPU parameter change counter indicating how many times were GPU parameters bound to the pipeline. */
		void IncNumGpuParamBinds() { Add(RenderStatistic::GpuParameterBindCount, 1); }

		/** Increments vertex buffer change counter indicating how many times was a vertex buffer bound to the pipeline. */
		void IncNumVertexBufferBinds() { Add(RenderStatistic::VertexBufferBindCount, 1); }

		/** Increments index buffer change counter indicating how many times was a index buffer bound to the pipeline. */
		void IncNumIndexBufferBinds() { Add(RenderStatistic::IndexBufferBindCount, 1); }

		/**
		 * Increments created GPU resource counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResCreated(u32 category)
		{
			// TODO - I'm ignoring resourceType for now. Later I will want to
			// count object creation/destruction/read/write per type. I will
			// also want to allow the caller to assign names to specific "resourceType" id.
			// (Since many types will be GpuBackend specific).

			// TODO - I should also track number of active GPU objects using this method, instead
			// of just keeping track of how many were created and destroyed during the frame.

			Add(RenderStatistic::ObjectsCreatedCount, 1);
		}

		/**
		 * Increments destroyed GPU resource counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResDestroyed(u32 category) { Add(RenderStatistic::ObjectsDestroyedCount, 1); }

		/**
		 * Increments GPU resource read counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResRead(u32 category) { Add(RenderStatistic::ResourceReadCount, 1); }

		/**
		 * Increments GPU resource write counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResWrite(u32 category) { Add(RenderStatistic::ResourceWriteCount, 1); }

		/** Returns the current values of the statistics. */
		RenderStatsData GetData() const
		{
			RenderStatsData output;
			for(u32 statisticIndex = 0; statisticIndex < (u32)RenderStatistic::Count; statisticIndex++)
				output.Values[statisticIndex] = mCounters[statisticIndex].load(std::memory_order_relaxed);

			return output;
		}

	private:
		/** Adds @p count to the counter of @p statistic. The counters order nothing, so the addition is relaxed. */
		void Add(RenderStatistic statistic, u64 count) { mCounters[(u32)statistic].fetch_add(count, std::memory_order_relaxed); }

		/** Counter of each statistic, indexed by RenderStatistic. */
		std::array<std::atomic<u64>, (u32)RenderStatistic::Count> mCounters {};
	};

#if B3D_PROFILING_ENABLED
#	define B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(Stat, Category) RenderStats::Instance().Inc##Stat((u32)Category)
#	define B3D_INCREMENT_RENDER_STATISTIC(Stat) RenderStats::Instance().Inc##Stat()
#	define B3D_ADD_RENDER_STATISTIC(Stat, Count) RenderStats::Instance().Add##Stat(Count)
#else
#	define B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(Stat, Category)
#	define B3D_INCREMENT_RENDER_STATISTIC(Stat)
#	define B3D_ADD_RENDER_STATISTIC(Stat, Count)
#endif

	/** @} */
} // namespace b3d
