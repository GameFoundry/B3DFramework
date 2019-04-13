//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
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
	struct BS_CORE_EXPORT RenderStatsData
	{
		RenderStatsData() = default;

		UINT64 numDrawCalls = 0;
		UINT64 numComputeCalls = 0;
		UINT64 numRenderTargetChanges = 0;
		UINT64 numPresents = 0;
		UINT64 numClears = 0;

		UINT64 numVertices = 0;
		UINT64 numPrimitives = 0;

		UINT64 numPipelineStateChanges = 0;

		UINT64 numGpuParamBinds = 0; 
		UINT64 numVertexBufferBinds = 0; 
		UINT64 numIndexBufferBinds = 0;

		UINT64 numResourceWrites;
		UINT64 numResourceReads;

		UINT64 numObjectsCreated; 
		UINT64 numObjectsDestroyed;
	};

	/**
	 * Tracks various render system statistics.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT RenderStats : public Module<RenderStats>
	{
	public:
		/** Increments draw call counter indicating how many times were render system API Draw methods called. */
		void incNumDrawCalls() { mData.numDrawCalls++; }

		/** Increments compute call counter indicating how many times were compute shaders dispatched. */
		void incNumComputeCalls() { mData.numComputeCalls++; }

		/** Increments render target change counter indicating how many times did the active render target change. */
		void incNumRenderTargetChanges() { mData.numRenderTargetChanges++; }

		/** Increments render target present counter indicating how many times did the buffer swap happen. */
		void incNumPresents() { mData.numPresents++; }

		/** 
		 * Increments render target clear counter indicating how many times did the target the cleared, entirely or 
		 * partially. 
		 */
		void incNumClears() { mData.numClears++; }

		/** Increments vertex draw counter indicating how many vertices were sent to the pipeline. */
		void addNumVertices(UINT32 count) { mData.numVertices += count; }

		/** Increments primitive draw counter indicating how many primitives were sent to the pipeline. */
		void addNumPrimitives(UINT32 count) { mData.numPrimitives += count; }

		/** Increments pipeline state change counter indicating how many times was a pipeline state bound. */
		void incNumPipelineStateChanges() { mData.numPipelineStateChanges++; }

		/** Increments GPU parameter change counter indicating how many times were GPU parameters bound to the pipeline. */
		void incNumGpuParamBinds() { mData.numGpuParamBinds++; }

		/** Increments vertex buffer change counter indicating how many times was a vertex buffer bound to the pipeline. */
		void incNumVertexBufferBinds() { mData.numVertexBufferBinds++; }

		/** Increments index buffer change counter indicating how many times was a index buffer bound to the pipeline. */
		void incNumIndexBufferBinds() { mData.numIndexBufferBinds++; }

		/**
		 * Increments created GPU resource counter. 
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void incResCreated(UINT32 category) 
		{
			// TODO - I'm ignoring resourceType for now. Later I will want to
			// count object creation/destruction/read/write per type. I will
			// also want to allow the caller to assign names to specific "resourceType" id.
			// (Since many types will be RenderAPI specific).

			// TODO - I should also track number of active GPU objects using this method, instead
			// of just keeping track of how many were created and destroyed during the frame.

			mData.numObjectsCreated++;
		}

		/**
		 * Increments destroyed GPU resource counter. 
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void incResDestroyed(UINT32 category) { mData.numObjectsDestroyed++; }

		/**
		 * Increments GPU resource read counter. 
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void incResRead(UINT32 category) { mData.numResourceReads++; }

		/**
		 * Increments GPU resource write counter. 
		 *
		 * @param[in]	category	Category of the resource.
		 */
		void incResWrite(UINT32 category) { mData.numResourceWrites++; }

		/**
		 * Returns an object containing various rendering statistics.
		 *			
		 * @note	
		 * Do not modify the returned state unless you know what you are doing, it will change the actual internal object.
		 */
		RenderStatsData& getData() { return mData; }

	private:
		RenderStatsData mData;
	};

#if BS_PROFILING_ENABLED
	#define BS_INC_RENDER_STAT_CAT(Stat, Category) RenderStats::instance().inc##Stat((UINT32)Category)
	#define BS_INC_RENDER_STAT(Stat) RenderStats::instance().inc##Stat()
	#define BS_ADD_RENDER_STAT(Stat, Count) RenderStats::instance().add##Stat(Count)
#else
	#define BS_INC_RENDER_STAT_CAT(Stat, Category)
	#define BS_INC_RENDER_STAT(Stat)
	#define BS_ADD_RENDER_STAT(Stat, Count)
#endif

	/** @} */
}