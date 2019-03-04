Command buffers		{#commandBuffers}
===============
[TOC]

Rendering can be a very CPU heavy operation even though GPU does all the rendering - but CPU is still the one submitting all those commands. For this purpose bs::f provides a @ref bs::ct::CommandBuffer "ct::CommandBuffer" object. This object allows you to queue low-level rendering commands on different threads, allowing you to better distribute the CPU usage. Normally rendering commands are only allowed to be submitted from the core thread, but when using command buffers you are allowed to use a different thread for each command buffer.

Almost every method on **RenderAPI** accepts a **CommandBuffer** as its last parameter. If you don't provide one the system will instead use its primary internal command buffer. When you do you can use **RenderAPI** from different threads safely.

> Note: At this point command buffers are only natively supported by the Vulkan render API and will be emulated on others. This means there will be no performance benefit on non-Vulkan render APIs.

# Creation {#commandBuffers_a}
To create a **CommandBuffer** call @ref bs::ct::CommandBuffer::create "ct::CommandBuffer::create()" with @ref bs::GpuQueueType "GpuQueueType" as the parameter. **GpuQueueType** can be:
 - @ref bs::GQT_GRAPHICS "GQT_GRAPHICS" - This is the default command buffer type that supports all type of operations.
 - @ref bs::GQT_COMPUTE "GQT_COMPUTE" - Command buffer type that only supports compute operations.

If you know a command buffer will only execute compute operations it is beneficial to only create it using **GQT_COMPUTE**.

~~~~~~~~~~~~~{.cpp}
SPtr<CommandBuffer> cmds = CommandBuffer::create(GQT_GRAPHICS);
~~~~~~~~~~~~~

# Usage {#commandBuffers_b}
Once created simply provide it to any relevant **RenderAPI** calls as the last parameter.

~~~~~~~~~~~~~{.cpp}
RenderAPI& rapi = RenderAPI::instance();

// ... bind pipeline, vertex/index buffers, etc.
rapi.drawIndexed(0, numIndices, 0, numVertices, 1 cmds);
~~~~~~~~~~~~~

# Submitting {#commandBuffers_c}
Commands queued on a command buffer will only get executed after the command buffer is submitted. Submission is done by calling @ref bs::ct::RenderAPI::submitCommandBuffer "ct::RenderAPI::submitCommandBuffer()".

~~~~~~~~~~~~~{.cpp}
RenderAPI& rapi = RenderAPI::instance();
rapi.submitCommandBuffer(cmds);
~~~~~~~~~~~~~

The default command buffer (one used when you provide no command buffer to **RenderAPI**) is automatically submitted on a call to **RenderAPI::swapBuffers()**.

Note that even though command buffers can be populated with commands on various threads, **ct::RenderAPI::submitCommandBuffer()** must only be called from the core thread. You must also externally synchronize access to **CommandBuffer** when passing it between threads, as it is not thread safe.