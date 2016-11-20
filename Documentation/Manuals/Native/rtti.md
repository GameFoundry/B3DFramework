Run-time type information						{#rtti}
===============
[TOC]

This manual describes how is run-time type information (RTTI) used to provide meta-information about C++ classes during program execution. In Banshee specifically it allows you to:
 - Serialize/deserialize objects with no additional code 
 - Get the name of a class, and exact polymorphic type of an object (including safe casting)
 - Get the hierarchy of base/derived classes of a specific type
 - Iterate over all fields in an object, find their name and type, get/set their value
 - Create new instances of a type with only its type-name
 - Generate "diffs" of objects
 
It is primarily used for saving/loading of objects (e.g. serialization/deserialization of a level), searching for objects (e.g. finding all resources used by a level) and generating "diffs" (Prefabs use diffs to persist/apply instance specific changes when the original changes).

RTTI doesn't automatically work on all classes. You must manually specify the information required by the RTTI system. This involves two things:
 - Making sure your class derives from @ref bs::IReflectable "IReflectable" and implements the required methods
 - Implement the @ref bs::RTTIType<Type, BaseType, MyRTTIType> "RTTIType" interface
 
# Adding RTTI to your own objects {#rtti_a}
Assume you have a simple object you want to serialize:
~~~~~~~~~~~~~{.cpp}
class Texture
{
	int width, height;
};
~~~~~~~~~~~~~

First off ensure it implements @ref bs::IReflectable "IReflectable" and `getRTTIStatic` and `getRTTI` methods:
~~~~~~~~~~~~~{.cpp}
class Texture : public IReflectable
{
   int width, height;

   static RTTITypeBase* getRTTIStatic()
   { return TextureRTTI::instance(); }

   RTTITypeBase* getRTTI() const override
   { return Texture::getRTTIStatic(); }
};
~~~~~~~~~~~~~
	
This interface and its methods only serve to return an instance of `TextureRTTI`, which derives from @ref bs::RTTIType<Type, BaseType, MyRTTIType> "RTTIType" and contains all the necessary RTTI information. You will also usually want to make the RTTI type a `friend` of the type so it can easily access its private and protected fields.

A basic implementation of @ref bs::RTTIType<Type, BaseType, MyRTTIType> "RTTIType" must provide information about the type name, unique ID of the type, its base class (@ref bs::IReflectable "IReflectable" if it doesn't derive from anything else), a way to create a new instance of the type and a list of fields with their getter/setter methods. A simple one for our example `Texture` class might look like so:
~~~~~~~~~~~~~{.cpp}
class TextureRTTI : public RTTIType<Texture, IReflectable, TextureRTTI> // Specify type the RTTI is for, its base type, and RTTI type itself
{
	// Specify getter/setter methods for each field we want to be able to access from RTTI.
	// Each getter/setter method always accepts a pointer to the instance being queried as the first parameter.
	int& getWidth(Texture* obj) { return obj->width; }
	void setWidth(Texture* obj, int& value) { obj->width = value; }
	
	int& getHeight(Texture* obj) { return obj->height; }
	void setHeight(Texture* obj, int& value) { obj->height = value; }

	TextureRTTI ()
	{
		// Register the getter/setter methods above. You must specify a name and field id (both must be unique within the type).
		addPlainField("width", 0, &TextureRTTI::getWidth, &TextureRTTI::setWidth);
		addPlainField("height", 1, &TextureRTTI::getHeight, &TextureRTTI::setHeight);
	}

	const String& getRTTIName() override
	{
		static String name = "Texture"; // Returns human readable name of the type
		return name;
	}

	UINT32 getRTTIId() override
	{
		return TID_Texture; // User-provided globally unique integer for the type
	}

	SPtr<IReflectable> newRTTIObject() override
	{
		return bs_shared_ptr_new<Texture>(); // Create a brand new empty instance
	}
};
~~~~~~~~~~~~~

This is the minimum you need to do to add a RTTI type. However there are some advanced features that you might be interested in. You may specify different types of fields that can handle references to other complex objects, as well as provide arrays, or supply a pre- and post- processing steps for serialization. We'll cover this in the next sections.

Important thing to note about RTTI type fields is that they all require a unique ID. This ID ensures that serialized data isn't broken if the layout of the object changes during development. This means you can add new fields, or remove old ones and expect data saved using an older version to "just work". You do need to ensure never to assign an ID of a previously removed field to a new field. You also need to change the ID if the field's type changes (i.e. it used to be `int` but now it's `long`).

# Advanced field types  {#rtti_b}

When registering fields with the RTTI type, the systems supports a several sets of `add*Field` methods, each expecting a unique name/id, but different getter/setter methods.

## Plain fields  {#rtti_b_a}
In the example above we have shown how to provide getter/setter methods for fields of `int` type. These fields are considered "plain" fields by the engine, and fields for types like `float`, `bool` and any other built-in language type also falls into this category.

You register plain fields by calling @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addPlainField "RTTIType::addPlainField". The getter/setter methods must return/accept a reference to the value of the field.

## Reflectable fields  {#rtti_b_b}
Reflectable fields contain types deriving from @ref bs::IReflectable "IReflectable", meaning they're complex objects that contain their own RTTI types. For example if we were to add a `Material` class to our example, it might contain a texture. In such case we would provide getter/setter methods like so:
~~~~~~~~~~~~~{.cpp}
class Material : public IReflectable
{ 
	Texture texture;
	
	... 
};

class MaterialRTTI : public RTTIType<Material, IReflectable, MaterialRTTI>
{
	Texture& getTexture(Material* obj) { return obj->texture; }
	void setTexture(Material* obj, Texture& value) { obj->texture = value; }
	
	MaterialRTTI ()
	{
		addReflectableField("texture", 0, &MaterialRTTI::getTexture, &MaterialRTTI::setTexture);
	}
	
	...
};
~~~~~~~~~~~~~

As you can see the reflectable field is similar to a plain field, only the @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectableField "RTTIType::addReflectableField" method is used instead. 

The problem with this approach is that during serialization whenever a material references a texture, that entire texture will end up being serialized with it. This is something you normally want to avoid since multiple materials will usually be referencing the same texture. For that purpose "Reflectable pointer" fields exist.

## Reflectable pointer fields  {#rtti_b_c}
This type of field is similar to the reflectable field, as it also contains types deriving from @ref bs::IReflectable "IReflectable". However they only return a pointer to the owned object, instead of copying the object by value. This is relevant for serialization as the system will be smart enough to detect multiple fields pointing to the same instance of an @ref bs::IReflectable "IReflectable" object, and only serialize it once (instead of every time it is encountered). When deserializing the system will also properly restore the pointers, so that all fields keep pointing to the same instance.

Reflectable pointer getter/setter methods must return shared pointers to the instance, and they're registered with @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectablePtrField "RTTIType::addReflectablePtrField". For example if we modified our `Material` class like so:
~~~~~~~~~~~~~{.cpp}
class Material : public IReflectable
{ 
	SPtr<Texture> texture;
	
	... 
};

class MaterialRTTI : public RTTIType<Material, IReflectable, MaterialRTTI>
{
	SPtr<Texture> getTexture(Material* obj) { return obj->texture; }
	void setTexture(Material* obj, SPtr<Texture> value) { obj->texture = value; }
	
	MaterialRTTI ()
	{
		addReflectablePtrField("texture", 0, &MaterialRTTI::getTexture, &MaterialRTTI::setTexture);
	}
	
	...
};
~~~~~~~~~~~~~

## Array fields  {#rtti_b_d}
Each of the valid field types (plain/reflectable/reflectable pointer), also come in array form. The array form requires two additional getter/setter methods that get/set array size, and normal getter/setter methods require an additional index parameter. For example if we wanted to extend our material so it contains multiple textures:
~~~~~~~~~~~~~{.cpp}
class Material : public IReflectable
{ 
	Vector<SPtr<Texture>> textures;
	
	... 
};

class MaterialRTTI : public RTTIType<Material, IReflectable, MaterialRTTI>
{
	SPtr<Texture> getTexture(Material* obj, UINT32 idx) { return obj->textures[idx]; }
	void setTexture(Material* obj, UINT32 idx, SPtr<Texture> value) { obj->textures[idx] = value; }
	
	UINT32 getNumTextures(Material* obj) { return obj->textures.size(); }
	void setNumTextures(Material* obj, UINT32 size) { obj->textures.resize(size); }
	
	MaterialRTTI ()
	{
		addReflectablePtrArrayField("texture", 0, &MaterialRTTI::getTexture, &MaterialRTTI::getNumTextures, 
			&MaterialRTTI::setTexture, &MaterialRTTI::setNumTextures);
	}
	
	...
};
~~~~~~~~~~~~~

Methods for registering array fields are:
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addPlainArrayField "RTTIType::addPlainArrayField"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectableArrayField "RTTIType::addReflectableArrayField"
 - @ref bs::RTTIType<Type, BaseType, MyRTTIType>::addReflectablePtrArrayField "RTTIType::addReflectablePtrArrayField"

They all follow the same syntax as in the example above.
 
## Advanced plain fields  {#rtti_b_e}
Although plain fields are primarily intended for simple built-in types, sometimes they also needs to be used on complex types. For example a `std::string` is often used as a field type, but it is not a simple built-in type, nor can we make it derive from @ref bs::IReflectable "IReflectable" so we can't use reflectable fields either. For these purposes you can use @ref bs::RTTIPlainType<T> "RTTIPlainType". This is a templated class you can specialize for your specific type. 

It provides methods for serializing/deserializing and retrieving object size. It has no advanced functionality like versioning (so if the structure of the type changes, it will break any previously serialized data), or keeping references to other objects.

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

Each specialization must implement all three (`toMemory/fromMemory/getDynamicSize`) methods. It must also provide a flag `hasDynamicSize` which determines whether or not it has dynamic size. Any structure whose size varies with each instance must set this flag to true. You must also set it to true if the size is static but larger than 255 bytes.

After you implement this class you will be able to use the type in getters/setters for plain fields as you would `int` or `float`. You can also use the @ref BS_ALLOW_MEMCPY_SERIALIZATION macro for simple structures. It will create a basic @ref bs::RTTIPlainType<T> "RTTIPlainType" specialization which uses `memcpy()`/`sizeof()` to implement the necessary methods.

If possible you should prefer implementing an @ref bs::IReflectable "IReflectable" for complex objects instead of this approach. But it can be useful for types where no other option is available (like third party or standard library types) or types you are sure won't change or require other advanced functionality.

@ref bs::RTTIPlainType<T> "RTTIPlainType" specializations can also be used as a more traditional form of serialization in case you find the RTTI system an overkill. For example if you needed to transfer data over a network. The system provides helper methods that allow you to easily work with plain types in such a case:
 - @ref bs::rttiReadElem "rttiReadElem" - Deserializes an object from the provided buffer and returns offset into the buffer after the read data
 - @ref bs::rttiWriteElem "rttiWriteElem" - Serializes an object into the provided buffer and returns offset into the buffer after the written data
 - @ref bs::rttiGetElemSize "rttiGetElemSize" - Returns a size an object

## RTTI field macros  {#rtti_b_f} 
In order to make definitions for fields in an RTTI type simpler, Banshee provides a set of macros you can use. These macros will automatically create getter/setter methods, and register the field. The macros are:
 - BS_RTTI_MEMBER_PLAIN(name, id) - Registers a new plain field for the variable `name` with a unique id `id`. 
 - BS_RTTI_MEMBER_PLAIN_NAMED(name, field, id) - Registers a new plain field for the variable `field` with a unique id `id`, and name `name`. Useful when the variable name cannot be used for the field name as it's done in BS_RTTI_MEMBER_PLAIN (e.g. if it is nested within another structure).
 - BS_RTTI_MEMBER_PLAIN_ARRAY(name, id) - Registers a new plain field for the variable `field` with a unique id `id`. The field contains a `Vector` of plain values.
 - BS_RTTI_MEMBER_REFL(name, id) - Same as BS_RTTI_MEMBER_PLAIN but for reflectable fields.
 - BS_RTTI_MEMBER_REFL_NAMED(name, field, id) - Same as BS_RTTI_MEMBER_PLAIN_NAMED but for reflectable fields.
 - BS_RTTI_MEMBER_REFL_ARRAY(name, id) - Same as BS_RTTI_MEMBER_PLAIN_ARRAY but for reflectable fields.
 - BS_RTTI_MEMBER_REFLPTR(name, id) - Same as BS_RTTI_MEMBER_PLAIN but for reflectable pointer fields.
 - BS_RTTI_MEMBER_REFLPTR_NAMED(name, field, id) - Same as BS_RTTI_MEMBER_PLAIN_NAMED but for reflectable pointer fields.
 - BS_RTTI_MEMBER_REFLPTR_ARRAY(name, id) - Same as BS_RTTI_MEMBER_PLAIN_ARRAY but for reflectable pointer fields.
 
Before you use any of those macros you must first call @ref BS_BEGIN_RTTI_MEMBERS() "BS_BEGIN_RTTI_MEMBERS" macro, and follow it with @ref BS_END_RTTI_MEMBERS() "BS_END_RTTI_MEMBERS" macro. These macros will define a new field `mInitMembers` in the @ref bs::RTTITypeBase "RTTIType", which you need to initialize in the constructor with the `this` pointer.

If we refactor our `TextureRTTI` example from above to use macros, it would look like so:
~~~~~~~~~~~~~{.cpp}
class TextureRTTI : public RTTIType<Texture, IReflectable, TextureRTTI>
{
	BS_BEGIN_RTTI_MEMBERS
		BS_RTTI_MEMBER_PLAIN(width, 0)
		BS_RTTI_MEMBER_PLAIN(height, 1)
	BS_END_RTTI_MEMBERS
	
	TextureRTTI ()
		:mInitMembers(this)
	{ }

	const String& getRTTIName() override
	{
		static String name = "Texture";
		return name;
	}

	UINT32 getRTTIId() override
	{
		return TID_Texture;
	}

	SPtr<IReflectable> newRTTIObject() override
	{
		return bs_shared_ptr_new<Texture>();
	}
};
~~~~~~~~~~~~~
 
# Advanced serialization  {#rtti_c}
Implementations of @ref bs::RTTIType<Type, BaseType, MyRTTIType> "RTTIType" can optionally override @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onSerializationStarted "RTTIType::onSerializationStarted", @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onSerializationEnded "RTTIType::onSerializationEnded", @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onDeserializationStarted "RTTIType::onDeserializationStarted" and @ref bs::RTTIType<Type, BaseType, MyRTTIType>::onDeserializationEnded "RTTIType::onDeserializationEnded" methods. As their names imply they will get called during serialization/deserialization and allow you to do any pre- or post-processing of the data. Most other systems (other than serialization) that access field data will also call these functions before reading, and after writing field data.

Each of those methods accepts an @ref bs::IReflectable "IReflectable" pointer to the object currently being processed. Each type that implements @ref bs::IReflectable "IReflectable" also comes with a `mRTTIData` field which is of @ref bs::Any "Any" type, and can be used for storing temporary data during serialization/deserialization (primarily when using the methods above).

# Using RTTI  {#rtti_d}
Once you have an object with a RTTI type fully implemented you can use it for various purposes.

## Getting object information  {#rtti_d_a}

~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

rtti_is_of_type<Texture>(myObject); // Check if the object's type matches the provided type
rtti_is_subclass<Texture>(myObject); // Check if type of the object is a subclass of the provided type
rtti_create(TID_Texture); // Create a new empty instance of the object

myObject->getTypeName(); // Name of the object's type
myObject->getTypeId(); // Unique identifier of the object's type

RTTITypeBase* rttiType = myObject->getRTTI(); // Returns the RTTIType object
rttiType->getBaseClass(); // Get RTTI type for the base class
rttiType->getDerivedClasses(); // Get all derived classes for an object
rttiType->getNumFields(); // Get number of RTTI fields 
rttiType->getField(index); // Get information about a field from its sequential index
RTTIField* rttiField = rttiType->findField(name); // Get information about a field from its name
rttiType->setPlainValue(myObject, "myField", someValue); // Set a value of a RTTI field (similar operations for all field types, including getters, and array fields)

rttiField->isPlainType(); // Is field storing a plain type
rttiField->isReflectableType(); // Is field storing a plain type
rttiField->isReflectablePtrType(); // Is field storing a plain type
rttiField->isArray(); // Does the field contain an array

// Cast RTTIField* to RTTIPlainFieldBase*, RTTIReflectableFieldBase* or RTTIReflectablePtrFieldBase* according to exact type.
// Use that type to access the field values (not shown here, but has the same effect as get/set methods on RTTIType* shown above, only more efficient)
~~~~~~~~~~~~~

## Serialization  {#rtti_d_b}
Serialization uses all the features shown in the chapter above in order to serialize an @ref bs::IReflectable "IReflectable" object into a stream of bytes, and vice versa. By default binary serialization is used, but user can implement textual serialization (like XML or JSON) using the RTTI system, if needed.

Binary serialized data can be output to memory, or to a file using: @ref bs::MemorySerializer "MemorySerializer", @ref bs::FileEncoder "FileEncoder", @ref bs::FileDecoder "FileDecoder". Their usage is simple:
~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

// Encode to file, then decode from it
FileEncoder fe("Path\To\My\File.asset");
fe.encode(myObject);

FileDecoder fd("Path\To\My\File.asset");
SPtr<IReflectable> myObjectCopy = fd.decode();

// Encode to memory, then decode from it
MemorySerializer ms;
UINT32 size;
UINT8* data = ms.encode(myObject, size);
SPtr<IReflectable> myObjectCopy2 = ms.decode(data, size);

bs_free(data);
~~~~~~~~~~~~~
