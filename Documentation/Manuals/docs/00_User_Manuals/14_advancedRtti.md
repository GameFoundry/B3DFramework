---
title: Advanced RTTI
---

This manual is a continuation of the [RTTI](User_Manuals/Gameplay/serializingObjects) manual, focusing on some more advanced features of the RTTI system.

# Manually defining fields
Previously we have shown how to define RTTI member fields by using the B3D_RTTI_MEMBER_ macros. While this method is in most cases preferable, it might not be useful if more advanced functionality is required. The macro approach is only able to reference class fields directly, but sometimes we might wish to access data returned by a method, or we might want to process the data in some way (e.g. compressing it).

We can do this by manually defining fields. In order to manually register fields the systems supports a several sets of `add*Field` methods, each expecting a unique name/id, and a pair of getter/setter methods. The getter/setter methods can then be used for any more advanced processing.

## Plain fields
You register plain fields by calling @bs::RTTIType::addPlainField. The getter/setter methods must return/accept a reference to the value of the field.

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

## Reflectable fields
The reflectable field getter/setter signature is similar to the plain field one, only the @bs::RTTIType::addReflectableField method is used for registration instead. 

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

## Reflectable pointer fields
Reflectable pointer getter/setter methods must return shared pointers to the instance, and they're registered with @bs::RTTIType::addReflectablePtrField.
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

## Array fields
Each of the valid field types (plain/reflectable/reflectable pointer), also come in array form. The array form requires two additional getter/setter methods that get/set array size, and normal getter/setter methods require an additional index parameter. 

Methods for registering array fields are:
 - @bs::RTTIType::addPlainArrayField
 - @bs::RTTIType::addReflectableArrayField
 - @bs::RTTIType::addReflectablePtrArrayField
 
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

# Advanced plain fields
Although plain fields are primarily intended for simple built-in types, sometimes they also need to be used on complex types. For example a **std::string** is often used as a field type, but it is not a simple built-in type, nor can we make it derive from **IReflectable**. For these purposes you can use @bs::RTTIPlainType<T>. This is a templated class you can specialize for your specific type. 

Once you specialize the class for your type, implementing all the required methods, you will then be able to use your type on plain fields in the RTTI class. Without this specialization the system will refuse to compile the RTTI type.

The specialization involves implementing methods for serializing/deserializing and retrieving object size. It has no advanced functionality like versioning (so if the structure of the type changes, it will break any previously serialized data), or keeping references to other objects.

For example if we wanted to serialize a simple `struct`:
~~~~~~~~~~~~~{.cpp}
struct MyStruct
{
	int a;
	float b;
	bool c;
};

template<> struct RTTIPlainType<MyStruct>
{	
	enum { id = 20 }; enum { hasDynamicSize = 0 }; // Provide unique ID, and a flag whether the size of the structure is dynamic.

	static BitLength toMemory(const MyStruct& data, Bitstream& stream, const RTTIFieldInfo& info, bool compress)
	{ 
		uint32_t size = 0;

		// Assume we only want to serialize the first two fields of the structure
		size += stream.writeBytes(data.a);
		size += stream.writeBytes(data.b);
		
		return size; 
	}

	static BitLength fromMemory(MyStruct& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
	{
		uint32_t size = 0;
		
		size += stream.readBytes(data.a);
		size += stream.readBytes(data.b);
		
		return size;
	}

	static BitLength getSize(const std::string& data, const RTTIFieldInfo& fieldInfo, bool compress)
	{ 
		return sizeof(data.a) + sizeof(data.b);
	}	
}; 
~~~~~~~~~~~~~

Each specialization must implement all three **toMemory()**, **fromMemory()** and **getSize()** methods. It must also provide an unique **id** that identifies the type, as well as a **hasDynamicSize** flag which determines whether or not the field has dynamic size. Any structure whose size varies with each instance (like a string) must set this flag to true. You must also set it to true if the size is fixed, but larger than 256 bytes.

Both **toMemory()**, **fromMemory()** have a similar signature:
 - **data** - Object that is to be written (**toMemory()**) or object that will receive the results of a read (**fromMemory()**)
 - **stream** - @bs::Bitstream object that is used for writing or reading the serialized data.
 - **info** - Additional optional information about the field being serialized.
 - **compress** - If true you are allowed to provide a more space efficient encoding for your type. This is generally used for serializing for networking where data sizes are important. When false all written/read data sizes must be in multiples of a byte. If true, data sizes are allowed to be sub-byte sized (e.g. a boolean is allowed to be encoded as a single bit).
 - **return** - The total size of the data that was written or read as a @bs::BitLength struct that contains the number of full bytes and any used bits in the last byte.
 
**getSize()** accepts similar parameters, without the **stream** parameter. It should calculate the size of the data and return it as **BitLength**.

You should set **id** to some unique number not used by existing types. Make sure not to clash with built-in type ID's which are all listed as enum values starting with **TID_** (e.g. **TID_Texture = 1001**).

After the specialization is implemented you will be able to use the type in getters/setters for plain fields as you would *int* or *float*. Note that the framework already provides many of such specializations, including ones for strings, vectors and maps.
 
## Dynamic size 
If your structure has dynamic size or is fixed size that is more than 256 bytes you must set the **hasDynamicSize** flag to 1. The size of the structure should be returned from the **getSize()** method. If the size is dynamic you must also encode the size as the first four bytes in a call to **toMemory()**.

For example, you should use **hasDynamicSize** with **String** as each instance will have a different size depending on the string stored. Fields with dynamic size must write the actual size as a header before all encoded data. You can use the helper methods @bs::B3DRTTIWriteWithSizeHeader to write the data with a header, @bs::B3DRTTIReadSizeHeader to read the size from the header and @bs::B3DRTTIAddHeaderSize to calculate the header size.

For example if we wanted to serialize a string:
~~~~~~~~~~~~~{.cpp}
template<> struct RTTIPlainType<String>
{	
	enum { id = 21 }; enum { hasDynamicSize = 1 }; // Provide unique ID, and a flag whether the size of the structure is dynamic.

	static BitLength toMemory(const String& data, Bitstream& stream, const RTTIFieldInfo& info, bool compress)
	{ 
		return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
		{
			stream.writeBytes((uint8_t*)data.data(), data.size());
			return data.size();
		});
	}

	static BitLength fromMemory(String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
	{
		BitLength size;
		BitLength headerSize = B3DRTTIReadSizeHeader(stream, compress, size);

		// 'size' includes the size of the header, so subtract that
		BitLength stringSize = size - headerSize;
		data = String(stream.cursor(), stringSize.bytes);

		return size;
	}

	static BitLength getSize(const String& data, const RTTIFieldInfo& fieldInfo, bool compress)
	{ 
		BitLength dataSize = data.size();

		B3DRTTIAddHeaderSize(dataSize, compress);
		return dataSize;
	}	
}; 
~~~~~~~~~~~~~

## Plain old data types
For very simple structures you can also use the @B3D_ALLOW_MEMCPY_SERIALIZATION macro as a shortcut. It will create a basic **RTTIPlainType<T>** specialization which basically just does a *memcpy()* and uses *sizeof()* to determine the object size.

~~~~~~~~~~~~~{.cpp}
// Simple plain old data type
struct SimpleData
{
	int some;
	float data;
	float here;
};

B3D_ALLOW_MEMCPY_SERIALIZATION(SimpleData)
~~~~~~~~~~~~~

## Helper methods
**RTTIPlainType** specializations can also be used as a more traditional form of serialization in case you find the RTTI system an overkill. For example if you needed to transfer data over a network and don't require advanced versioning features. The system provides helper methods that allow you to easily work with plain types in such a case:
 - @bs::B3DRTTIRead - Deserializes an object from the provided stream, advances the stream cursor and returns number of bytes/bits read
 - @bs::B3DRTTIWrite - Serializes an object into the provided stream, advances the stream cursor and returns number of bytes/bits written
 - @bs::B3DRTTISize - Returns a size an object as **BitLength**
 
~~~~~~~~~~~~~{.cpp}
// Assuming Vector has a RTTIPlainType<T> specialization (which it has, bs::f provides it by default)

Vector<SimpleData> myData;
// fill out myData

// Serialize the entire vector and all of its contents
BitLength size = B3DRTTISize(myData);

Bitstream stream(size.bytes);
B3DRTTIWrite(myData, stream);

// Deserialize the data
stream.seek(0); // Reset cursor to beginning
Vector<SimpleData> myDataCopy;
B3DRTTIRead(myDataCopy, stream);
~~~~~~~~~~~~~

# Querying more RTTI information
You can manually query the class hierarchy and well as class members from the RTTI type object.

**RTTIType** queries:
 - @bs::RTTITypeBase::getBaseClass - Returns the **RTTIType** object of the base class
 - @bs::RTTITypeBase::getDerivedClasses - Returns a list of **RTTIType** objects for all derived classes
 - @bs::RTTITypeBase::getNumFields - Returns the number of member fields
 - @bs::RTTITypeBase::getField - Returns information about a field from its sequential index, in the form of @bs::RTTIField
 - @bs::RTTITypeBase::findField - Searches for a field with a specific name and returns information about it in form of **RTTIField**
 
**RTTIField** queries:
 - @bs::RTTIField::isPlainType - Checks if field contains a plain data type. **RTTIField** can then be cast to @bs::RTTIPlainFieldBase for more operations.
 - @bs::RTTIField::isReflectableType - Checks if field contains an **IReflectable** data type. **RTTIField** can then be cast to @bs::RTTIReflectableFieldBase for more operations.
 - @bs::RTTIField::isReflectablePtrType - Checks if field contains an **IReflectable** pointer data type. **RTTIField** can then be cast to @bs::RTTIReflectablePtrFieldBase for more operations.
 - @bs::RTTIField::isArray - Checks if the field contains an array or a single value
 
~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

B3DRTTIIsOfType<Texture>(myObject);
B3DRTTIIsSubclass<Texture>(myObject);
B3DRTTICreate(TID_Texture);
Texture* myTexture = B3DRTTICast<Texture>(myObject);

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

# Advanced serialization
When implementing **RTTIType<Type, BaseType, MyRTTIType>** can optionally override any of these methods, for additional functionality:
 - @bs::RTTIType::onSerializationStarted
 - @bs::RTTIType::onSerializationEnded
 - @bs::RTTIType::onDeserializationStarted
 - @bs::RTTIType::onDeserializationEnded
 
As their names imply they will get called during serialization/deserialization and allow you to do any pre- or post-processing of the data. Each of those methods accepts an **IReflectable** pointer to the object currently being processed. 

During the calls to those methods, as well as any field getter/setter methods, your instance of the **RTTIType** object is guaranteed to be unique for the instance of the **IReflectable** object it is processing. This means you may use object for temporary data storage during serialization/deserialization.

```
// RTTI for a Texture class that requires special initialization and checking after
// deserialization ends
class B3D_CORE_EXPORT TextureRTTI : public RTTIType<Texture, Resource, TextureRTTI>
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

	void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
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
