Advanced memory allocation									{#advMemAlloc}
===============
[TOC]

bs::f allows you to allocate memory in various ways, so you can have fast memory allocations for many situations. We have already shown how to allocate memory for the general case, using **bs_new** / **bs_delete**, **bs_alloc**, **bs_free** and shown how to use shared pointers. But allocating memory using these general purpose allocators can be expensive. Therefore it is beneficial to have more specialized allocator types that have certain restrictions, but allocate memory with almost no overhead.

# Stack allocator {#advMemAlloc_a}
Stack allocator allows you to allocate memory quickly and with zero fragmentation. It comes with a restriction that it can only deallocate memory in the opposite order it was allocated. This usually only makes it suitable for temporary allocations within a single method, where you can guarantee the proper order.

Use @ref bs::bs_stack_alloc "bs_stack_alloc()" / @ref bs::bs_stack_free "bs_stack_free()" or @ref bs::bs_stack_new "bs_stack_new()" / @ref bs::bs_stack_delete "bs_stack_delete()" to allocate/free memory using the stack allocator.

~~~~~~~~~~~~~{.cpp}
UINT8* buffer = bs_stack_alloc(1024);
... do something with buffer ...
UINT8* buffer2 = bs_stack_alloc(512);
... do something with buffer2 ...
bs_stack_free(buffer2); // Must free buffer2 first!
bs_stack_free(buffer);
~~~~~~~~~~~~~

# Frame allocator {#advMemAlloc_b}
Frame allocator segments all allocated memory into *frames*. These frames are stored in a stack-like fashion, and must be deallocated in the opposite order they were allocated, similar to how the stack allocator works. However there are no restrictions on memory deallocation within a single frame, which makes this type of allocator usable in many more situations than the stack allocator. Its downside is that it doesn't deallocate memory until the whole frame is freed - which means it usually uses up more memory than it would otherwise need to.

Use @ref bs::bs_frame_mark "bs_frame_mark()" to start a new frame, and use @ref bs::bs_frame_clear "bs_frame_clear()" to free all of the memory in a single frame. The frames have to be released in opposite order they were created. 

Once you have started a frame use @ref bs::bs_frame_alloc "bs_frame_alloc()" / @ref bs::bs_frame_free "bs_frame_free()" or @ref bs::bs_frame_new "bs_frame_new()" / @ref bs::bs_frame_delete "bs_frame_delete()" to allocate/free memory using the frame allocator. Calls to **bs_frame_free()** / **bs_frame_delete()** are required even through the frame allocator doesn't process individual deallocations, and this is used primarily for debug purposes.

~~~~~~~~~~~~~{.cpp}
// Mark a new frame
bs_frame_mark();
UINT8* buffer = bs_frame_alloc(1024);
... do something with buffer ...
UINT8* buffer2 = bs_frame_alloc(512);
... do something with buffer2 ...
bs_frame_free(buffer); // Only does some checks in debug mode, doesn't actually free anything
bs_frame_free(buffer2); // Only does some checks in debug mode, doesn't actually free anything
bs_frame_clear(); // Frees memory for both buffers
~~~~~~~~~~~~~

## Local frame allocators {#advMemAlloc_b_a}

You can also create your own frame allocators by constructing a @ref bs::FrameAlloc "FrameAlloc" object and calling memory management methods on it directly. While the global frame allocator methods are mostly useful for temporary allocations within a single method, creating your own frame allocator allows you to share frame allocated memory between different objects and persist it for a longer period of time.

For example if you are running some complex algorithm involving multiple classes you might create a frame allocator to be used throughout the algorithm, and then just free all the memory at once when the algorithm finishes.

~~~~~~~~~~~~~{.cpp}
FrameAlloc alloc;
alloc.markFrame();
UINT8* buffer = alloc.alloc(1024);
... do something with buffer ...
UINT8* buffer2 = alloc.alloc(512);
... do something with buffer2 ...
alloc.dealloc(buffer);
alloc.dealloc(buffer2);
alloc.clear();
~~~~~~~~~~~~~

## Container allocators {#advMemAlloc_b_b}

You may also use frame allocator to allocate containers like **String**, **Vector** or **Map**. Simply mark the frame as in the above example, and then use the following container alternatives: @ref bs::String "FrameString", @ref bs::FrameVector "FrameVector" or @ref bs::FrameMap "FrameMap" (most other containers also have a *Frame* version). For example:

~~~~~~~~~~~~~{.cpp}
// Mark a new frame
bs_frame_mark();
{
	FrameVector<UINT8> vector;
	... populate the vector ... // No dynamic memory allocation cost as with a normal Vector
} // Block making sure the vector is deallocated before calling bs_frame_clear
bs_frame_clear(); // Frees memory for the vector
~~~~~~~~~~~~~

# Pool allocator {#advMemAlloc_c}
@ref bs::PoolAlloc<ElemSize, ElemsPerBlock, Alignment> "PoolAlloc<ElemSize, ElemsPerBlock>" is a specialized type of allocator that can be used for permanent allocations. It performs fast allocations with very low fragmentation. Its downside is that it is only able to allocate memory in chunks of specific size, making it suitable for creation of many instances of the same object.

When creating it you need to specify the size of the object you need to allocate, as well as the default number of objects it can contain. If the allocator exceeds the number of objects it can contain it will dynamically allocate another block capable of handling more objects.

~~~~~~~~~~~~~{.cpp}
struct MyData
{
	int foo;
	float bar;
};

// Pool allocator capable of holding 128 objects of type MyData
PoolAlloc<sizeof(MyData), 128> allocator;

// Allocate a single object from the pool
MyData* obj = (MyData*)allocator.alloc();

// ... do something with the data ...

// When done return the memory back to the pool
allocator.free(obj);
~~~~~~~~~~~~~

# Static allocator {#advMemAlloc_d}
@ref bs::StaticAlloc<BlockSize, MaxDynamicMemory> "StaticAlloc<BlockSize, MaxDynamicMemory>" is a specialized type of allocator that can be used for permanent allocations. It works by pre-allocating a user-defined number of bytes. It then tries to use this pre-allocated buffer for any allocations requested from it. As long as the number of allocated bytes doesn't exceed the size of the pre-allocated buffer, allocations are basically free. If you exceed the size of the pre-allocated buffer the allocator will fall back on dynamic allocations.

The downside of this allocator is that the pre-allocated buffer will be using up memory, whether that memory is is actually required or not. Therefore it is important to predict a good static buffer size so that not much memory is wasted, and that most objects don't exceed the static buffer size. This kind of allocator is mostly useful when you have many relatively small objects, each of which requires dynamic allocation of a different size.

~~~~~~~~~~~~~{.cpp}
class MyObj
{
	StaticAlloc<512> mAlloc; // Ensures that every instance of this object has 512 bytes pre-allocated
	UINT8* mData = nullptr;
	
	MyObj(int size)
	{
		// As long as size doesn't go over 512 bytes, no dynamic allocations will be made
		mData = mAlloc.alloc(size);
	}
	
	~MyObj()
	{
		mAlloc.free(mData);
	}
}
~~~~~~~~~~~~~