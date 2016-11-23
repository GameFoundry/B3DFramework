@page queuesDoc Device queues

Using a non-default queue index allows the GPU to perform operations on different queues in parallel (for example, streaming texture data from the CPU while the GPU is rendering).

When a write operation is being performed it is the responsibility of the caller not to use that resource until that write has completed. This can be ensured by providing a sync mask to commands that accept it (such as RenderAPICore::executeCommands) with the bit corresponding to the queue index set. This way queues will know to wait until write completes. This is not required if the resource is being used on the same queue the write is being performed on, as operations within a queue are sequential.

This value is a global queue index which encodes both the queue type and queue index. Retrieve it from CommandSyncMask::getGlobalQueueIdx().

@page asyncMethod Core thread asynchronous methods
This type of method doesn't execute immediately when called. Instead it is queued for execution and will be executed when the current frame ends. These methods generally need to perform some kind of communication between the main and core (rendering) threads, making direct calls impossible.

You can force the core thread to start executing queued methods before the end of the current frame, by calling @ref bs::CoreThread::submit() "CoreThread::submit()". This guarantees the commands will start executing, but doesn't guarantee they will finish, as it might take the core thread some time to process all queued commands.

If you wish to wait until your command finishes executing then set @p blockUntilComplete parameter of **CoreThread::submit()** to true. 

An example that reads the texture data from the GPU:
~~~~~~~~~~~~~{.cpp}
SPtr<Texture> texture = ... some texture we created earlier ...;
SPtr<PixelData> output = ... buffer to receive the data ...;
texture->readData(output);

// Assuming we need the data right away, we submit the 
// core thread queue and block until the command executes
gCoreThread().submit(true);

// It's now safe to read the pixel data
Color color = output->getColorAt(0, 0);

~~~~~~~~~~~~~

Optionally, if the method returns an @ref bs::AsyncOp "AsyncOp" object, you can call @ref bs::AsyncOp::blockUntilComplete "AsyncOp::blockUntilComplete" for the same result. However you must ensure that you don't call it before **CoreThread::submit()** has been called, otherwise the calling thread might deadlock as it waits for the operations to finish, even though it was never started.

If the method provides a return value, you can use the provided **AsyncOp** object to retrieve it using @ref bs::AsyncOp::getReturnValue<T> "AsyncOp::getReturnValue<T>". Calling this is only valid if @ref bs::AsyncOp::hasCompleted "AsyncOp::hasCompleted" returns true. Return value is always available after you blocked using either of the ways mentioned above.

Note that blocking is a very expensive operation and should not be done in performance critical code. If you need to wait until an async method completes, it is preferable to keep querying **AsyncOp::hasCompleted** and do something else until it returns true.