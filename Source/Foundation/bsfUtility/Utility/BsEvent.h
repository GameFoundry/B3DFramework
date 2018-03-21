//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup General-Internal
	 *  @{
	 */

	/** Data common to all event connections. */
	class BaseConnectionData
	{
	public:
		BaseConnectionData() = default;

		virtual ~BaseConnectionData()
		{
			assert(!handleLinks && !isActive);
		}

		virtual void deactivate()
		{
			isActive = false;
		}

		BaseConnectionData* prev = nullptr;
		BaseConnectionData* next = nullptr;
		bool isActive = true;
		UINT32 handleLinks = 0;
	};

	/** Internal data for an Event, storing all connections. */
	struct EventInternalData
	{
		EventInternalData() = default;

		~EventInternalData()
		{
			BaseConnectionData* conn = mConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				bs_free(conn);

				conn = next;
			}

			conn = mFreeConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				bs_free(conn);

				conn = next;
			}

			conn = mNewConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				bs_free(conn);

				conn = next;
			}
		}

		/** Appends a new connection to the active connection array. */
		void connect(BaseConnectionData* conn)
		{
			conn->prev = mLastConnection;

			if (mLastConnection != nullptr)
				mLastConnection->next = conn;

			mLastConnection = conn;

			// First connection
			if (mConnections == nullptr)
				mConnections = conn;
		}

		/**
		 * Disconnects the connection with the specified data, ensuring the event doesn't call its callback again.
		 *
		 * @note	Only call this once.
		 */
		void disconnect(BaseConnectionData* conn)
		{
			RecursiveLock lock(mMutex);

			conn->deactivate();
			conn->handleLinks--;

			if (conn->handleLinks == 0)
				free(conn);
		}

		/** Disconnects all connections in the event. */
		void clear()
		{
			RecursiveLock lock(mMutex);

			BaseConnectionData* conn = mConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				conn->deactivate();

				if (conn->handleLinks == 0)
					free(conn);

				conn = next;
			}

			mConnections = nullptr;
			mLastConnection = nullptr;
		}

		/**
		 * Called when the event handle no longer keeps a reference to the connection data. This means we might be able to 
		 * free (and reuse) its memory if the event is done with it too.
		 */
		void freeHandle(BaseConnectionData* conn)
		{
			RecursiveLock lock(mMutex);

			conn->handleLinks--;

			if (conn->handleLinks == 0 && !conn->isActive)
				free(conn);
		}

		/** Releases connection data and makes it available for re-use when next connection is formed. */
		void free(BaseConnectionData* conn)
		{
			if (conn->prev != nullptr)
				conn->prev->next = conn->next;
			else
				mConnections = conn->next;

			if (conn->next != nullptr)
				conn->next->prev = conn->prev;
			else
				mLastConnection = conn->prev;

			conn->prev = nullptr;
			conn->next = nullptr;

			if (mFreeConnections != nullptr)
			{
				conn->next = mFreeConnections;
				mFreeConnections->prev = conn;
			}

			mFreeConnections = conn;
			mFreeConnections->~BaseConnectionData();
		}

		BaseConnectionData* mConnections = nullptr;
		BaseConnectionData* mLastConnection = nullptr;
		BaseConnectionData* mFreeConnections = nullptr;
		BaseConnectionData* mNewConnections = nullptr;

		RecursiveMutex mMutex;
		bool mIsCurrentlyTriggering = false;
	};

	/** @} */
	/** @} */

	/** @addtogroup General
	 *  @{
	 */

	/** Event handle. Allows you to track to which events you subscribed to and disconnect from them when needed. */
	class HEvent
	{
	public:
		HEvent() = default;

		explicit HEvent(SPtr<EventInternalData> eventData, BaseConnectionData* connection)
			:mConnection(connection), mEventData(std::move(eventData))
		{
			connection->handleLinks++;
		}

		~HEvent()
		{
			if (mConnection != nullptr)
				mEventData->freeHandle(mConnection);
		}

		/** Disconnect from the event you are subscribed to. */
		void disconnect()
		{
			if (mConnection != nullptr)
			{
				mEventData->disconnect(mConnection);
				mConnection = nullptr;
				mEventData = nullptr;
			}
		}

		/** @cond IGNORE */

		struct Bool_struct
		{
			int _Member;
		};

		/** @endcond */

		/**
		* Allows direct conversion of a handle to bool.
		*
		* @note		
		* Additional struct is needed because we can't directly convert to bool since then we can assign pointer to bool 
		* and that's wrong.
		*/
		operator int Bool_struct::*() const
		{
			return (mConnection != nullptr ? &Bool_struct::_Member : 0);
		}

		HEvent& operator=(const HEvent& rhs)
		{
			mConnection = rhs.mConnection;
			mEventData = rhs.mEventData;

			if (mConnection != nullptr)
				mConnection->handleLinks++;

			return *this;
		}

	private:
		BaseConnectionData* mConnection = nullptr;
		SPtr<EventInternalData> mEventData;
	};	

	/** @} */

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup General-Internal
	 *  @{
	 */

	/**
	 * Events allows you to register method callbacks that get notified when the event is triggered.
	 *
	 * @note	Callback method return value is ignored.
	 */
	// Note: I could create a policy template argument that allows creation of 
	// lockable and non-lockable events in the case mutex is causing too much overhead.
	template <class RetType, class... Args>
	class TEvent
	{
		struct ConnectionData : BaseConnectionData
		{
		public:
			void deactivate() override
			{
				func = nullptr;

				BaseConnectionData::deactivate();
			}

			std::function<RetType(Args...)> func;
		};

	public:
		TEvent()
			:mInternalData(bs_shared_ptr_new<EventInternalData>())
		{ }

		~TEvent()
		{
			clear();
		}

		/** Register a new callback that will get notified once the event is triggered. */
		HEvent connect(std::function<RetType(Args...)> func)
		{
			RecursiveLock lock(mInternalData->mMutex);

			ConnectionData* connData = nullptr;
			if (mInternalData->mFreeConnections != nullptr)
			{
				connData = static_cast<ConnectionData*>(mInternalData->mFreeConnections);
				mInternalData->mFreeConnections = connData->next;

				new (connData)ConnectionData();
				if (connData->next != nullptr)
					connData->next->prev = nullptr;

				connData->isActive = true;
			}

			if (connData == nullptr)
				connData = bs_new<ConnectionData>();

			// If currently iterating over the connection list, delay modifying it until done
			if(mInternalData->mIsCurrentlyTriggering)
			{
				connData->prev = mInternalData->mNewConnections;

				if (mInternalData->mNewConnections != nullptr)
					mInternalData->mNewConnections->next = connData;

				mInternalData->mNewConnections = connData;
			}
			else
			{
				mInternalData->connect(connData);
			}

			connData->func = func;

			return HEvent(mInternalData, connData);
		}

		/** Trigger the event, notifying all register callback methods. */
		void operator() (Args... args)
		{
			// Increase ref count to ensure this event data isn't destroyed if one of the callbacks
			// deletes the event itself.
			SPtr<EventInternalData> internalData = mInternalData;

			RecursiveLock lock(internalData->mMutex);
			internalData->mIsCurrentlyTriggering = true;

			ConnectionData* conn = static_cast<ConnectionData*>(internalData->mConnections);
			while (conn != nullptr)
			{
				// Save next here in case the callback itself disconnects this connection
				ConnectionData* next = static_cast<ConnectionData*>(conn->next);
				
				if (conn->func != nullptr)
					conn->func(std::forward<Args>(args)...);

				conn = next;
			}

			internalData->mIsCurrentlyTriggering = false;

			// If any new connections were added during the above calls, add them to the connection list
			if(internalData->mNewConnections != nullptr)
			{
				BaseConnectionData* lastNewConnection = internalData->mNewConnections;
				while (lastNewConnection != nullptr)
					lastNewConnection = lastNewConnection->next;

				BaseConnectionData* currentConnection = lastNewConnection;
				while(currentConnection != nullptr)
				{
					BaseConnectionData* prevConnection = currentConnection->prev;
					currentConnection->next = nullptr;
					currentConnection->prev = nullptr;

					mInternalData->connect(currentConnection);
					currentConnection = prevConnection;
				}

				internalData->mNewConnections = nullptr;
			}
		}

		/** Clear all callbacks from the event. */
		void clear()
		{
			mInternalData->clear();
		}

		/**
		 * Check if event has any callbacks registered.
		 *
		 * @note	It is safe to trigger an event even if no callbacks are registered.
		 */
		bool empty() const
		{
			RecursiveLock lock(mInternalData->mMutex);

			return mInternalData->mConnections == nullptr;
		}

	private:
		SPtr<EventInternalData> mInternalData;
	};

	/** @} */
	/** @} */

	/** @addtogroup General
	 *  @{
	 */

	/************************************************************************/
	/* 							SPECIALIZATIONS                      		*/
	/* 	SO YOU MAY USE FUNCTION LIKE SYNTAX FOR DECLARING EVENT SIGNATURE   */
	/************************************************************************/
	
	/** @copydoc TEvent */
	template <typename Signature>
	class Event;

	/** @copydoc TEvent */
	template <class RetType, class... Args>
	class Event<RetType(Args...) > : public TEvent <RetType, Args...>
	{ };

	/** @} */
}
