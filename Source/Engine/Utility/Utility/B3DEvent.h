//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup General-Internal
	 *  @{
	 */

	/** Data common to all event connections. */
	struct EventConnection
	{
	public:
		EventConnection() = default;

		virtual ~EventConnection()
		{
			B3D_ASSERT(!ActiveHandleCount && !IsActive);
		}

		virtual void Deactivate()
		{
			IsActive = false;
		}

		EventConnection* Previous = nullptr;
		EventConnection* Next = nullptr;
		bool IsActive = true;
		u32 ActiveHandleCount = 0;
	};

	/** Internal data for an Event, storing all connections. */
	struct EventControlBlock
	{
		EventControlBlock() = default;

		~EventControlBlock()
		{
			EventConnection* connection = ActiveConnections;
			while(connection != nullptr)
			{
				EventConnection* next = connection->Next;
				B3DFree(connection);

				connection = next;
			}

			connection = FreeConnections;
			while(connection != nullptr)
			{
				EventConnection* next = connection->Next;
				B3DFree(connection);

				connection = next;
			}

			connection = NewConnections;
			while(connection != nullptr)
			{
				EventConnection* next = connection->Next;
				B3DFree(connection);

				connection = next;
			}
		}

		/** Appends a new connection to the active connection array. */
		void Connect(EventConnection* connection)
		{
			connection->Previous = LastConnection;

			if(LastConnection != nullptr)
				LastConnection->Next = connection;

			LastConnection = connection;

			// First connection
			if(ActiveConnections == nullptr)
				ActiveConnections = connection;
		}

		/**
		 * Disconnects the connection with the specified data, ensuring the event doesn't call its callback again.
		 *
		 * @note	Only call this once.
		 */
		void Disconnect(EventConnection* connection)
		{
			RecursiveLock lock(Mutex);

			connection->Deactivate();
			connection->ActiveHandleCount--;

			if(connection->ActiveHandleCount == 0)
				FreeConnection(connection);
		}

		/** Disconnects all connections in the event. */
		void DisconnectAll()
		{
			RecursiveLock lock(Mutex);

			EventConnection* connection = ActiveConnections;
			while(connection != nullptr)
			{
				EventConnection* next = connection->Next;
				connection->Deactivate();

				if(connection->ActiveHandleCount == 0)
					FreeConnection(connection);

				connection = next;
			}

			ActiveConnections = nullptr;
			LastConnection = nullptr;
		}

		/**
		 * Called when the event handle no longer keeps a reference to the connection data. This means we might be able to
		 * free (and reuse) its memory if the event is done with it too.
		 */
		void FreeHandle(EventConnection* connection)
		{
			RecursiveLock lock(Mutex);

			connection->ActiveHandleCount--;

			if(connection->ActiveHandleCount == 0 && !connection->IsActive)
				FreeConnection(connection);
		}

		/** Releases connection data and makes it available for re-use when next connection is formed. */
		void FreeConnection(EventConnection* connection)
		{
			if(connection->Previous != nullptr)
				connection->Previous->Next = connection->Next;
			else
				ActiveConnections = connection->Next;

			if(connection->Next != nullptr)
				connection->Next->Previous = connection->Previous;
			else
				LastConnection = connection->Previous;

			connection->Previous = nullptr;
			connection->Next = nullptr;

			if(FreeConnections != nullptr)
			{
				connection->Next = FreeConnections;
				FreeConnections->Previous = connection;
			}

			FreeConnections = connection;
			FreeConnections->~EventConnection();
		}

		EventConnection* ActiveConnections = nullptr;
		EventConnection* LastConnection = nullptr;
		EventConnection* FreeConnections = nullptr;
		EventConnection* NewConnections = nullptr;

		RecursiveMutex Mutex;
		bool IsCurrentlyTriggering = false;
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

		explicit HEvent(TShared<EventControlBlock> eventControlBlock, EventConnection* connection)
			: mConnection(connection), mEventControlBlock(std::move(eventControlBlock))
		{
			connection->ActiveHandleCount++;
		}

		~HEvent()
		{
			if(mConnection != nullptr)
				mEventControlBlock->FreeHandle(mConnection);
		}

		/** Disconnect from the event you are subscribed to. */
		void Disconnect()
		{
			if(mConnection != nullptr)
			{
				mEventControlBlock->Disconnect(mConnection);
				mConnection = nullptr;
				mEventControlBlock = nullptr;
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
			mEventControlBlock = rhs.mEventControlBlock;

			if(mConnection != nullptr)
				mConnection->ActiveHandleCount++;

			return *this;
		}

	private:
		EventConnection* mConnection = nullptr;
		TShared<EventControlBlock> mEventControlBlock;
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
	// Note: Create a variant of event without the overhead of std::function
	template <typename ReturnType, typename... ArgumentType>
	class TEvent
	{
		struct TEventConnection : EventConnection
		{
			void Deactivate() override
			{
				Callback = nullptr;

				EventConnection::Deactivate();
			}

			std::function<ReturnType(ArgumentType...)> Callback;
		};

	public:
		TEvent()
			: mControlBlock(B3DMakeShared2<EventControlBlock>())
		{}

		~TEvent()
		{
			Clear();
		}

		/** Register a new callback that will get notified once the event is triggered. */
		HEvent Connect(std::function<ReturnType(ArgumentType...)> callback)
		{
			RecursiveLock lock(mControlBlock->Mutex);

			TEventConnection* connection = nullptr;
			if(mControlBlock->FreeConnections != nullptr)
			{
				connection = static_cast<TEventConnection*>(mControlBlock->FreeConnections);
				mControlBlock->FreeConnections = connection->Next;

				new(connection) TEventConnection();
				if(connection->Next != nullptr)
					connection->Next->Previous = nullptr;

				connection->IsActive = true;
			}

			if(connection == nullptr)
				connection = B3DNew<TEventConnection>();

			// If currently iterating over the connection list, delay modifying it until done
			if(mControlBlock->IsCurrentlyTriggering)
			{
				connection->Previous = mControlBlock->NewConnections;

				if(mControlBlock->NewConnections != nullptr)
					mControlBlock->NewConnections->Next = connection;

				mControlBlock->NewConnections = connection;
			}
			else
			{
				mControlBlock->Connect(connection);
			}

			connection->Callback = callback;

			return HEvent(mControlBlock, connection);
		}

		/** Trigger the event, notifying all register callback methods. */
		void operator()(ArgumentType... arguments)
		{
			// Increase ref count to ensure this event data isn't destroyed if one of the callbacks deletes the event itself.
			TShared<EventControlBlock> hoistedControlBlock = mControlBlock;

			RecursiveLock lock(hoistedControlBlock->Mutex);
			hoistedControlBlock->IsCurrentlyTriggering = true;

			TEventConnection* connection = static_cast<TEventConnection*>(hoistedControlBlock->ActiveConnections);
			while(connection != nullptr)
			{
				// Save next here in case the callback itself disconnects this connection
				TEventConnection* next = static_cast<TEventConnection*>(connection->Next);

				if(connection->Callback != nullptr)
					connection->Callback(std::forward<ArgumentType>(arguments)...);

				connection = next;
			}

			hoistedControlBlock->IsCurrentlyTriggering = false;

			// If any new connections were added during the above calls, add them to the connection list
			if(hoistedControlBlock->NewConnections != nullptr)
			{
				EventConnection* lastNewConnection = hoistedControlBlock->NewConnections;
				while(lastNewConnection != nullptr)
					lastNewConnection = lastNewConnection->Next;

				EventConnection* currentConnection = lastNewConnection;
				while(currentConnection != nullptr)
				{
					EventConnection* prevConnection = currentConnection->Previous;
					currentConnection->Next = nullptr;
					currentConnection->Previous = nullptr;

					mControlBlock->Connect(currentConnection);
					currentConnection = prevConnection;
				}

				hoistedControlBlock->NewConnections = nullptr;
			}
		}

		/** Clear all callbacks from the event. */
		void Clear()
		{
			mControlBlock->DisconnectAll();
		}

		/**
		 * Check if event has any callbacks registered.
		 *
		 * @note	It is safe to trigger an event even if no callbacks are registered.
		 */
		bool Empty() const
		{
			RecursiveLock lock(mControlBlock->Mutex);

			return mControlBlock->ActiveConnections == nullptr;
		}

	private:
		TShared<EventControlBlock> mControlBlock; // Note: Use thread unsafe shared if not using a thread safe policy
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
} // namespace b3d
