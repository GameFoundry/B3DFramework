---
title: Advanced memory allocation
---

The framework allows you to allocate memory in various ways, so you can have fast memory allocations for many situations. We have already shown how to allocate memory for the general case, using **B3DNew** / **B3DDelete**, **B3DAllocate**, **B3DFree** and shown how to use shared pointers. But allocating memory using these general purpose allocators can be expensive. Therefore it is beneficial to have more specialized allocator types that have certain restrictions, but allocate memory with almost no overhead.

# Stack allocator
Stack allocator allows you to allocate memory quickly and with zero fragmentation. It comes with a restriction that it can only deallocate memory in the opposite order it was allocated. This usually only makes it suitable for temporary allocations within a single method, where you can guarantee the proper order.

Use @b3d::B3DStackAllocate / @b3d::B3DStackFree or @b3d::B3DStackNew / @b3d::B3DStackDelete to allocate/free memory using the stack allocator.

~~~~~~~~~~~~~{.cpp}
u8* buffer = B3DStackAllocate(1024);
... do something with buffer ...
u8* buffer2 = B3DStackAllocate(512);
... do something with buffer2 ...
B3DStackFree(buffer2); // Must free buffer2 first!
B3DStackFree(buffer);
~~~~~~~~~~~~~

# Frame allocator
Frame allocator segments all allocated memory into *frames*. These frames are stored in a stack-like fashion, and must be deallocated in the opposite order they were allocated, similar to how the stack allocator works. However there are no restrictions on memory deallocation within a single frame, which makes this type of allocator usable in many more situations than the stack allocator. Its downside is that it doesn't deallocate memory until the whole frame is freed - which means it usually uses up more memory than it would otherwise need to.

Use @b3d::B3DMarkAllocatorFrame to start a new frame, and use @b3d::B3DClearAllocatorFrame to free all of the memory in a single frame. The frames have to be released in opposite order they were created. 

Once you have started a frame use @b3d::B3DFrameAllocate / @b3d::B3DFrameFree or @b3d::B3DFrameNew / @b3d::B3DFrameDelete to allocate/free memory using the frame allocator. Calls to **B3DFrameFree()** / **B3DFrameDelete()** are required even through the frame allocator doesn't process individual deallocations, and this is used primarily for debug purposes.

~~~~~~~~~~~~~{.cpp}
// Mark a new frame
B3DMarkAllocatorFrame();
u8* buffer = B3DFrameAllocate(1024);
... do something with buffer ...
u8* buffer2 = B3DFrameAllocate(512);
... do something with buffer2 ...
B3DFrameFree(buffer); // Only does some checks in debug mode, doesn't actually free anything
B3DFrameFree(buffer2); // Only does some checks in debug mode, doesn't actually free anything
B3DClearAllocatorFrame(); // Frees memory for both buffers
~~~~~~~~~~~~~

## Local frame allocators

You can also create your own frame allocators by constructing a @b3d::FrameAllocator object and calling memory management methods on it directly. While the global frame allocator methods are mostly useful for temporary allocations within a single method, creating your own frame allocator allows you to share frame allocated memory between different objects and persist it for a longer period of time.

For example if you are running some complex algorithm involving multiple classes you might create a frame allocator to be used throughout the algorithm, and then just free all the memory at once when the algorithm finishes.

~~~~~~~~~~~~~{.cpp}
FrameAllocator alloc;
alloc.MarkFrame();
u8* buffer = alloc.Allocate(1024);
... do something with buffer ...
u8* buffer2 = alloc.Allocate(512);
... do something with buffer2 ...
alloc.Free(buffer);
alloc.Free(buffer2);
alloc.Clear();
~~~~~~~~~~~~~

## Container allocators

You may also use frame allocator to allocate containers like **String**, **Vector** or **Map**. Simply mark the frame as in the above example, and then use the following container alternatives: @b3d::String, @b3d::FrameVector or @b3d::FrameMap (most other containers also have a *Frame* version). For example:

~~~~~~~~~~~~~{.cpp}
// Mark a new frame
B3DMarkAllocatorFrame();
{
	FrameVector<u8> vector;
	... populate the vector ... // No dynamic memory allocation cost as with a normal Vector
} // Block making sure the vector is deallocated before calling B3DClearAllocatorFrame
B3DClearAllocatorFrame(); // Frees memory for the vector
~~~~~~~~~~~~~

# Pool allocator
@b3d::PoolAlloc<ElemSize, ElemsPerBlock, Alignment> is a specialized type of allocator that can be used for permanent allocations. It performs fast allocations with very low fragmentation. Its downside is that it is only able to allocate memory in chunks of specific size, making it suitable for creation of many instances of the same object.

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
MyData* obj = (MyData*)allocator.Allocate();

// ... do something with the data ...

// When done return the memory back to the pool
allocator.Free(obj);
~~~~~~~~~~~~~

# Static allocator
@b3d::StaticAlloc<BlockSize, MaxDynamicMemory> is a specialized type of allocator that can be used for permanent allocations. It works by pre-allocating a user-defined number of bytes. It then tries to use this pre-allocated buffer for any allocations requested from it. As long as the number of allocated bytes doesn't exceed the size of the pre-allocated buffer, allocations are basically free. If you exceed the size of the pre-allocated buffer the allocator will fall back on dynamic allocations.

The downside of this allocator is that the pre-allocated buffer will be using up memory, whether that memory is is actually required or not. Therefore it is important to predict a good static buffer size so that not much memory is wasted, and that most objects don't exceed the static buffer size. This kind of allocator is mostly useful when you have many relatively small objects, each of which requires dynamic allocation of a different size.

~~~~~~~~~~~~~{.cpp}
class MyObj
{
	StaticAlloc<512> mAlloc; // Ensures that every instance of this object has 512 bytes pre-allocated
	u8* mData = nullptr;
	
	MyObj(int size)
	{
		// As long as size doesn't go over 512 bytes, no dynamic allocations will be made
		mData = mAlloc.Allocate(size);
	}
	
	~MyObj()
	{
		mAlloc.Free(mData);
	}
}
~~~~~~~~~~~~~
