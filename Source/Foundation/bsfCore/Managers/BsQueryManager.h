//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsEventQuery.h"
#include "Utility/BsModule.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Handles creation and destruction of GPU queries.
	 * 			
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT QueryManager : public Module<QueryManager>
	{
	public:
		QueryManager() = default;
		~QueryManager();

		/** 
		 * Creates a new event query that allows you to get notified when GPU starts executing the query.  
		 *
		 * @param[in]	deviceIdx	Index of the GPU device to create the query on. 
		 */
		virtual SPtr<EventQuery> createEventQuery(UINT32 deviceIdx = 0) const = 0;

		/**
		 * Creates a new timer query that allows you to get notified of how much time has passed between query start and end.
		 * 
		 * @param[in]	deviceIdx	Index of the GPU device to create the query on. 
		 */
		virtual SPtr<TimerQuery> createTimerQuery(UINT32 deviceIdx = 0) const = 0;

		/**
		 * Creates a new occlusion query that allows you to know how many fragments were rendered between query start and 
		 * end.
		 *
		 * @param[in]	binary		If query is binary it will not give you an exact count of fragments rendered, but will 
		 *							instead just return 0 (no fragments were rendered) or 1 (one or more fragments were 
		 *							rendered). Binary queries can return sooner as they potentially do not need to wait
		 *							until all of the geometry is rendered.
		 * @param[in]	deviceIdx	Index of the GPU device to create the query on. 
		 */
		virtual SPtr<OcclusionQuery> createOcclusionQuery(bool binary, UINT32 deviceIdx = 0) const = 0;

		/** Triggers completed queries. Should be called every frame. */
		void _update();

	protected:
		friend class EventQuery;
		friend class TimerQuery;
		friend class OcclusionQuery;

		/**
		 * Deletes an Event query. Always use this method and don't delete them manually. Actual deletion will be delayed 
		 * until next update.
		 */
		static void deleteEventQuery(EventQuery* query);

		/**
		 * Deletes a Timer query. Always use this method and don't delete them manually. Actual deletion will be delayed 
		 * until next update.
		 */
		static void deleteTimerQuery(TimerQuery* query);

		/**
		 * Deletes an Occlusion query. Always use this method and don't delete them manually. Actual deletion will be 
		 * delayed until next update.
		 */
		static void deleteOcclusionQuery(OcclusionQuery* query);

		/** Deletes any queued queries. */
		void processDeletedQueue();

	protected:
		mutable Vector<EventQuery*> mEventQueries;
		mutable Vector<TimerQuery*> mTimerQueries;
		mutable Vector<OcclusionQuery*> mOcclusionQueries;

		mutable Vector<EventQuery*> mDeletedEventQueries;
		mutable Vector<TimerQuery*> mDeletedTimerQueries;
		mutable Vector<OcclusionQuery*> mDeletedOcclusionQueries;
	};

	/** @} */
}}