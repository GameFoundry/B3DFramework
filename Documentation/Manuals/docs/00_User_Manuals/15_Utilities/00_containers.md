---
title: Containers
---
Containers are data types that contain a set of elements. The framework provides both custom container implementations and type aliases for C++ standard library containers.

# Standard library container aliases

The following containers are aliases for C++ standard library containers. They behave exactly like their standard library counterparts:

 - @b3d::Vector - Alias for `std::vector`. A sequential list of elements. Fast iteration, slow lookup, slow insertion/deletion (except for the last element).
 - @b3d::List - Alias for `std::list`. A sequential list of elements. Slow iteration, slow lookup, fast insertion/deletion for all elements.
 - @b3d::Stack - Alias for `std::stack`. A collection of elements where elements can only be retrieved and inserted in a specific order. Last element inserted is always the first element retrieved.
 - @b3d::Queue - Alias for `std::queue`. A collection of elements where elements can only be retrieved and inserted in a specific order. First element inserted is always the first element retrieved.
 - @b3d::Set - Alias for `std::set`. An ordered list of elements. Fast iteration, fast lookup, mediocre insertion/deletion.
 - @b3d::Map - Alias for `std::map`. Same as **Set**, only each element is represented as a key-value pair, while **Set** only contains keys.
 - @b3d::UnorderedSet - Alias for `std::unordered_set`. Similar to **Set** it allows fast lookup, but elements aren't sorted. In general offers better performance than **Set**.
 - @b3d::UnorderedMap - Alias for `std::unordered_map`. Same as **UnorderedSet**, only each element is represented as a key-value pair, while **Set** only contains keys.

An example with a vector and an unordered map:
~~~~~~~~~~~~~{.cpp}
int nextUserId = 0;
Vector<String> userNames = { "Sleepy", "Grumpy", "Dopey" };

// Generate IDs for each user and store them in a map for quick lookup
UnorderedMap<String, int> users;
for(auto& entry : userNames)
	users[entry] = nextUserId++;

// Perform lookup to check if user exists
auto iteratorFind = users.find("Sneezy");
if(iteratorFind != users.end())
{
	// User exists
	int userId = iteratorFind->second;
}
~~~~~~~~~~~~~

# Custom framework containers

The following containers are custom implementations provided by the framework:

 - @b3d::TArray - Dynamically sized array, similar to **Vector** but without relying on the standard library.
 - @b3d::TInlineArray - A dynamically sized container that statically allocates enough room for N elements. If the element count exceeds the statically allocated buffer size the array falls back to general purpose dynamic allocator.
 - @b3d::TQueue - Lock-free queue implemented as a linked list. Supports multiple producer/single consumer, single producer/single consumer, and thread-unsafe modes.
 - @b3d::TDenseMap - Hash-map with densely stored values, using quadratic probing for lookup.
 - @b3d::TBitfield - Dynamically sized field that contains a sequential list of bits. The bits are compactly stored and allow for quick sequential searches.
 - @b3d::TChunkedArray - Dynamically-growing array that stores elements in fixed-size pages (chunks). Growth only allocates a new page — existing data is never copied or moved, making it ideal for large arrays that grow incrementally.

# TArray
@b3d::TArray is a dynamically sized array similar to `std::vector`, but implemented without relying on the standard library. It provides efficient random access and supports custom allocators.

~~~~~~~~~~~~~{.cpp}
// Create an empty array
TArray<int> numbers;

// Create an array with initial size
TArray<String> names(5, "Default");

// Create from initializer list
TArray<float> values = { 1.0f, 2.0f, 3.0f, 4.0f };

// Add elements
numbers.Add(42);
numbers.Add(100);
numbers.Add(7);

// Access elements by index
int firstNumber = numbers[0]; // 42
int lastNumber = numbers.Back(); // 7

// Insert elements at specific position
numbers.Insert(numbers.Begin() + 1, 50); // Insert 50 at index 1

// Remove elements
numbers.Remove(2); // Remove element at index 2
numbers.Pop(); // Remove last element

// Append multiple elements
TArray<int> moreNumbers = { 10, 20, 30 };
numbers.Append(moreNumbers.Begin(), moreNumbers.End());

// Resize the array
numbers.Resize(10, 0); // Resize to 10 elements, new elements initialized to 0

// Reserve capacity without adding elements
numbers.Reserve(100);

// Get array properties
u64 size = numbers.Size();
u64 capacity = numbers.Capacity();
bool isEmpty = numbers.Empty();

// Access raw data pointer
int* data = numbers.Data();

// Iterate over elements
for(auto& number : numbers)
{
	// Process each number
}

// Clear the array
numbers.Clear();
~~~~~~~~~~~~~

# TInlineArray
@b3d::TInlineArray is a specialized version of TArray that statically allocates space for N elements inline. This avoids heap allocation for small arrays, improving performance. If the array grows beyond N elements, it automatically falls back to heap allocation.

~~~~~~~~~~~~~{.cpp}
// Create an inline array that can hold up to 16 elements without heap allocation
TInlineArray<int, 16> smallNumbers;

// Add elements - these will be stored inline (no heap allocation)
for(int i = 0; i < 10; i++)
	smallNumbers.Add(i);

// The array behaves exactly like TArray
smallNumbers[5] = 100;
int value = smallNumbers.Front();

// If we exceed 16 elements, the array automatically allocates on the heap
for(int i = 10; i < 50; i++)
	smallNumbers.Add(i);

// All TArray operations are available
smallNumbers.Remove(5);
u64 size = smallNumbers.Size();

// Common use case: avoiding allocations for temporary collections
TInlineArray<Vector3, 8> vertices;
vertices.Add(Vector3(0.0f, 0.0f, 0.0f));
vertices.Add(Vector3(1.0f, 0.0f, 0.0f));
vertices.Add(Vector3(1.0f, 1.0f, 0.0f));
~~~~~~~~~~~~~

# TDenseMap
@b3d::TDenseMap is a hash-map implementation that stores values densely using quadratic probing for collision resolution. Unlike **UnorderedMap** (which uses separate chaining with linked lists), **TDenseMap** stores all key-value pairs in a contiguous array. This provides better cache locality and faster iteration, making it advantageous when you need to frequently iterate over all elements or when memory layout is important for performance. The trade-off is that keys must have special empty and tombstone values defined via DenseMapInfo specialization.

~~~~~~~~~~~~~{.cpp}
// Create a dense map (default size is 64 buckets)
TDenseMap<u32, String> userIdToName;

// Add key-value pairs using operator[]
userIdToName[1] = "Alice";
userIdToName[2] = "Bob";
userIdToName[42] = "Charlie";

// Insert using construct method
auto& pair = userIdToName.construct(100);
pair.second = "David";

// Access values
String name = userIdToName[42]; // "Charlie"

// Check if key exists
auto iterator = userIdToName.find(2);
if(iterator != userIdToName.end())
{
	String foundName = iterator->second; // "Bob"
}

// Get the number of entries
u32 size = userIdToName.size();

// Check if empty
bool isEmpty = userIdToName.empty();

// Iterate over all entries
for(auto& pair : userIdToName)
{
	u32 userId = pair.first;
	String userName = pair.second;
}

// Erase a key
userIdToName.erase(1);

// Clear all entries
userIdToName.clear();

// Create with custom initial capacity
TDenseMap<u32, float, DenseMapInfo<u32>, 128> largeMap;
~~~~~~~~~~~~~

For pointer types and unsigned integers, DenseMapInfo is automatically specialized. For custom key types, you need to provide a DenseMapInfo specialization:

~~~~~~~~~~~~~{.cpp}
// Example: Using TDenseMap with custom key type
struct MyKey
{
	u32 value;

	bool operator==(const MyKey& other) const { return value == other.value; }
};

// Specialize DenseMapInfo for MyKey
template<>
struct DenseMapInfo<MyKey>
{
	static MyKey getEmptyKey() { return MyKey{ 0xFFFFFFFF }; }
	static MyKey getTombstoneKey() { return MyKey{ 0xFFFFFFFE }; }
};

// Also need to specialize B3DHash for MyKey
template<>
struct B3DHash<MyKey>
{
	size_t operator()(const MyKey& key) const
	{
		return B3DHash<u32>()(key.value);
	}
};

// Now you can use TDenseMap with MyKey
TDenseMap<MyKey, String> myMap;
myMap[MyKey{10}] = "Ten";
~~~~~~~~~~~~~

# TQueue
@b3d::TQueue is a lock-free queue that supports different threading policies for safe concurrent access. The queue is implemented as a linked list and provides efficient FIFO (first-in-first-out) operations.

Threading policies are specified via @b3d::QueueThreadingPolicy:
 - @b3d::QueueThreadingPolicy::MPSC - Multiple producer threads can write, one consumer thread can read safely.
 - @b3d::QueueThreadingPolicy::SPSC - Single producer thread can write, one consumer thread can read safely.
 - @b3d::QueueThreadingPolicy::SingleThread - No threading safety, should only be used by a single thread.

~~~~~~~~~~~~~{.cpp}
// Create a thread-safe queue for multiple producers and single consumer
TQueue<int, QueueThreadingPolicy::MPSC> queue;

// Add elements to the queue
queue.Enqueue(42);
queue.Enqueue(100);
queue.Enqueue(7);

// Remove and retrieve an element from the front
TOptional<int> element = queue.Dequeue();
if(element.IsSet())
{
	int value = element.GetValue(); // value = 42
}

// Alternative dequeue method
int outElement;
if(queue.Dequeue(outElement))
{
	// outElement = 100
}

// Peek at the front element without removing it
int* frontElement = queue.Peek();
if(frontElement != nullptr)
{
	// *frontElement = 7
}

// Check if queue is empty
bool isEmpty = queue.IsEmpty();
~~~~~~~~~~~~~

# TBitfield
@b3d::TBitfield is a dynamically sized container that stores individual bits in a compact form. It provides efficient storage and operations for boolean values, using only 1 bit per element instead of a full byte.

~~~~~~~~~~~~~{.cpp}
// Create a bitfield with 100 bits, all initialized to false
TBitfield<> bitfield(false, 100);

// Set individual bits
bitfield[0] = true;
bitfield[5] = true;
bitfield[42] = true;

// Read individual bits
bool isSet = bitfield[5]; // true

// Add a new bit to the end
u64 newIndex = bitfield.Add(true);

// Find the first bit with a specific value
u64 firstTrueBit = bitfield.Find(true); // Returns 0
u64 firstFalseBit = bitfield.Find(false); // Returns 1

// Count how many bits have a specific value
u64 numTrueBits = bitfield.Count(true);
u64 numFalseBits = bitfield.Count(false);

// Resize the bitfield
bitfield.Resize(200, false); // Resize to 200 bits, new bits set to false

// Reserve capacity without adding elements
bitfield.Reserve(500);

// Reset all bits to a specific value
bitfield.Reset(false); // Set all bits to false

// Remove a bit at a specific index
bitfield.Remove(5);

// Get the number of bits
u64 size = bitfield.Size();

// Get the allocated capacity in bits
u64 capacity = bitfield.Capacity();

// Iterate over all bits
for(auto bit : bitfield)
{
	if(bit)
	{
		// Bit is set to true
	}
}

// Clear the bitfield
bitfield.Clear(); // Removes all bits but keeps allocated memory
bitfield.Clear(true); // Removes all bits and frees memory
~~~~~~~~~~~~~

# TChunkedArray
@b3d::TChunkedArray is a dynamically-growing array that stores elements in fixed-size pages (chunks) rather than a single contiguous buffer. When the array grows, only a new page is allocated — existing data is never copied or moved. This makes it ideal for large arrays that grow incrementally, where avoiding reallocation overhead is important.

The `PageSize` template parameter (default 1024) controls how many elements fit in each page. It must be a power of two.

~~~~~~~~~~~~~{.cpp}
// Create a chunked array with default page size (1024 elements per page)
TChunkedArray<int> numbers;

// Create with custom page size (must be power of two)
TChunkedArray<Vector3, 256> positions;

// Add elements — never triggers reallocation of existing data
numbers.Add(42);
numbers.Add(100);

// Emplace elements in-place
numbers.EmplaceBack(7);

// Random access by index
int firstNumber = numbers[0]; // 42

// Remove the last element
numbers.Pop();

// Resize the array
numbers.Resize(50); // Grow to 50 elements (default-constructed)
numbers.Resize(100, 0); // Grow to 100 elements, new elements initialized to 0

// Get array properties
u64 size = numbers.Size();
bool isEmpty = numbers.Empty();

// Iterate over all elements (random-access iterators supported)
for(auto& number : numbers)
{
	// Process each number
}

// Clear the array
numbers.Clear();
~~~~~~~~~~~~~
