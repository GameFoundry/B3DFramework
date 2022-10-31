---
title: Memory allocation
---

When allocating memory in bs::f it is prefered (but not required) to use bs::f allocator functions instead of the standard *new* / *delete* operators or *malloc* / *free*.

- Use @bs::B3DNew instead of *new* and @bs::B3DDelete instead of *delete*.
- Use @bs::B3DNewMultiple instead of *new[]* and @bs::B3DDeleteMultiple instead of *delete[]*.
- Use @bs::B3DAllocate instead of *malloc* and @bs::B3DFree instead of *free*.

This ensures the bs::f can keep track of all allocated memory, which ensures better debugging and profiling, as well as ensuring that internal memory allocation method can be changed in the future.

~~~~~~~~~~~~~{.cpp}
// Helper structure
struct MyStruct 
{ 
	MyStruct() {}
	MyStruct(int a, bool b)
		:a(a), b(b)
	{ }
	
	int a; 
	bool b; 
};

// Allocating memory the normal way
MyStruct* ptr = new MyStruct(123, false);
MyStruct** ptrArray = new MyStruct[5];
void* rawMem = malloc(12);

delete ptr;
delete[] ptrArray;
free(rawMem);

// Allocating memory the bs::f way
MyStruct* bsPtr = B3DNew<MyStruct>(123, false);
MyStruct** bsPtrArray = B3DNewMultiple<MyStruct>(5);
void* bsRawMem = B3DAllocate(12);

B3DDelete(bsPtr);
B3DDeleteMultiple(bsPtrArray, 5);
B3DFree(bsRawMem);
~~~~~~~~~~~~~
