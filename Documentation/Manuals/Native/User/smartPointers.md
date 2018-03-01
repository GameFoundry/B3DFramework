Smart pointers						{#smartPointers}
===============

Smart pointers allow the user to allocate objects dynamically, but without having to worry about freeing the object. They are prefered to normal memory allocation as they prevent memory leaks when the user might forget to free memory. They are particularily very useful in situations when object ownership is not clearly defined and it might not be clear who is responsible for freeing the object, or when.

# Unique pointers
Unique pointers hold ownership of a dynamically allocated object, and automatically free it when they go out of scope. As their name implies they cannot be copied - in other words, only one pointer to that object can exist. They are mostly useful for temporary allocations, or for places where object ownership is clearly defined to a single owner.

In bs::f type're represented with @ref bs::UPtr "UPtr", which is just a wrapper for the standard library's *std::unique_ptr*. Use @ref bs::bs_unique_ptr_new "bs_unique_ptr_new<T>" to create a unique pointer pointing to a new instance of T, or @ref bs::bs_unique_ptr "bs_unique_ptr" to create one from an existing instance. 

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

UPtr<MyStruct> ptr = bs_unique_ptr_new<MyStruct>(123, false);

// No need to free "ptr", it will automatically be freed when it goes out of scope
~~~~~~~~~~~~~

## Transfering ownership
Since only a single instance of a unique pointer to a specific object may exist, they cannot be copied. However sometimes it is useful to move them to another object (transfer of ownership). In such case you can use *std::move* as shown below:

~~~~~~~~~~~~~{.cpp}
UPtr<MyStruct> ptr = bs_unique_ptr_new<MyStruct>(123, false);

// Transfer ownership
UPtr<MyStruct> ptrOther = std::move(ptr);

// ptrOther now owns the object, while ptr is undefined
~~~~~~~~~~~~~

# Shared pointers
Shared pointers are similar to unique pointers, as they also don't require the object to be explicitly freed after creation. However, unlike unique pointers they can be copied (therefore their name "shared"). This means multiple entities can hold a shared pointer to a single object. Only once ALL such entities lose their shared pointers will the pointed-to object be destroyed.

In bs::f they are represented with @ref bs::SPtr "SPtr", which is just a wrapper for the standard library's *std::shared_ptr*. Use @ref bs::bs_shared_ptr_new "bs_shared_ptr_new<T>" to create a shared pointer pointing to a new instance of T, or @ref bs::bs_shared_ptr "bs_shared_ptr" to create one from an existing instance. 

You will find bs::f uses shared pointers commonly all around its codebase.

~~~~~~~~~~~~~{.cpp}
SPtr<MyStruct> ptr = bs_shared_ptr_new<MyStruct>(123, false);
SPtr<MyStruct> anotherPtr = ptr;

// Object will be freed after both "ptr" and "anotherPtr" go out of scope. 
// Normally you'd want to pass a copy of the pointer to some other system, otherwise we could have used a unique ptr in this particular example
~~~~~~~~~~~~~