//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"

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
			B3D_ASSERT(!HandleLinks && !IsActive);
		}

		virtual void Deactivate()
		{
			IsActive = false;
		}

		BaseConnectionData* Prev = nullptr;
		BaseConnectionData* Next = nullptr;
		bool IsActive = true;
		u32 HandleLinks = 0;
	};

	/** Internal data for an Event, storing all connections. */
	struct EventInternalData
	{
		EventInternalData() = default;

		~EventInternalData()
		{
			BaseConnectionData* conn = MConnections;
			while(conn != nullptr)
			{
				BaseConnectionData* next = conn->Next;
				B3DFree(conn);

				conn = next;
			}

			conn = MFreeConnections;
			while(conn != nullptr)
			{
				BaseConnectionData* next = conn->Next;
				B3DFree(conn);

				conn = next;
			}

			conn = MNewConnections;
			while(conn != nullptr)
			{
				BaseConnectionData* next = conn->Next;
				B3DFree(conn);

				conn = next;
			}
		}

		/** Appends a new connection to the active connection array. */
		void Connect(BaseConnectionData* conn)
		{
			conn->Prev = MLastConnection;

			if(MLastConnection != nullptr)
				MLastConnection->Next = conn;

			MLastConnection = conn;

			// First connection
			if(MConnections == nullptr)
				MConnections = conn;
		}

		/**
		 * Disconnects the connection with the specified data, ensuring the event doesn't call its callback again.
		 *
		 * @note	Only call this once.
		 */
		void Disconnect(BaseConnectionData* conn)
		{
			RecursiveLock lock(MMutex);

			conn->Deactivate();
			conn->HandleLinks--;

			if(conn->HandleLinks == 0)
				Free(conn);
		}

		/** Disconnects all connections in the event. */
		void Clear()
		{
			RecursiveLock lock(MMutex);

			BaseConnectionData* conn = MConnections;
			while(conn != nullptr)
			{
				BaseConnectionData* next = conn->Next;
				conn->Deactivate();

				if(conn->HandleLinks == 0)
					Free(conn);

				conn = next;
			}

			MConnections = nullptr;
			MLastConnection = nullptr;
		}

		/**
		 * Called when the event handle no longer keeps a reference to the connection data. This means we might be able to
		 * free (and reuse) its memory if the event is done with it too.
		 */
		void FreeHandle(BaseConnectionData* conn)
		{
			RecursiveLock lock(MMutex);

			conn->HandleLinks--;

			if(conn->HandleLinks == 0 && !conn->IsActive)
				Free(conn);
		}

		/** Releases connection data and makes it available for re-use when next connection is formed. */
		void Free(BaseConnectionData* conn)
		{
			if(conn->Prev != nullptr)
				conn->Prev->Next = conn->Next;
			else
				MConnections = conn->Next;

			if(conn->Next != nullptr)
				conn->Next->Prev = conn->Prev;
			else
				MLastConnection = conn->Prev;

			conn->Prev = nullptr;
			conn->Next = nullptr;

			if(MFreeConnections != nullptr)
			{
				conn->Next = MFreeConnections;
				MFreeConnections->Prev = conn;
			}

			MFreeConnections = conn;
			MFreeConnections->~BaseConnectionData();
		}

		BaseConnectionData* MConnections = nullptr;
		BaseConnectionData* MLastConnection = nullptr;
		BaseConnectionData* MFreeConnections = nullptr;
		BaseConnectionData* MNewConnections = nullptr;

		RecursiveMutex MMutex;
		bool MIsCurrentlyTriggering = false;
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
			: mConnection(connection), mEventData(std::move(eventData))
		{
			connection->HandleLinks++;
		}

		~HEvent()
		{
			if(mConnection != nullptr)
				mEventData->FreeHandle(mConnection);
		}

		/** Disconnect from the event you are subscribed to. */
		void Disconnect()
		{
			if(mConnection != nullptr)
			{
				mEventData->Disconnect(mConnection);
				mConnection = nullptr;
				mEventData = nullptr;
			}
		}

		/** @cond IGNORE */

		struct Bool_struct
		{
			int Member;
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
			return (mConnection != nullptr ? &Bool_struct::Member : 0);
		}

		HEvent& operator=(const HEvent& rhs)
		{
			mConnection = rhs.mConnection;
			mEventData = rhs.mEventData;

			if(mConnection != nullptr)
				mConnection->HandleLinks++;

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
			void Deactivate() override
			{
				Func = nullptr;

				BaseConnectionData::Deactivate();
			}

			std::function<RetType(Args...)> Func;
		};

	public:
		TEvent()
			: mInternalData(B3DMakeShared<EventInternalData>())
		{}

		~TEvent()
		{
			Clear();
		}

		/** Register a new callback that will get notified once the event is triggered. */
		HEvent Connect(std::function<RetType(Args...)> func)
		{
			RecursiveLock lock(mInternalData->MMutex);

			ConnectionData* connData = nullptr;
			if(mInternalData->MFreeConnections != nullptr)
			{
				connData = static_cast<ConnectionData*>(mInternalData->MFreeConnections);
				mInternalData->MFreeConnections = connData->Next;

				new(connData) ConnectionData();
				if(connData->Next != nullptr)
					connData->Next->Prev = nullptr;

				connData->IsActive = true;
			}

			if(connData == nullptr)
				connData = B3DNew<ConnectionData>();

			// If currently iterating over the connection list, delay modifying it until done
			if(mInternalData->MIsCurrentlyTriggering)
			{
				connData->Prev = mInternalData->MNewConnections;

				if(mInternalData->MNewConnections != nullptr)
					mInternalData->MNewConnections->Next = connData;

				mInternalData->MNewConnections = connData;
			}
			else
			{
				mInternalData->Connect(connData);
			}

			connData->Func = func;

			return HEvent(mInternalData, connData);
		}

		/** Trigger the event, notifying all register callback methods. */
		void operator()(Args... args)
		{
			// Increase ref count to ensure this event data isn't destroyed if one of the callbacks
			// deletes the event itself.
			SPtr<EventInternalData> internalData = mInternalData;

			RecursiveLock lock(internalData->MMutex);
			internalData->MIsCurrentlyTriggering = true;

			ConnectionData* conn = static_cast<ConnectionData*>(internalData->MConnections);
			while(conn != nullptr)
			{
				// Save next here in case the callback itself disconnects this connection
				ConnectionData* next = static_cast<ConnectionData*>(conn->Next);

				if(conn->Func != nullptr)
					conn->Func(std::forward<Args>(args)...);

				conn = next;
			}

			internalData->MIsCurrentlyTriggering = false;

			// If any new connections were added during the above calls, add them to the connection list
			if(internalData->MNewConnections != nullptr)
			{
				BaseConnectionData* lastNewConnection = internalData->MNewConnections;
				while(lastNewConnection != nullptr)
					lastNewConnection = lastNewConnection->Next;

				BaseConnectionData* currentConnection = lastNewConnection;
				while(currentConnection != nullptr)
				{
					BaseConnectionData* prevConnection = currentConnection->Prev;
					currentConnection->Next = nullptr;
					currentConnection->Prev = nullptr;

					mInternalData->Connect(currentConnection);
					currentConnection = prevConnection;
				}

				internalData->MNewConnections = nullptr;
			}
		}

		/** Clear all callbacks from the event. */
		void Clear()
		{
			mInternalData->Clear();
		}

		/**
		 * Check if event has any callbacks registered.
		 *
		 * @note	It is safe to trigger an event even if no callbacks are registered.
		 */
		bool Empty() const
		{
			RecursiveLock lock(mInternalData->MMutex);

			return mInternalData->MConnections == nullptr;
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
	class Event<RetType(Args...)> : public TEvent<RetType, Args...>
	{};

	/** @} */
} // namespace bs
