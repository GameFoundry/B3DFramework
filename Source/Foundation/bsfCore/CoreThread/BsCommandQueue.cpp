//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCommandQueue.h"
#include "Error/BsException.h"
#include "CoreThread/BsCoreThread.h"
#include "Debug/BsDebug.h"

namespace bs
{
#if BS_DEBUG_MODE
	CommandQueueBase::CommandQueueBase(ThreadId threadId)
		:mMyThreadId(threadId), mMaxDebugIdx(0)
	{
		mAsyncOpSyncData = bs_shared_ptr_new<AsyncOpSyncData>();
		mCommands = bs_new<bs::Queue<QueuedCommand>>();

		{
			Lock Lock(CommandQueueBreakpointMutex);

			mCommandQueueIdx = MaxCommandQueueIdx++;
		}
	}
#else
	CommandQueueBase::CommandQueueBase(ThreadId threadId)
		:mMyThreadId(threadId)
	{
		mAsyncOpSyncData = bs_shared_ptr_new<AsyncOpSyncData>();
		mCommands = bs_new<bs::Queue<QueuedCommand>>();
	}
#endif

	CommandQueueBase::~CommandQueueBase()
	{
		if(mCommands != nullptr)
			bs_delete(mCommands);

		while(!mEmptyCommandQueues.empty())
		{
			bs_delete(mEmptyCommandQueues.Top());
			mEmptyCommandQueues.pop();
		}
	}

	AsyncOp CommandQueueBase::QueueReturn(std::function<void(AsyncOp&)> commandCallback, bool _notifyWhenComplete, UINT32 _callbackId)
	{
#if BS_DEBUG_MODE
		breakIfNeeded(mCommandQueueIdx, mMaxDebugIdx);

		QueuedCommand NewCommand(commandCallback, mMaxDebugIdx++, mAsyncOpSyncData, _notifyWhenComplete, _callbackId);
#else
		QueuedCommand NewCommand(commandCallback, mAsyncOpSyncData, _notifyWhenComplete, _callbackId);
#endif

		mCommands->Push(newCommand);

#if BS_FORCE_SINGLETHREADED_RENDERING
		Queue<QueuedCommand>* commands = flush();
		playback(commands);
#endif

		return newCommand.asyncOp;
	}

	void CommandQueueBase::Queue(std::function<void()> commandCallback, bool _notifyWhenComplete, UINT32 _callbackId)
	{
#if BS_DEBUG_MODE
		breakIfNeeded(mCommandQueueIdx, mMaxDebugIdx);

		QueuedCommand NewCommand(commandCallback, mMaxDebugIdx++, _notifyWhenComplete, _callbackId);
#else
		QueuedCommand NewCommand(commandCallback, _notifyWhenComplete, _callbackId);
#endif

		mCommands->Push(newCommand);

#if BS_FORCE_SINGLETHREADED_RENDERING
		Queue<QueuedCommand>* commands = flush();
		playback(commands);
#endif
	}

	bs::Queue<QueuedCommand>* CommandQueueBase::flush()
	{
		bs::Queue<QueuedCommand>* oldCommands = mCommands;

		Lock Lock(mEmptyCommandQueueMutex);
		if(!mEmptyCommandQueues.empty())
		{
			mCommands = mEmptyCommandQueues.Top();
			mEmptyCommandQueues.pop();
		}
		else
		{
			mCommands = bs_new<bs::Queue<QueuedCommand>>();
		}

		return oldCommands;
	}

	void CommandQueueBase::PlaybackWithNotify(bs::Queue<QueuedCommand>* commands, std::function<void(UINT32)> notifyCallback)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(commands == nullptr)
			return;

		while(!commands->Empty())
		{
			QueuedCommand& command = commands->Front();

			if(command.returnsValue)
			{
				AsyncOp& op = command.asyncOp;
				command.CallbackWithReturnValue(op);

				if(!command.asyncOp.HasCompleted())
				{
					BS_LOG(Warning, CoreThread,
						"Async operation return value wasn't resolved properly. Resolving automatically to nullptr. " \
						"Make sure to complete the operation before returning from the command callback method.");
					command.asyncOp._completeOperation(nullptr);
				}
			}
			else
			{
				command.Callback();
			}

			if(command.notifyWhenComplete && notifyCallback != nullptr)
			{
				notifyCallback(command.callbackId);
			}

			commands->Pop();
		}

		Lock Lock(mEmptyCommandQueueMutex);
		mEmptyCommandQueues.Push(commands);
	}

	void CommandQueueBase::Playback(bs::Queue<QueuedCommand>* commands)
	{
		playbackWithNotify(commands, std::function<void(UINT32)>());
	}

	void CommandQueueBase::CancelAll()
	{
		bs::Queue<QueuedCommand>* commands = flush();

		while(!commands->Empty())
			commands->Pop();

		Lock Lock(mEmptyCommandQueueMutex);
		mEmptyCommandQueues.Push(commands);
	}

	bool CommandQueueBase::IsEmpty()
	{
		if(mCommands != nullptr && mCommands->Size() > 0)
			return false;

		return true;
	}

	void CommandQueueBase::ThrowInvalidThreadException(const String& message) const
	{
		BS_EXCEPT(InternalErrorException, message);
	}

#if BS_DEBUG_MODE
	Mutex CommandQueueBase::CommandQueueBreakpointMutex;

	UINT32 CommandQueueBase::MaxCommandQueueIdx = 0;

	UnorderedSet<CommandQueueBase::QueueBreakpoint, CommandQueueBase::QueueBreakpoint::HashFunction,
		CommandQueueBase::QueueBreakpoint::EqualFunction> CommandQueueBase::SetBreakpoints;

	inline size_t CommandQueueBase::QueueBreakpoint::HashFunction::operator()(const QueueBreakpoint& v) const
	{
		size_t seed = 0;
		bs_hash_combine(seed, v.queueIdx);
		bs_hash_combine(seed, v.commandIdx);
		return seed;
	}

	inline bool CommandQueueBase::QueueBreakpoint::EqualFunction::operator()(const QueueBreakpoint &a, const QueueBreakpoint &b) const
	{
		return a.queueIdx == b.queueIdx && a.commandIdx == b.commandIdx;
	}

	void CommandQueueBase::AddBreakpoint(UINT32 queueIdx, UINT32 commandIdx)
	{
		Lock Lock(CommandQueueBreakpointMutex);

		SetBreakpoints.Insert(QueueBreakpoint(queueIdx, commandIdx));
	}

	void CommandQueueBase::BreakIfNeeded(UINT32 queueIdx, UINT32 commandIdx)
	{
		// I purposely don't use a mutex here, as this gets called very often. Generally breakpoints
		// will only be added at the start of the application, so race conditions should not occur.
		auto iterFind = SetBreakpoints.find(QueueBreakpoint(queueIdx, commandIdx));

		if(iterFind != SetBreakpoints.end())
		{
			assert(false && "Command queue breakpoint triggered!");
		}
	}
#else
	void CommandQueueBase::AddBreakpoint(UINT32 queueIdx, UINT32 commandIdx)
	{
		// Do nothing, no breakpoints in release
	}
#endif
}
