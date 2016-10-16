//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Represents a GPU query that gets triggered when GPU starts processing the query.
	 * 			
	 * @note	
	 * Normally GPU will have many commands in its command buffer. When begin() is called it is placed in that command 
	 * buffer. Once the buffer empties and GPU reaches the EventQuery command, the query callback is triggered.
	 * @note			
	 * Core thread only.
	 */
	class BS_CORE_EXPORT EventQuery
	{
	public:
		EventQuery()
			:mActive(false) {}
		virtual ~EventQuery() {}

		/**
		 * Starts the query. 
		 * 			
		 * @note	
		 * Once the query is started you may poll isReady() method to check when query has finished, or you may hook up 
		 * an #onTriggered callback and be notified that way.
		 */
		virtual void begin() = 0;

		/** Check if GPU has processed the query. */
		virtual bool isReady() const = 0;

		/**	Triggered when GPU starts processing the query. */
		Event<void()> onTriggered;

		/**	
		 * Creates a new query, but does not schedule it on GPU. 
		 * 
		 * @param[in]	deviceIdx	Index of the GPU device to create the query on. 
		 */
		static SPtr<EventQuery> create(UINT32 deviceIdx = 0);

	protected:
		friend class QueryManager;

		/**	Returns true if the has still not been completed by the GPU. */
		bool isActive() const { return mActive; }
		void setActive(bool active) { mActive = active; }

	protected:
		bool mActive;
	};

	/** @} */
}