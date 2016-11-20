//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAsyncOp.h"
#include <functional>

namespace bs
{
	/** @addtogroup CoreThread-Internal
	 *  @{
	 */

	/**
	 * Command queue policy that provides no synchonization. Should be used with command queues that are used on a single 
	 * thread only.
	 */
	class CommandQueueNoSync
	{
	public:
		CommandQueueNoSync() {}
		virtual ~CommandQueueNoSync() {}

		bool isValidThread(ThreadId ownerThread) const
		{
			return BS_THREAD_CURRENT_ID == ownerThread;
		}

		void lock() { };
		void unlock() { }
	};

	/**
	 * Command queue policy that provides synchonization. Should be used with command queues that are used on multiple 
	 * threads.
	 */
	class CommandQueueSync
	{
	public:
		CommandQueueSync()
			:mLock(mCommandQueueMutex, std::defer_lock)
		{ }
		virtual ~CommandQueueSync() {}

		bool isValidThread(ThreadId ownerThread) const
		{
			return true;
		}

		void lock() 
		{
			mLock.lock();
		};

		void unlock()
		{
			mLock.unlock();
		}

	private:
		Mutex mCommandQueueMutex;
		Lock mLock;
	};

	/**
	 * Represents a single queued command in the command list. Contains all the data for executing the command and checking 
	 * up on the command status.
	 */
	struct QueuedCommand
	{
#if BS_DEBUG_MODE
		QueuedCommand(std::function<void(AsyncOp&)> _callback, UINT32 _debugId, const SPtr<AsyncOpSyncData>& asyncOpSyncData,
			bool _notifyWhenComplete = false, UINT32 _callbackId = 0)
			: debugId(_debugId), callbackWithReturnValue(_callback), asyncOp(asyncOpSyncData), returnsValue(true)
			, callbackId(_callbackId), notifyWhenComplete(_notifyWhenComplete)
		{ }

		QueuedCommand(std::function<void()> _callback, UINT32 _debugId, bool _notifyWhenComplete = false, UINT32 _callbackId = 0)
			:debugId(_debugId), callback(_callback), asyncOp(AsyncOpEmpty()), returnsValue(false), callbackId(_callbackId)
			, notifyWhenComplete(_notifyWhenComplete)
		{ }

		UINT32 debugId;
#else
		QueuedCommand(std::function<void(AsyncOp&)> _callback, const SPtr<AsyncOpSyncData>& asyncOpSyncData, 
			bool _notifyWhenComplete = false, UINT32 _callbackId = 0)
			:callbackWithReturnValue(_callback), returnsValue(true), notifyWhenComplete(_notifyWhenComplete), 
			callbackId(_callbackId), asyncOp(asyncOpSyncData)
		{ }

		QueuedCommand(std::function<void()> _callback, bool _notifyWhenComplete = false, UINT32 _callbackId = 0)
			:callback(_callback), returnsValue(false), notifyWhenComplete(_notifyWhenComplete), callbackId(_callbackId), asyncOp(AsyncOpEmpty())
		{ }
#endif

		~QueuedCommand()
		{ }

		QueuedCommand(const QueuedCommand& source)
		{
			callback = source.callback;
			callbackWithReturnValue = source.callbackWithReturnValue;
			asyncOp = source.asyncOp;
			returnsValue = source.returnsValue;
			callbackId = source.callbackId;
			notifyWhenComplete = source.notifyWhenComplete;

#if BS_DEBUG_MODE
			debugId = source.debugId;
#endif
		}

		QueuedCommand& operator=(const QueuedCommand& rhs)
		{
			callback = rhs.callback;
			callbackWithReturnValue = rhs.callbackWithReturnValue;
			asyncOp = rhs.asyncOp;
			returnsValue = rhs.returnsValue;
			callbackId = rhs.callbackId;
			notifyWhenComplete = rhs.notifyWhenComplete;
			
#if BS_DEBUG_MODE
			debugId = rhs.debugId;
#endif

			return *this;
		}

		std::function<void()> callback;
		std::function<void(AsyncOp&)> callbackWithReturnValue;
		AsyncOp asyncOp;
		bool returnsValue;
		UINT32 callbackId;
		bool notifyWhenComplete;
	};

	/** Manages a list of commands that can be queued for later execution on the core thread. */
	class BS_CORE_EXPORT CommandQueueBase
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param[in]	threadId	   	Identifier for the thread the command queue will be getting commands from.					
		 */
		CommandQueueBase(ThreadId threadId);
		virtual ~CommandQueueBase();

		/**
		 * Gets the thread identifier the command queue is used on.
		 * 			
		 * @note	If the command queue is using a synchonized access policy generally this is not relevant as it may be 
		 *			used on multiple threads.
		 */
		ThreadId getThreadId() const { return mMyThreadId; }

		/**
		 * Executes all provided commands one by one in order. To get the commands you should call flush().
		 *
		 * @param[in]	commands			Commands to execute.
		 * @param[in]	notifyCallback  	Callback that will be called if a command that has @p notifyOnComplete flag set.
		 * 									The callback will receive @p callbackId of the command.
		 */
		void playbackWithNotify(Queue<QueuedCommand>* commands, std::function<void(UINT32)> notifyCallback);

		/** Executes all provided commands one by one in order. To get the commands you should call flush(). */
		void playback(Queue<QueuedCommand>* commands);

		/**
		 * Allows you to set a breakpoint that will trigger when the specified command is executed.		
		 *
		 * @param[in]	queueIdx  	Zero-based index of the queue the command was queued on.
		 * @param[in]	commandIdx	Zero-based index of the command.
		 *
		 * @note	
		 * This is helpful when you receive an error on the executing thread and you cannot tell from where was the command 
		 * that caused the error queued from. However you can make a note of the queue and command index and set a 
		 * breakpoint so that it gets triggered next time you run the program. At that point you can know exactly which part
		 * of code queued the command by examining the stack trace.
		 */
		static void addBreakpoint(UINT32 queueIdx, UINT32 commandIdx);

		/**
		 * Queue up a new command to execute. Make sure the provided function has all of its parameters properly bound. 
		 * Last parameter must be unbound and of AsyncOp& type. This is used to signal that the command is completed, and 
		 * also for storing the return value.		
		 *
		 * @param[in]	commandCallback		Command to queue for execution.
		 * @param[in]	_notifyWhenComplete	(optional) Call the notify method (provided in the call to playback())
		 * 									when the command is complete.
		 * @param[in]	_callbackId			(optional) Identifier for the callback so you can then later find it
		 * 									if needed.
		 *
		 * @return							Async operation object that you can continuously check until the command 
		 *									completes. After it completes AsyncOp::isResolved() will return true and return 
		 *									data will be valid (if the callback provided any).
		 *
		 * @note	
		 * Callback method also needs to call AsyncOp::markAsResolved once it is done processing. (If it doesn't it will 
		 * still be called automatically, but the return value will default to nullptr)
		 */
		AsyncOp queueReturn(std::function<void(AsyncOp&)> commandCallback, bool _notifyWhenComplete = false, UINT32 _callbackId = 0);

		/**
		 * Queue up a new command to execute. Make sure the provided function has all of its parameters properly bound. 
		 * Provided command is not expected to return a value. If you wish to return a value from the callback use the 
		 * queueReturn() which accepts an AsyncOp parameter.
		 *
		 * @param[in]	commandCallback		Command to queue for execution.
		 * @param[in]	_notifyWhenComplete	(optional) Call the notify method (provided in the call to playback())
		 * 									when the command is complete.
		 * @param[in]	_callbackId		   	(optional) Identifier for the callback so you can then later find
		 * 									it if needed.
		 */
		void queue(std::function<void()> commandCallback, bool _notifyWhenComplete = false, UINT32 _callbackId = 0);

		/**
		 * Returns a copy of all queued commands and makes room for new ones. Must be called from the thread that created 
		 * the command queue. Returned commands must be passed to playback() method.
		 */
		Queue<QueuedCommand>* flush();

		/** Cancels all currently queued commands. */
		void cancelAll();

		/**	Returns true if no commands are queued. */
		bool isEmpty();

	protected:
		/**
		 * Helper method that throws an "Invalid thread" exception. Used primarily so we can avoid including Exception 
		 * include in this header.
		 */
		void throwInvalidThreadException(const String& message) const;

	private:
		Queue<QueuedCommand>* mCommands;
		Stack<Queue<QueuedCommand>*> mEmptyCommandQueues; /**< List of empty queues for reuse. */

		SPtr<AsyncOpSyncData> mAsyncOpSyncData;
		ThreadId mMyThreadId;

		// Various variables that allow for easier debugging by allowing us to trigger breakpoints
		// when a certain command was queued.
#if BS_DEBUG_MODE
		struct QueueBreakpoint
		{
			class HashFunction
			{
			public:
				size_t operator()(const QueueBreakpoint &key) const;
			};

			class EqualFunction
			{
			public:
				bool operator()(const QueueBreakpoint &a, const QueueBreakpoint &b) const;
			};

			QueueBreakpoint(UINT32 _queueIdx, UINT32 _commandIdx)
				:queueIdx(_queueIdx), commandIdx(_commandIdx)
			{ }

			UINT32 queueIdx;
			UINT32 commandIdx;

			inline size_t operator()(const QueueBreakpoint& v) const;
		};

		UINT32 mMaxDebugIdx;
		UINT32 mCommandQueueIdx;
		
		static UINT32 MaxCommandQueueIdx;
		static UnorderedSet<QueueBreakpoint, QueueBreakpoint::HashFunction, QueueBreakpoint::EqualFunction> SetBreakpoints;
		static Mutex CommandQueueBreakpointMutex;

		/** Checks if the specified command has a breakpoint and throw an assert if it does. */
		static void breakIfNeeded(UINT32 queueIdx, UINT32 commandIdx);
#endif
	};

	/**
	 * @copydoc CommandQueueBase
	 * 			
	 * Use SyncPolicy to choose whether you want command queue be synchonized or not. Synchonized command queues may be 
	 * used across multiple threads and non-synchonized only on one.
	 */
	template<class SyncPolicy = CommandQueueNoSync>
	class CommandQueue : public CommandQueueBase, public SyncPolicy
	{
	public:
		/** @copydoc CommandQueueBase::CommandQueueBase */
		CommandQueue(ThreadId threadId)
			:CommandQueueBase(threadId)
		{ }

		~CommandQueue() 
		{ }

		/** @copydoc CommandQueueBase::queueReturn */
		AsyncOp queueReturn(std::function<void(AsyncOp&)> commandCallback, bool _notifyWhenComplete = false, UINT32 _callbackId = 0)
		{
#if BS_DEBUG_MODE
#if BS_THREAD_SUPPORT != 0
			if(!this->isValidThread(getThreadId()))
				throwInvalidThreadException("Command queue accessed outside of its creation thread.");
#endif
#endif

			this->lock();
			AsyncOp asyncOp = CommandQueueBase::queueReturn(commandCallback, _notifyWhenComplete, _callbackId);
			this->unlock();

			return asyncOp;
		}

		/** @copydoc CommandQueueBase::queue */
		void queue(std::function<void()> commandCallback, bool _notifyWhenComplete = false, UINT32 _callbackId = 0)
		{
#if BS_DEBUG_MODE
#if BS_THREAD_SUPPORT != 0
			if(!this->isValidThread(getThreadId()))
				throwInvalidThreadException("Command queue accessed outside of its creation thread.");
#endif
#endif

			this->lock();
			CommandQueueBase::queue(commandCallback, _notifyWhenComplete, _callbackId);
			this->unlock();
		}

		/** @copydoc CommandQueueBase::flush */
		bs::Queue<QueuedCommand>* flush()
		{
#if BS_DEBUG_MODE
#if BS_THREAD_SUPPORT != 0
			if(!this->isValidThread(getThreadId()))
				throwInvalidThreadException("Command queue accessed outside of its creation thread.");
#endif
#endif

			this->lock();
			bs::Queue<QueuedCommand>* commands = CommandQueueBase::flush();
			this->unlock();

			return commands;
		}

		/** @copydoc CommandQueueBase::cancelAll */
		void cancelAll()
		{
#if BS_DEBUG_MODE
#if BS_THREAD_SUPPORT != 0
			if(!this->isValidThread(getThreadId()))
				throwInvalidThreadException("Command queue accessed outside of its creation thread.");
#endif
#endif

			this->lock();
			CommandQueueBase::cancelAll();
			this->unlock();
		}

		/** @copydoc CommandQueueBase::isEmpty */
		bool isEmpty()
		{
#if BS_DEBUG_MODE
#if BS_THREAD_SUPPORT != 0
			if(!this->isValidThread(getThreadId()))
				throwInvalidThreadException("Command queue accessed outside of its creation thread.");
#endif
#endif

			this->lock();
			bool empty = CommandQueueBase::isEmpty();
			this->unlock();

			return empty;
		}
	};

	/** @} */
}