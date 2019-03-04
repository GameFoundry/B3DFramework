Compute			{#compute}
===============
[TOC]

Compute GPU programs are not meant to be used for drawing/rendering, but instead of arbitrary computations. In order to execute a compute program you must bind a **ComputePipelineState** to the **RenderAPI**, as previously shown. After it is bound you must call @ref bs::ct::RenderAPI "ct::RenderAPI::dispatch()".

Compute GPU programs are executed in one or multiple thread groups. Each thread group has one or multiple threads, as defined in the GPU program code itself. Thread groups and threads can be organized in one, two or three dimensions, depending on what is most relevant to the data being processed. **ct::RenderAPI::dispatch()** expects the number of thread-groups to launch as parameters.

~~~~~~~~~~~~~{.cpp}
// Execute a GPU program with 32x32 thread-groups
RenderAPI& rapi = RenderAPI::instance();
rapi.dispatch(32, 32, 1);
~~~~~~~~~~~~~