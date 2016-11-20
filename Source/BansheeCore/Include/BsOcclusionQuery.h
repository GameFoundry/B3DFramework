//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Represents a query that counts number of samples rendered by the GPU while the query is active.
	 *
	 * @note		Core thread only.
	 */
	class BS_CORE_EXPORT OcclusionQuery
	{
	public:
		OcclusionQuery(bool binary);
		virtual ~OcclusionQuery() {}

		/**
		 * Starts the query. Any draw calls after this call will have any rendered samples counted in the query.
		 *
		 * @note	Place any commands you want to measure after this call. Call end() when done.
		 */
		virtual void begin(const SPtr<CommandBuffer>& cb = nullptr) = 0;

		/**
		 * Stops the query. 
		 *
		 * @note	Be aware that queries are executed on the GPU and the results will not be immediately available.
		 */
		virtual void end(const SPtr<CommandBuffer>& cb = nullptr) = 0;

		/** Check if GPU has processed the query. */
		virtual bool isReady() const = 0;

		/**
		 * Returns the number of samples that passed the depth and stencil test between query start and end.
		 *
		 * @note	
		 * If the query is binary, this will return 0 or 1. 1 meaning one or more samples were rendered, but will not give
		 * you the exact count.
		 */
		virtual UINT32 getNumSamples() = 0;

		/** Triggered when the query has completed. Argument provided is the number of samples counted by the query. */
		Event<void(UINT32)> onComplete;

		/**
		 * Creates a new occlusion query. 
		 *
		 * @param[in] binary		If query is binary it will not give you an exact count of samples rendered, but will
		 *							instead just return 0 (no samples were rendered) or 1 (one or more samples were 
		 *							rendered). Binary queries can return sooner as they potentially do not need to wait
		 *							until all of the geometry is rendered.
		 * @param[in]	deviceIdx	Index of the GPU device to create the query on. 
		 */
		static SPtr<OcclusionQuery> create(bool binary, UINT32 deviceIdx = 0);

	protected:
		friend class QueryManager;

		/**	Returns true if the has still not been completed by the GPU. */
		bool isActive() const { return mActive; }
		void setActive(bool active) { mActive = active; }

	protected:
		bool mActive;
		bool mBinary;
	};

	/** @} */
}