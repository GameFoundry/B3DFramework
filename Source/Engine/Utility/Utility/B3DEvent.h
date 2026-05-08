//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Threading-Internal
	 *  @{
	 */

	namespace detail
	{
		/** No-op mutex for ThreadUnsafe events. Satisfies the BasicLockable concept. */
		struct NullMutex {};

		/** No-op RAII lock guard for ThreadUnsafe events. */
		struct NullLock
		{
			NullLock(NullMutex&) {}
		};
	}

	/** @} */

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

	/** Internal data for an Event, storing all connections. Templated on ThreadSafetyPolicy to conditionally include a mutex. */
	template<ThreadSafetyPolicy Policy = ThreadSafe>
	struct TEventControlBlock
	{
		using MutexType = std::conditional_t<Policy == ThreadSafe, RecursiveMutex, detail::NullMutex>;
		using LockType = std::conditional_t<Policy == ThreadSafe, RecursiveLock, detail::NullLock>;

		TEventControlBlock() = default;

		~TEventControlBlock()
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
			LockType lock(Mutex);

			connection->Deactivate();
			connection->ActiveHandleCount--;

			if(connection->ActiveHandleCount == 0)
				FreeConnection(connection);
		}

		/** Disconnects all connections in the event. */
		void DisconnectAll()
		{
			LockType lock(Mutex);

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
			LockType lock(Mutex);

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

		MutexType Mutex;
		bool IsCurrentlyTriggering = false;
	};

	/** @} */
	/** @} */

	/** @addtogroup General
	 *  @{
	 */

	/** Event handle. Allows you to track to which events you subscribed to and disconnect from them when needed. */
	template<ThreadSafetyPolicy Policy = ThreadSafe>
	class THEvent
	{
	public:
		THEvent() = default;

		explicit THEvent(TShared2<TEventControlBlock<Policy>, Policy> eventControlBlock, EventConnection* connection)
			: mConnection(connection), mEventControlBlock(std::move(eventControlBlock))
		{
			connection->ActiveHandleCount++;
		}

		~THEvent()
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

		THEvent& operator=(const THEvent& rhs)
		{
			mConnection = rhs.mConnection;
			mEventControlBlock = rhs.mEventControlBlock;

			if(mConnection != nullptr)
				mConnection->ActiveHandleCount++;

			return *this;
		}

	private:
		EventConnection* mConnection = nullptr;
		TShared2<TEventControlBlock<Policy>, Policy> mEventControlBlock;
	};

	/** Alias for thread-safe event handles. */
	using HEvent = THEvent<ThreadSafe>;

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
	 * @tparam	Policy			Thread safety policy. ThreadSafe uses a mutex and atomic ref-counting. ThreadUnsafe eliminates
	 *							all locking overhead for use in single-threaded contexts.
	 *
	 * @note	Callback method return value is ignored.
	 */
	template <typename ReturnType, ThreadSafetyPolicy Policy, typename... ArgumentType>
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

		using LockType = typename TEventControlBlock<Policy>::LockType;

	public:
		TEvent()
		{
			// We thread unsafe we allocate control block lazily to avoid overhead of unused events
			if constexpr(Policy == ThreadSafe)
				mControlBlock = B3DMakeShared2<TEventControlBlock<Policy>, Policy>();
		}

		~TEvent()
		{
			Clear();
		}

		/** Register a new callback that will get notified once the event is triggered. */
		THEvent<Policy> Connect(std::function<ReturnType(ArgumentType...)> callback)
		{
			if constexpr(Policy == ThreadUnsafe)
			{
				if(!mControlBlock)
					mControlBlock = B3DMakeShared2<TEventControlBlock<Policy>, Policy>();
			}

			LockType lock(mControlBlock->Mutex);

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

			return THEvent<Policy>(mControlBlock, connection);
		}

		/** Trigger the event, notifying all register callback methods. */
		void operator()(ArgumentType... arguments)
		{
			if constexpr(Policy == ThreadUnsafe)
			{
				// Fast path: skip when no connections exist or control block was never allocated.
				// Only safe for ThreadUnsafe events — for ThreadSafe events another thread could Connect() between this check and the lock acquisition.
				if(!mControlBlock || (mControlBlock->ActiveConnections == nullptr && mControlBlock->NewConnections == nullptr))
					return;
			}

			// Increase ref count to ensure this event data isn't destroyed if one of the callbacks deletes the event itself.
			TShared2<TEventControlBlock<Policy>, Policy> hoistedControlBlock = mControlBlock;

			LockType lock(hoistedControlBlock->Mutex);
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
			if(mControlBlock)
				mControlBlock->DisconnectAll();
		}

		/**
		 * Check if event has any callbacks registered.
		 *
		 * @note	It is safe to trigger an event even if no callbacks are registered.
		 */
		bool Empty() const
		{
			if(!mControlBlock)
				return true;

			LockType lock(mControlBlock->Mutex);

			return mControlBlock->ActiveConnections == nullptr;
		}

	private:
		TShared2<TEventControlBlock<Policy>, Policy> mControlBlock;
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
	template <typename Signature, ThreadSafetyPolicy Policy = ThreadSafe>
	class Event;

	/** @copydoc TEvent */
	template <class RetType, class... Args, ThreadSafetyPolicy Policy>
	class Event<RetType(Args...), Policy> : public TEvent<RetType, Policy, Args...>
	{};

	/** @} */
} // namespace b3d
