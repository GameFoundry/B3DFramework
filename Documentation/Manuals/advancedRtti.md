Advanced RTTI				{#advancedRtti}
===============
[TOC]

This manual is a continuation of the @ref serializingObjects manual, focusing on some more advanced features of the RTTI system.

# Manually defining fields {#advancedRtti_a}
Previously we have shown how to define RTTI member fields by using the BS_RTTI_MEMBER_* macros. While this method is in most cases preferable, it might not be useful if more advanced functionality is required. The macro approach is only able to reference class fields directly, but sometimes we might wish to access data returned by a method, or we might want to process the data in some way (e.g. compressing it).

We can do this by manually defining fields. In order to manually register fields the systems supports a several sets of `add*Field` methods, each expecting a unique name/id, and a pair of getter/setter methods. The getter/setter methods can then be used for any more advanced processing.

## Plain fields {#advancedRtti_a_a}
You register plain fields by calling @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addPlainField "RTTIType::addPlainField()". The getter/setter methods must return/accept a reference to the value of the field.

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	UINT32 myInt;

	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	UINT32& getMyInt(MyComponent* obj) { return obj->myInt; }
	void setMyInt(MyComponent* obj, UINT32& value) { obj->myInt = value; }

	MyComponentRTTI ()
	{
		// Register the getter/setter methods above. You must specify a name and field id (both must be unique within the type).
		addPlainField("myInt", 0, &MyComponentRTTI::getMyInt, &MyComponentRTTI::setMyInt);
	}
	
	// ... 
};
~~~~~~~~~~~~~

## Reflectable fields {#advancedRtti_a_b}
The reflectable field getter/setter signature is similar to the plain field one, only the @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectableField "RTTIType::addReflectableField()" method is used for registration instead. 

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	HMesh mesh;

	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	HMesh& getMesh(MyComponent* obj) { return obj->mesh; }
	void setMesh(MyComponent* obj, HMesh& value) { obj->mesh = value; }

	MyComponentRTTI ()
	{
		addReflectableField("mesh", 0, &MyComponentRTTI::getMesh, &MyComponentRTTI::setMesh);
	}
	
	// ... 
};
~~~~~~~~~~~~~

## Reflectable pointer fields {#advancedRtti_a_c}
Reflectable pointer getter/setter methods must return shared pointers to the instance, and they're registered with @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectablePtrField "RTTIType::addReflectablePtrField()".
~~~~~~~~~~~~~{.cpp}
class MyClass : public IReflectable
{
	// ...
}

class MyComponent : public Component
{
public:
	// ...

	SPtr<MyClass> myClass;

	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	SPtr<MyClass> getMyClass(MyComponent* obj) { return obj->myClass; }
	void setMyClass(MyComponent* obj, SPtr<MyClass> value) { obj->myClass = value; }

	MyComponentRTTI ()
	{
		addReflectablePtrField("myClass", 0, &MyComponentRTTI::getMyClass, &MyComponentRTTI::setMyClass);
	}
	
	// ... 
};
~~~~~~~~~~~~~

## Array fields {#advancedRtti_a_d}
Each of the valid field types (plain/reflectable/reflectable pointer), also come in array form. The array form requires two additional getter/setter methods that get/set array size, and normal getter/setter methods require an additional index parameter. 

Methods for registering array fields are:
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addPlainArrayField "RTTIType::addPlainArrayField()"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectableArrayField "RTTIType::addReflectableArrayField()"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectablePtrArrayField "RTTIType::addReflectablePtrArrayField()"
 
~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	Vector<HMesh> meshes;

	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	HMesh& getMesh(MyComponent* obj, UINT32 idx) { return obj->meshes[idx]; }
	void setMesh(MyComponent* obj, HMesh& value, UINT32 idx) { obj->meshes[idx] = value; }

	UINT32 getNumMeshes(MyComponent* obj) { return obj->meshes.size(); }
	void setNumMeshes(MyComponent* obj, UINT32 size) { obj->meshes.resize(size); }
	
	MyComponentRTTI ()
	{
		addReflectablePtrArrayField("meshes", 0, &MyComponentRTTI::getMesh, &MyComponentRTTI::getNumMeshes, 
			&MyComponentRTTI::setMesh, &MyComponentRTTI::setNumMeshes);
		
		// And similarly for other field types
	}
	
	// ... 
};
~~~~~~~~~~~~~

> By using this form of field definitions you are also no longer limited to arrays only contained with the **Vector** container.

# Advanced plain fields {#advancedRtti_b}
Although plain fields are primarily intended for simple built-in types, sometimes they also need to be used on complex types. For example a **std::string** is often used as a field type, but it is not a simple built-in type, nor can we make it derive from **IReflectable**. For these purposes you can use @ref bs::RTTIPlainType<T> "RTTIPlainType<T>". This is a templated class you can specialize for your specific type. 

Once you specialize the class for your type, implementing all the required methods, you will then be able to use your type on plain fields in the RTTI class. Without this specialization the system will refuse to compile the RTTI type.

The specialization involves implementing methods for serializing/deserializing and retrieving object size. It has no advanced functionality like versioning (so if the structure of the type changes, it will break any previously serialized data), or keeping references to other objects.

For example if we wanted to serialize a string:
~~~~~~~~~~~~~{.cpp}
template<> struct RTTIPlainType<std::string>
{	
	enum { id = 20 }; enum { hasDynamicSize = 1 }; // Provide unique ID, and a flag whether the size of the structure is dynamic.

	static void toMemory(const std::string& data, char* memory)
	{ 
		UINT32 size = getDynamicSize(data);

		memcpy(memory, &size, sizeof(UINT32));
		memory += sizeof(UINT32);
		size -= sizeof(UINT32);
		memcpy(memory, data.data(), size); 
	}

	static UINT32 fromMemory(std::string& data, char* memory)
	{ 
		UINT32 size;
		memcpy(&size, memory, sizeof(UINT32)); 
		memory += sizeof(UINT32);

		UINT32 stringSize = size - sizeof(UINT32);
		data = std::string(memory, stringSize);

		return size;
	}

	static UINT32 getDynamicSize(const std::string& data)	
	{ 
		UINT64 dataSize = data.size() * sizeof(std::string::value_type) + sizeof(UINT32);
		return (UINT32)dataSize;
	}	
}; 
~~~~~~~~~~~~~

> Note: bs::f already provides many of such specializations, including ones for strings, vectors and maps.

Each specialization must implement all three **toMemory()**, **fromMemory()** and **getDynamicSize()** methods. It must also provide a flag **hasDynamicSize** which determines whether or not it has dynamic size. Any structure whose size varies with each instance (like a string) must set this flag to true. You must also set it to true if the size is static but larger than 255 bytes.

After the specialization is implemented you will be able to use the type in getters/setters for plain fields as you would *int* or *float*. 

For very simple classes you can also use the @ref BS_ALLOW_MEMCPY_SERIALIZATION macro instead. It will create a basic **RTTIPlainType<T>** specialization which uses *memcpy()* and *sizeof()* to implement the necessary methods.

~~~~~~~~~~~~~{.cpp}
// Simple plain old data type
struct SimpleData
{
	int some;
	float data;
	float here;
};

BS_ALLOW_MEMCPY_SERIALIZATION(SimpleData)
~~~~~~~~~~~~~

**RTTIPlainType** specializations can also be used as a more traditional form of serialization in case you find the RTTI system an overkill. For example if you needed to transfer data over a network and don't require advanced versioning features. The system provides helper methods that allow you to easily work with plain types in such a case:
 - @ref bs::rttiReadElem "rttiReadElem()" - Deserializes an object from the provided buffer and returns offset into the buffer after the read data
 - @ref bs::rttiWriteElem "rttiWriteElem()" - Serializes an object into the provided buffer and returns offset into the buffer after the written data
 - @ref bs::rttiGetElemSize "rttiGetElemSize()" - Returns a size an object
 
~~~~~~~~~~~~~{.cpp}
// Assuming Vector has a RTTIPlainType<T> specialization (which it has, bs::f provides it by default)

Vector<SimpleData> myData;
// fill out myData

// Simply serialize the entire vector and all of its contents
UINT32 size = rttiGetElemSize(myData);
UINT8* data = (UINT8*)bs_alloc(size);
rttiWriteElem(myData, data);

// Simply deserialize the data
Vector<SimpleData> myDataCopy;
rttiReadElem(myDataCopy, data);

bs_free(data);
~~~~~~~~~~~~~

# Querying RTTI {#advancedRtti_c}
Aside from using RTTI for serialization, you can also use it to manually query various information about objects. 

Global queries:
 - @ref bs::rtti_is_of_type "rtti_is_of_type<T>()" - Checks is a specific object of type *T*
 - @ref bs::rtti_is_subclass "rtti_is_subclass<T>()" - Checks is a specific object derived from type *T*
 - @ref bs::rtti_create "rtti_create()" - Creates a new object from its type ID
 
**IReflectable** queries:
 - @ref bs::IReflectable::getTypeName "IReflectable::getTypeName()" - Gets the name of the object's type
 - @ref bs::IReflectable::getTypeId "IReflectable::getTypeId()" - Gets the type ID of the object's type
 
**RTTIType** queries:
 - @ref bs::RTTITypeBase::getBaseClass "RTTITypeBase::getBaseClass()" - Returns the **RTTIType** object of the base class
 - @ref bs::RTTITypeBase::getDerivedClasses "RTTITypeBase::getDerivedClasses()" - Returns a list of **RTTIType**%s for all derived classes
 - @ref bs::RTTITypeBase::getNumFields "RTTITypeBase::getNumFields()" - Returns the number of member fields
 - @ref bs::RTTITypeBase::getField "RTTITypeBase::getField()" - Returns information about a field from its sequential index, in the form of @ref bs::RTTIField "RTTIField"
 - @ref bs::RTTITypeBase::findField "RTTITypeBase::findField()" - Searches for a field with a specific name and returns information about it in form of **RTTIField**
 
**RTTIField** queries:
 - @ref bs::RTTIField::isPlainType "RTTIField::isPlainType()" - Checks if field contains a plain data type. **RTTIField** can then be cast to @ref bs::RTTIPlainFieldBase "RTTIPlainFieldBase" for more operations.
 - @ref bs::RTTIField::isReflectableType "RTTIField::isReflectableType()" - Checks if field contains an **IReflectable** data type. **RTTIField** can then be cast to @ref bs::RTTIReflectableFieldBase "RTTIReflectableFieldBase" for more operations.
 - @ref bs::RTTIField::isReflectablePtrType "RTTIField::isReflectablePtrType()" - Checks if field contains an **IReflectable** pointer data type. **RTTIField** can then be cast to @ref bs::RTTIReflectablePtrFieldBase "RTTIReflectablePtrFieldBase" for more operations.
 - @ref bs::RTTIField::isArray "RTTIField::isArray()" - Checks if the field contains an array or a single value
 
~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

rtti_is_of_type<Texture>(myObject);
rtti_is_subclass<Texture>(myObject);
rtti_create(TID_Texture);

myObject->getTypeName();
myObject->getTypeId();

RTTITypeBase* rttiType = myObject->getRTTI();
rttiType->getBaseClass();
rttiType->getDerivedClasses();
rttiType->getNumFields();
rttiType->getField(index);

RTTIField* rttiField = rttiType->findField(name);
rttiField->isPlainType();
rttiField->isReflectableType();
rttiField->isReflectablePtrType();
rttiField->isArray();
~~~~~~~~~~~~~

Once you have an instance of a **RTTIField** object, you can also use it to directly read and write values on object instances for that particular field.

# Advanced serialization  {#advancedRtti_d}
When implementing **RTTIType<Type, BaseType, MyRTTIType>** can optionally override any of these methods, for additional functionality:
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onSerializationStarted "RTTIType::onSerializationStarted"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onSerializationEnded "RTTIType::onSerializationEnded"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onDeserializationStarted "RTTIType::onDeserializationStarted"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onDeserializationEnded "RTTIType::onDeserializationEnded"
 
As their names imply they will get called during serialization/deserialization and allow you to do any pre- or post-processing of the data. Each of those methods accepts an **IReflectable** pointer to the object currently being processed. 

During the calls to those methods, as well as any field getter/setter methods, your instance of the **RTTIType** object is guaranteed to be unique for the instance of the **IReflectable** object it is processing. This means you may use object for temporary data storage during serialization/deserialization.

```
// RTTI for a Texture class that requires special initialization and checking after
// deserialization ends
class BS_CORE_EXPORT TextureRTTI : public RTTIType<Texture, Resource, TextureRTTI>
{
private:

	// The setter doesn't write to Texture directly, instead it just stores the
	// data in temporary field
	void setPixelData(Texture* obj, SPtr<PixelData> data) { mPixelData = data; }
	
	SPtr<PixelData> getPixelData(Texture* obj) { /* Not relevant */ return ...; };	

public:
	TextureRTTI()
	{
		addReflectablePtrField("mPixelData", 0, 
			&TextureRTTI::getPixelData, &TextureRTTI::setPixelData);
	}

	void onDeserializationEnded(IReflectable* obj, 
		const UnorderedMap<String, UINT64>& params) override
	{
		Texture* texture = static_cast<Texture*>(obj);
		
		// Initialize the texture now that all fields have been deserialized
		texture->initialize();
		
		// Check if the pixel data matches our texture and write the data from
		// our temporary field
		
		// Do the checks
		...
		
		texture->writeData(mPixelData);
	}

	// RTTIType boilerplate (name, id, newRTTIObject)
	...
	
private:
	// Temporary storage to be used during deserialization
	SPtr<PixelData> mPixelData;
};
```