//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DCorePrerequisites.h"
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

	/** Object that stores various render statistics. */
	struct B3D_CORE_EXPORT RenderStatsData
	{
		RenderStatsData() = default;

		u64 NumDrawCalls = 0;
		u64 NumComputeCalls = 0;
		u64 NumRenderTargetChanges = 0;
		u64 NumPresents = 0;
		u64 NumClears = 0;

		u64 NumVertices = 0;
		u64 NumPrimitives = 0;

		u64 NumPipelineStateChanges = 0;

		u64 NumGpuParamBinds = 0;
		u64 NumVertexBufferBinds = 0;
		u64 NumIndexBufferBinds = 0;

		u64 NumResourceWrites;
		u64 NumResourceReads;

		u64 NumObjectsCreated;
		u64 NumObjectsDestroyed;
	};

	/**
	 * Tracks various render system statistics.
	 *
	 * @note	Render thread only.
	 */
	class B3D_CORE_EXPORT RenderStats : public Module<RenderStats>
	{
	public:
		/** Increments draw call counter indicating how many times were render system API Draw methods called. */
		void IncNumDrawCalls() { mData.NumDrawCalls++; }

		/** Increments compute call counter indicating how many times were compute shaders dispatched. */
		void IncNumComputeCalls() { mData.NumComputeCalls++; }

		/** Increments render target change counter indicating how many times did the active render target change. */
		void IncNumRenderTargetChanges() { mData.NumRenderTargetChanges++; }

		/** Increments render target present counter indicating how many times did the buffer swap happen. */
		void IncNumPresents() { mData.NumPresents++; }

		/**
		 * Increments render target clear counter indicating how many times did the target the cleared, entirely or
		 * partially.
		 */
		void IncNumClears() { mData.NumClears++; }

		/** Increments vertex draw counter indicating how many vertices were sent to the pipeline. */
		void AddNumVertices(u32 count) { mData.NumVertices += count; }

		/** Increments primitive draw counter indicating how many primitives were sent to the pipeline. */
		void AddNumPrimitives(u32 count) { mData.NumPrimitives += count; }

		/** Increments pipeline state change counter indicating how many times was a pipeline state bound. */
		void IncNumPipelineStateChanges() { mData.NumPipelineStateChanges++; }

		/** Increments GPU parameter change counter indicating how many times were GPU parameters bound to the pipeline. */
		void IncNumGpuParamBinds() { mData.NumGpuParamBinds++; }

		/** Increments vertex buffer change counter indicating how many times was a vertex buffer bound to the pipeline. */
		void IncNumVertexBufferBinds() { mData.NumVertexBufferBinds++; }

		/** Increments index buffer change counter indicating how many times was a index buffer bound to the pipeline. */
		void IncNumIndexBufferBinds() { mData.NumIndexBufferBinds++; }

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
			// (Since many types will be RenderAPI specific).

			// TODO - I should also track number of active GPU objects using this method, instead
			// of just keeping track of how many were created and destroyed during the frame.

			mData.NumObjectsCreated++;
		}

		/**
		 * Increments destroyed GPU resource counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResDestroyed(u32 category) { mData.NumObjectsDestroyed++; }

		/**
		 * Increments GPU resource read counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResRead(u32 category) { mData.NumResourceReads++; }

		/**
		 * Increments GPU resource write counter.
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void IncResWrite(u32 category) { mData.NumResourceWrites++; }

		/**
		 * Returns an object containing various rendering statistics.
		 *
		 * @note
		 * Do not modify the returned state unless you know what you are doing, it will change the actual internal object.
		 */
		RenderStatsData& GetData() { return mData; }

	private:
		RenderStatsData mData;
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
