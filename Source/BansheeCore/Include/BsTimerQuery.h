//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Represents a GPU query that measures execution time of GPU operations. The query will measure any GPU operations 
	 * that take place between its begin() and end() calls.
	 * 			
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT TimerQuery
	{
	public:
		virtual ~TimerQuery() {}

		/**
		 * Starts the counter. 
		 * 
		 * @param[in]	cb		Optional command buffer to queue the operation on. If not provided operation
		 *						is executed on the main command buffer. Otherwise it is executed when 
		 *						RenderAPI::executeCommands() is called. Buffer must support graphics or compute operations.
		 *									
		 * @note	Place any commands you want to measure after this call. Call "end" when done.
		 */
		virtual void begin(const SPtr<CommandBuffer>& cb = nullptr) = 0;

		/**	
		 * Stops the counter. 
		 *
		 * @param[in]	cb		Command buffer that was provided to the last begin() operation (if any).
		 */
		virtual void end(const SPtr<CommandBuffer>& cb = nullptr) = 0;

		/**	Check if GPU has processed the query. */
		virtual bool isReady() const = 0;

		/**
		 * Returns the time it took for the query to execute.
		 *
		 * @return	The time milliseconds.
		 * 			
		 * @note	Only valid after isReady() returns true.
		 */
		virtual float getTimeMs() = 0;

		/** Triggered when GPU processes the query. As a parameter it provides query duration in milliseconds. */
		Event<void(float)> onTriggered;

		/**	
		 * Creates a new query, but does not schedule it on GPU. 
		 * 
		 * @param[in]	deviceIdx	Index of the GPU device to create the query on. 
		 */
		static SPtr<TimerQuery> create(UINT32 deviceIdx = 0);

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