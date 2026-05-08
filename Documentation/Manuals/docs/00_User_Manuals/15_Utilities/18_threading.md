---
title: Threading
---

The framework provides comprehensive threading support, from basic primitives like mutexes and threads to high-level abstractions like the task scheduler and thread pools. This manual covers how to create threads, synchronize between them, and efficiently distribute work across CPU cores.

# Fibers vs threads

By default, the entire framework operates using fibers rather than traditional threads. Fibers are lightweight threads that can yield execution without blocking the entire OS thread, allowing much better CPU utilization. The @b3d::Scheduler manages a pool of worker threads (typically one per CPU core) and schedules fibers onto these threads.

When you use @b3d::Signal, @b3d::WaitGroup, or other synchronization primitives, they automatically detect whether you're running on a fiber or a thread. If on a fiber, they yield instead of blocking, allowing the thread to continue processing other fibers.

# Task scheduler

@b3d::Scheduler provides fine-grained control over task execution using fibers. It ensures optimal CPU utilization by maintaining only as many threads as there are logical CPU cores, preventing thread contention.

The scheduler stores work as @b3d::SchedulerTask objects and dispatches them to available threads using a fiber-based system that allows tasks to yield without blocking the entire thread.

## Creating and posting tasks

Create a task by constructing a @b3d::SchedulerTask with a worker function:

~~~~~~~~~~~~~{.cpp}
void WorkerFunction()
{
	// This runs on a scheduler fiber
	B3D_LOG(Info, LogGeneric, "Task executing");
}

Scheduler* scheduler = Scheduler::Get();
scheduler->Post(SchedulerTask(WorkerFunction, "MyTask"));
~~~~~~~~~~~~~

## Task flags

Tasks can have flags that control their execution behavior:

~~~~~~~~~~~~~{.cpp}
Scheduler* scheduler = Scheduler::Get();

// Ensure task runs on the same thread it was posted from
scheduler->Post(SchedulerTask(WorkerFunction, "SameThreadTask", SchedulerTaskFlag::SameThread));

// Prevent task from being stolen by other threads
scheduler->Post(SchedulerTask(WorkerFunction, "NoStealTask", SchedulerTaskFlag::NoStealing));

// Combine multiple flags
SchedulerTaskFlags flags = SchedulerTaskFlag::SameThread | SchedulerTaskFlag::NoStealing;
scheduler->Post(SchedulerTask(WorkerFunction, "RestrictedTask", flags));
~~~~~~~~~~~~~

## Waiting for tasks

You can post a task and wait for it to complete using Signal and WaitGroup:

~~~~~~~~~~~~~{.cpp}
int result = 0;
bool isComplete = false;
Signal signal;
Mutex mutex;

auto workerFunction = [&result, &isComplete, &signal, &mutex]()
{
	result = 42 * 100;

	{
		Lock lock(mutex);
		isComplete = true;
	}

	signal.NotifyOne();
};

// Post task
Scheduler* scheduler = Scheduler::Get();
scheduler->Post(SchedulerTask(workerFunction, "CalculationTask"));

// Wait for completion
Lock lock(mutex);
signal.Wait(lock, [&isComplete] { return isComplete; });

// result is now ready to use
B3D_LOG(Info, LogGeneric, "Result: {0}", result);
~~~~~~~~~~~~~

## Fiber yielding

When running within a scheduler task, you can yield execution to allow other tasks to run:

~~~~~~~~~~~~~{.cpp}
void LongRunningTask()
{
	for(int i = 0; i < 1000; i++)
	{
		// Do some work
		ProcessData(i);

		// Periodically yield to allow other tasks to run
		if(i % 100 == 0)
		{
			Fiber* fiber = Fiber::Get();
			if(fiber)
				fiber->Wait(); // Yield and resume later
		}
	}
}
~~~~~~~~~~~~~

## Scheduler initialization

The scheduler must be initialized before use. Typically this is done by the @b3d::Application, but you can also create a custom scheduler:

~~~~~~~~~~~~~{.cpp}
SchedulerInformation schedulerInfo;
schedulerInfo.WorkerThreadCount = B3D_THREAD_HARDWARE_CONCURRENCY;
schedulerInfo.FiberStackSize = 512 * 1024; // 512 KB per fiber

Scheduler* scheduler = Scheduler::Create(schedulerInfo);

// Use the scheduler
scheduler->Post(SchedulerTask(WorkerFunction, "MyTask"));

// Later, destroy the scheduler
Scheduler::Destroy(scheduler);
~~~~~~~~~~~~~

## Accessing from the current thread

Get the scheduler thread bound to the current thread:

~~~~~~~~~~~~~{.cpp}
const TShared<SchedulerThread>& schedulerThread = SchedulerThread::Get();
if(schedulerThread)
{
	// Post work directly to this thread
	schedulerThread->Post(SchedulerTask(WorkerFunction, "LocalTask"));
}
~~~~~~~~~~~~~

# Basic primitives

## Mutex

Use @b3d::Mutex and @b3d::Lock to synchronize access between multiple fibers or threads. **Lock** automatically locks the mutex when constructed, and unlocks it when it goes out of scope.

~~~~~~~~~~~~~{.cpp}
Vector<int> sharedOutput;
int startIndex = 0;
Mutex mutex;

void WorkerFunction()
{
	// Lock the mutex before modifying shared data
	// This ensures only one fiber accesses it at once
	Lock lock(mutex);
	sharedOutput.Add(startIndex++);
}

// Post tasks to the scheduler
Scheduler* scheduler = Scheduler::Get();
scheduler->Post(SchedulerTask(WorkerFunction, "TaskA"));
scheduler->Post(SchedulerTask(WorkerFunction, "TaskB"));
~~~~~~~~~~~~~

If a mutex can be locked recursively, use @b3d::RecursiveMutex and @b3d::RecursiveLock instead.

## Signal

@b3d::Signal is similar to `std::condition_variable`, but also works with fibers allowing waiting fibers to yield rather than blocking the entire thread.

~~~~~~~~~~~~~{.cpp}
bool isReady = false;
int result = 0;

Signal signal;
Mutex mutex;

void WorkerFunction()
{
	for(int i = 0; i < 100000; i++)
		result += i; // Or some more complex calculation

	// Lock the mutex so we can safely modify isReady
	{
		Lock lock(mutex);
		isReady = true;
	} // Automatically unlocked when lock goes out of scope

	// Notify everyone waiting that the signal is ready
	signal.NotifyAll();
}

// Post task to scheduler
Scheduler* scheduler = Scheduler::Get();
scheduler->Post(SchedulerTask(WorkerFunction, "CalculationTask"));

// Wait until the signal is triggered and isReady is set to true
// This yields the fiber instead of blocking the thread
Lock lock(mutex);
signal.Wait(lock, [&isReady] { return isReady; });

// result is now ready to use
B3D_LOG(Info, LogGeneric, "Result: {0}", result);
~~~~~~~~~~~~~

**Signal** provides several notification methods:

- @b3d::Signal::NotifyOne - Wakes up one waiting thread or fiber
- @b3d::Signal::NotifyAll - Wakes up all waiting threads and fibers

And several wait methods:

- @b3d::Signal::Wait - Waits until predicate returns true
- @b3d::Signal::WaitFor - Waits with a timeout duration
- @b3d::Signal::WaitUntil - Waits until a specific time point

~~~~~~~~~~~~~{.cpp}
// Wait for up to 5 seconds
Lock lock(mutex);
bool succeeded = signal.WaitFor(lock, std::chrono::seconds(5), [&isReady] { return isReady; });

if(succeeded)
{
	// Work completed within timeout
}
else
{
	// Timeout expired
}
~~~~~~~~~~~~~

## WaitGroup

@b3d::WaitGroup provides an easy way to wait for N operations to complete executing. It's particularly useful when you have multiple concurrent operations and need to wait for all of them to finish.

~~~~~~~~~~~~~{.cpp}
const int operationCount = 10;
WaitGroup waitGroup(operationCount);

Vector<int> results;
results.Resize(operationCount);
Mutex resultsMutex;

Scheduler* scheduler = Scheduler::Get();

for(int i = 0; i < operationCount; i++)
{
	auto task = [i, &results, &resultsMutex, &waitGroup]()
	{
		// Perform work
		int result = i * i;

		// Store result
		{
			Lock lock(resultsMutex);
			results[i] = result;
		}

		// Notify that this operation completed
		waitGroup.NotifyDone();
	};

	scheduler->Post(SchedulerTask(task, "ComputeTask"));
}

// Wait for all operations to complete
// This yields the fiber instead of blocking
waitGroup.Wait();

// All results are now ready
for(int result : results)
{
	B3D_LOG(Info, LogGeneric, "Result: {0}", result);
}
~~~~~~~~~~~~~

You can also increment the operation count dynamically:

~~~~~~~~~~~~~{.cpp}
WaitGroup waitGroup;

// Start with some operations
waitGroup.Increment(5);

// Later, add more operations
waitGroup.Increment(3);

// Each operation calls NotifyDone() when complete
// Wait() will block until all 8 operations complete
waitGroup.Wait();
~~~~~~~~~~~~~

## Other utilities

- @b3d::B3D_THREAD_HARDWARE_CONCURRENCY - Returns number of logical CPU cores
- @b3d::B3D_CURRENT_THREAD_ID - Returns @b3d::ThreadId of the current thread
- @b3d::B3D_THREAD_SLEEP - Pauses the current thread for a set number of milliseconds

~~~~~~~~~~~~~{.cpp}
// Get number of CPU cores
u32 coreCount = B3D_THREAD_HARDWARE_CONCURRENCY;
B3D_LOG(Info, LogGeneric, "System has {0} CPU cores", coreCount);

// Get current thread ID
ThreadId currentThread = B3D_CURRENT_THREAD_ID;

// Sleep for 100 milliseconds
B3D_THREAD_SLEEP(100);
~~~~~~~~~~~~~

## SingleConsumerQueue

@b3d::SingleConsumerQueue allows multiple threads or fibers to safely enqueue commands that are then processed sequentially by a single worker thread or fiber. This ensures commands execute in order without data races.

~~~~~~~~~~~~~{.cpp}
SingleConsumerQueue commandQueue;

// Start processing commands on a scheduler thread
Scheduler* scheduler = Scheduler::Get();
commandQueue.ScheduleRunUntilShutdown(*scheduler, false, 10ms);

// From any thread or fiber, post commands
commandQueue.PostCommand([]()
{
	B3D_LOG(Info, LogGeneric, "Command 1 executed");
}, "Command1");

commandQueue.PostCommand([]()
{
	B3D_LOG(Info, LogGeneric, "Command 2 executed");
}, "Command2");

// Commands execute sequentially in the order they were posted
~~~~~~~~~~~~~

You can wait for a command to complete:

~~~~~~~~~~~~~{.cpp}
int result = 0;

commandQueue.PostCommand([&result]()
{
	result = 42 * 100;
}, "Calculation", true); // Wait until complete

B3D_LOG(Info, LogGeneric, "Result: {0}", result);
~~~~~~~~~~~~~

Process commands manually using @b3d::SingleConsumerQueue::RunUntilIdle:

~~~~~~~~~~~~~{.cpp}
SingleConsumerQueue manualQueue;

// Post commands from various threads
commandQueue.PostCommand([]() { ProcessTask1(); }, "Task1");
commandQueue.PostCommand([]() { ProcessTask2(); }, "Task2");

// Process all queued commands
manualQueue.RunUntilIdle();
~~~~~~~~~~~~~

Run commands in a loop until shutdown:

~~~~~~~~~~~~~{.cpp}
SingleConsumerQueue workerQueue;

// On worker thread
Thread workerThread([&workerQueue]()
{
	workerQueue.RunUntilShutdown();
});

// Post work from main thread
workerQueue.PostCommand([]() { DoWork(); }, "Work");

// Request shutdown
workerQueue.PostRequestShutdownCommand(true);
~~~~~~~~~~~~~

Check if the queue is empty:

~~~~~~~~~~~~~{.cpp}
if(commandQueue.IsEmpty())
{
	B3D_LOG(Info, LogGeneric, "All commands processed");
}
~~~~~~~~~~~~~

Cancel all pending commands:

~~~~~~~~~~~~~{.cpp}
commandQueue.CancelAll();
~~~~~~~~~~~~~

# Thread pool

Instead of creating threads directly, you can use the @b3d::ThreadPool module for running threads. It maintains a pool of idle threads and reuses them, avoiding the cost of thread creation and destruction.

~~~~~~~~~~~~~{.cpp}
void WorkerFunction()
{
	// This runs on a pooled thread
	B3D_LOG(Info, LogGeneric, "Pooled thread executing");
}

// Get the thread pool instance
ThreadPool& threadPool = ThreadPool::Instance();

// Run work on a pooled thread
threadPool.Run("MyThread", &WorkerFunction);
~~~~~~~~~~~~~

When a thread completes, it returns to the pool in an idle state and can be reused for future work.

# Low-level threads

While the framework primarily uses fibers, you can create traditional OS threads when necessary using @b3d::Thread. This is typically only needed for specialized I/O operations or interfacing with third-party libraries that require dedicated threads.

~~~~~~~~~~~~~{.cpp}
void IOWorkerFunction()
{
	// This runs on a dedicated OS thread
	// Use for blocking I/O or third-party library integration
	B3D_LOG(Info, LogGeneric, "Dedicated thread executing");
}

Thread ioThread(&IOWorkerFunction);
~~~~~~~~~~~~~

For most work, prefer using the scheduler or thread pool instead of creating threads directly.
# Thread safety best practices

**Do:**
- Always use mutexes to protect shared data
- Use RAII locks (Lock, RecursiveLock) to prevent deadlocks
- Use Signal with predicates to avoid spurious wakeups
- Use WaitGroup for coordinating multiple operations
- Prefer the scheduler for CPU-bound work
- Use thread pools for I/O-bound work

**Don't:**
- Access shared data without synchronization
- Hold locks longer than necessary
- Lock mutexes in different orders (causes deadlocks)
- Create more threads than CPU cores for CPU-bound work
- Forget to notify signals after changing conditions
- Use busy-waiting (prefer Signal over polling)

# Practical examples

## Parallel processing with WaitGroup

~~~~~~~~~~~~~{.cpp}
void ProcessDataInParallel(const Vector<DataItem>& items)
{
	const u32 workerCount = B3D_THREAD_HARDWARE_CONCURRENCY;
	const u32 itemsPerWorker = (u32)items.Size() / workerCount;

	WaitGroup waitGroup(workerCount);
	Scheduler* scheduler = Scheduler::Get();

	for(u32 i = 0; i < workerCount; i++)
	{
		const u32 startIndex = i * itemsPerWorker;
		const u32 endIndex = (i == workerCount - 1) ? items.Size() : (i + 1) * itemsPerWorker;

		auto task = [&items, startIndex, endIndex, &waitGroup]()
		{
			for(u32 j = startIndex; j < endIndex; j++)
			{
				ProcessItem(items[j]);
			}

			waitGroup.NotifyDone();
		};

		scheduler->Post(SchedulerTask(task, "ProcessData"));
	}

	// Wait for all tasks to finish (yields fiber instead of blocking)
	waitGroup.Wait();
}
~~~~~~~~~~~~~

## Producer-consumer with Signal

~~~~~~~~~~~~~{.cpp}
class TaskQueue
{
public:
	void AddTask(const Task& task)
	{
		Lock lock(mMutex);
		mTasks.Add(task);
		mSignal.NotifyOne();
	}

	Task WaitForTask()
	{
		Lock lock(mMutex);
		mSignal.Wait(lock, [this] { return !mTasks.Empty(); });

		Task task = mTasks.Front();
		mTasks.RemoveAt(0);
		return task;
	}

private:
	Mutex mMutex;
	Signal mSignal;
	Vector<Task> mTasks;
};

// Producer task
void ProducerTask(TaskQueue& queue)
{
	for(int i = 0; i < 100; i++)
	{
		queue.AddTask(CreateTask(i));
	}
}

// Consumer task
void ConsumerTask(TaskQueue& queue)
{
	while(true)
	{
		Task task = queue.WaitForTask();
		ProcessTask(task);
	}
}

// Usage
TaskQueue taskQueue;
Scheduler* scheduler = Scheduler::Get();

scheduler->Post(SchedulerTask([&taskQueue]() { ProducerTask(taskQueue); }, "Producer"));
scheduler->Post(SchedulerTask([&taskQueue]() { ConsumerTask(taskQueue); }, "Consumer"));
~~~~~~~~~~~~~

## Scheduler-based parallel work

~~~~~~~~~~~~~{.cpp}
void ProcessLargeDataset(const Vector<DataItem>& items)
{
	Scheduler* scheduler = Scheduler::Get();
	WaitGroup waitGroup;

	// Split work into chunks
	const u32 chunkSize = 1000;
	const u32 chunkCount = (items.Size() + chunkSize - 1) / chunkSize;

	waitGroup.Increment(chunkCount);

	for(u32 i = 0; i < chunkCount; i++)
	{
		const u32 startIndex = i * chunkSize;
		const u32 endIndex = std::min((i + 1) * chunkSize, (u32)items.Size());

		auto task = [&items, startIndex, endIndex, &waitGroup]()
		{
			for(u32 j = startIndex; j < endIndex; j++)
			{
				ProcessItem(items[j]);
			}

			waitGroup.NotifyDone();
		};

		scheduler->Post(SchedulerTask(task, "ProcessChunk"));
	}

	// Wait for all chunks to complete
	waitGroup.Wait();
}
~~~~~~~~~~~~~
