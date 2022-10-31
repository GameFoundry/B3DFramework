//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCommandQueue.h"
#include "Error/BsException.h"
#include "CoreThread/BsCoreThread.h"
#include "Debug/BsDebug.h"

using namespace bs;

#if BS_DEBUG_MODE
CommandQueueBase::CommandQueueBase(ThreadId threadId)
	: mMyThreadId(threadId), mMaxDebugIdx(0)
{
	mAsyncOpSyncData = B3DMakeShared<AsyncOpSyncData>();
	mCommands = B3DNew<bs::Queue<QueuedCommand>>();

	{
		Lock lock(CommandQueueBreakpointMutex);

		mCommandQueueIdx = MaxCommandQueueIdx++;
	}
}
#else
CommandQueueBase::CommandQueueBase(ThreadId threadId)
	: mMyThreadId(threadId)
{
	mAsyncOpSyncData = B3DMakeShared<AsyncOpSyncData>();
	mCommands = B3DNew<bs::Queue<QueuedCommand>>();
}
#endif

CommandQueueBase::~CommandQueueBase()
{
	if(mCommands != nullptr)
		B3DDelete(mCommands);

	while(!mEmptyCommandQueues.empty())
	{
		B3DDelete(mEmptyCommandQueues.top());
		mEmptyCommandQueues.pop();
	}
}

AsyncOp CommandQueueBase::QueueReturn(std::function<void(AsyncOp&)> commandCallback, bool _notifyWhenComplete, u32 _callbackId)
{
#if BS_DEBUG_MODE
	BreakIfNeeded(mCommandQueueIdx, mMaxDebugIdx);

	QueuedCommand newCommand(commandCallback, mMaxDebugIdx++, mAsyncOpSyncData, _notifyWhenComplete, _callbackId);
#else
	QueuedCommand newCommand(commandCallback, mAsyncOpSyncData, _notifyWhenComplete, _callbackId);
#endif

	mCommands->push(newCommand);

#if BS_FORCE_SINGLETHREADED_RENDERING
	Queue<QueuedCommand>* commands = flush();
	playback(commands);
#endif

	return newCommand.AsyncOp;
}

void CommandQueueBase::Queue(std::function<void()> commandCallback, bool _notifyWhenComplete, u32 _callbackId)
{
#if BS_DEBUG_MODE
	BreakIfNeeded(mCommandQueueIdx, mMaxDebugIdx);

	QueuedCommand newCommand(commandCallback, mMaxDebugIdx++, _notifyWhenComplete, _callbackId);
#else
	QueuedCommand newCommand(commandCallback, _notifyWhenComplete, _callbackId);
#endif

	mCommands->push(newCommand);

#if BS_FORCE_SINGLETHREADED_RENDERING
	Queue<QueuedCommand>* commands = flush();
	playback(commands);
#endif
}

bs::Queue<QueuedCommand>* CommandQueueBase::Flush()
{
	bs::Queue<QueuedCommand>* oldCommands = mCommands;

	Lock lock(mEmptyCommandQueueMutex);
	if(!mEmptyCommandQueues.empty())
	{
		mCommands = mEmptyCommandQueues.top();
		mEmptyCommandQueues.pop();
	}
	else
	{
		mCommands = B3DNew<bs::Queue<QueuedCommand>>();
	}

	return oldCommands;
}

void CommandQueueBase::PlaybackWithNotify(bs::Queue<QueuedCommand>* commands, std::function<void(u32)> notifyCallback)
{
	THROW_IF_NOT_CORE_THREAD;

	if(commands == nullptr)
		return;

	while(!commands->empty())
	{
		QueuedCommand& command = commands->front();

		if(command.ReturnsValue)
		{
			AsyncOp& op = command.AsyncOp;
			command.CallbackWithReturnValue(op);

			if(!command.AsyncOp.HasCompleted())
			{
				BS_LOG(Warning, CoreThread, "Async operation return value wasn't resolved properly. Resolving automatically to nullptr. "
											"Make sure to complete the operation before returning from the command callback method.");
				command.AsyncOp.CompleteOperationInternal(nullptr);
			}
		}
		else
		{
			command.Callback();
		}

		if(command.NotifyWhenComplete && notifyCallback != nullptr)
		{
			notifyCallback(command.CallbackId);
		}

		commands->pop();
	}

	Lock lock(mEmptyCommandQueueMutex);
	mEmptyCommandQueues.push(commands);
}

void CommandQueueBase::Playback(bs::Queue<QueuedCommand>* commands)
{
	PlaybackWithNotify(commands, std::function<void(u32)>());
}

void CommandQueueBase::CancelAll()
{
	bs::Queue<QueuedCommand>* commands = Flush();

	while(!commands->empty())
		commands->pop();

	Lock lock(mEmptyCommandQueueMutex);
	mEmptyCommandQueues.push(commands);
}

bool CommandQueueBase::IsEmpty()
{
	if(mCommands != nullptr && mCommands->size() > 0)
		return false;

	return true;
}

void CommandQueueBase::ThrowInvalidThreadException(const String& message) const
{
	BS_EXCEPT(InternalErrorException, message);
}

#if BS_DEBUG_MODE
Mutex CommandQueueBase::CommandQueueBreakpointMutex;

u32 CommandQueueBase::MaxCommandQueueIdx = 0;

UnorderedSet<CommandQueueBase::QueueBreakpoint, CommandQueueBase::QueueBreakpoint::HashFunction, CommandQueueBase::QueueBreakpoint::EqualFunction> CommandQueueBase::SetBreakpoints;

inline size_t CommandQueueBase::QueueBreakpoint::HashFunction::operator()(const QueueBreakpoint& v) const
{
	size_t seed = 0;
	bs_hash_combine(seed, v.QueueIdx);
	bs_hash_combine(seed, v.CommandIdx);
	return seed;
}

inline bool CommandQueueBase::QueueBreakpoint::EqualFunction::operator()(const QueueBreakpoint& a, const QueueBreakpoint& b) const
{
	return a.QueueIdx == b.QueueIdx && a.CommandIdx == b.CommandIdx;
}

void CommandQueueBase::AddBreakpoint(u32 queueIdx, u32 commandIdx)
{
	Lock lock(CommandQueueBreakpointMutex);

	SetBreakpoints.insert(QueueBreakpoint(queueIdx, commandIdx));
}

void CommandQueueBase::BreakIfNeeded(u32 queueIdx, u32 commandIdx)
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
void CommandQueueBase::addBreakpoint(u32 queueIdx, u32 commandIdx)
{
	// Do nothing, no breakpoints in release
}
#endif
