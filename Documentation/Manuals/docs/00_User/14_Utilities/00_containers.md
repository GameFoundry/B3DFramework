Containers 						{#containers}
===============

Containers are data types that contain a set of elements. bs::f uses wrappers for C++ standard library containers like vector, set or map. Aside from different names these containers act exactly like standard library containers.

All available containers:
 - @ref bs::Vector "Vector" - A sequential list of elements. Fast iteration, slow lookup, slow insertion/deletion (except for the last element).
 - @ref bs::List "List" - A sequential list of elements. Slow iteration, slow lookup, fast insertion/deletion for all elements.
 - @ref bs::Stack "Stack" - A collection of elements where elements can only be retrieved and inserted in a specific order. Last element inserted is always the first element retrieved.
 - @ref bs::Queue "Queue" - A collection of elements where elements can only be retrieved and inserted in a specific order. First element inserted is always the first element retrieved.
 - @ref bs::Set "Set" - An ordered list of elements. Fast iteration, fast lookup, mediocre insertion/deletion.
 - @ref bs::Map "Map" - Same as **Set**, only each element is represented as a key-value pair, while **Set** only contains keys.
 - @ref bs::UnorderedSet "UnorderedSet" - Similar to **Set** it allows fast lookup, but elements aren't sorted. In general offers better performance than **Set**.
 - @ref bs::UnorderedMap "UnorderedMap" - Same as **UnorderedSet**, only each element is represented as a key-value pair, while **Set** only contains keys.
 
An example with a vector and an unordered map:
~~~~~~~~~~~~~{.cpp}
int nextUserId = 0;
Vector<String> userNames = { "Sleepy", "Grumpy", "Dopey" };

// Generate IDs for each user and store them in a map for quick lookup.
UnorderedMap<String, int> users;
for(auto& entry : userNames)
	users[entry] = nextUserId++;

// Perform lookup to check if user exists
auto iterFind = users.find("Sneezy");
if(iterFind != users.end())
{
	// User exists
	int userId = iterFind->second;
}
~~~~~~~~~~~~~