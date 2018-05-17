Persisting data				{#serializingObjects}
===============
[TOC]

Often components will have data you will want to persist across application sessions (for example **Renderable** component needs to remember which **Mesh** and **Material** it references). This persistent data will be automatically saved when a scene is saved, and loaded along with the scene. This process is called data serialization.

In order to make an object serializable you need to set up a special interface that allows the system to query information about the object, retrieve and set its data. This interface is known as Run Time Type Information (RTTI). In this example we talk primarily about components, but the same interface can be used for resources and normal objects.

Any object that is serializable (and therefore provides RTTI information) must implement the @ref bs::IReflectable "IReflectable" interface. If you are creating custom components or resources, **Component** and **Resource** base classes already derive from this interface so you don't need to specify it manually. The interface is simple, requiring you to implement two methods:
 - RTTITypeBase* getRTTI() const;
 - static RTTITypeBase* getRTTIStatic();
 
Implementations of these methods will return an object containing all RTTI for a specific class. In the rest of this manual we'll focus on explaning how to create a RTTI class implementation returned by these methods.

~~~~~~~~~~~~~{.cpp}
// IReflectable implementation for a component
class MyComponent : public Component
{
public:
	MyComponent(const HSceneObject& parent)
		:Component(parent)
	{}

	// ...class members...

	static RTTITypeBase* getRTTIStatic()
	{ return MyComponentRTTI::instance(); }

	RTTITypeBase* getRTTI() const override
	{ return MyComponent::getRTTIStatic(); }
};

// IReflectable implementation for a normal class
class MyClass : public IReflectable
{
	// ...class members...

	static RTTITypeBase* getRTTIStatic()
	{ return MyClassRTTI::instance(); }

	RTTITypeBase* getRTTI() const override
	{ return MyClass::getRTTIStatic(); }
};
~~~~~~~~~~~~~

# Creating the RTTI object {#serializingObjects_a}
All RTTI objects must implement the @ref bs::RTTIType<Type, BaseType, MyRTTIType> "RTTIType<Type, BaseType, MyRTTIType>" interface. The interface accepts three template parameters:
 - *Type* - Class of the object we're creating RTTI for (e.g. *MyClass* or *MyComponent* from example above)
 - *BaseType* - Base type of the object we're creating RTTI for (e.g. *IReflectable* or *Component* from example above)
 - *MyRTTIType* - Name of the RTTI class itself
 
~~~~~~~~~~~~~{.cpp}
class MyClassRTTI : public RTTIType<MyClass, IReflectable, MyClassRTTI>
{
	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
	// ...
};
~~~~~~~~~~~~~

The RTTI object must at least implement the following methods:
 - @ref bs::RTTITypeBase::getRTTIName() "RTTITypeBase::getRTTIName()" - Returns the name of the class the RTTI describes
 - @ref bs::RTTITypeBase::getRTTIId() "RTTITypeBase::getRTTIId()" - Returns an identifier that uniquely identifies the class
 - @ref bs::RTTITypeBase::newRTTIObject() "RTTITypeBase::newRTTIObject()" - Creates a new empty instance of the class the RTTI describes
 
~~~~~~~~~~~~~{.cpp}
enum TypeIds
{
	// These numbers must be unique or the system will complain. Use numbers higher than 200000 to avoid conflicts with built-in types
	TID_MyClass = 200000,
	TID_MyComponent = 200001
}

class MyClassRTTI : public RTTIType<MyClass, IReflectable, MyClassRTTI>
{
public:
	const String& getRTTIName() override
	{
		static String name = "MyClass";
		return name;
	}

	UINT32 getRTTIId() override
	{
		return TID_MyClass;
	}

	SPtr<IReflectable> newRTTIObject() override
	{
		return bs_shared_ptr_new<MyClass>();
	}
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	const String& getRTTIName() override
	{
		static String name = "MyComponent";
		return name;
	}

	UINT32 getRTTIId() override
	{
		return TID_MyComponent;
	}

	SPtr<IReflectable> newRTTIObject() override
	{
		return GameObjectRTTI::createGameObject<MyComponent>();
	}
};
~~~~~~~~~~~~~

> Note that when creating new instances of components within RTTI class, you must use **GameObjectRTTI::createGameObject<T>()** method, instead of just creating a normal shared pointer.

This is the minimal amount of work you need to do in order to implement RTTI. The RTTI types above now describe the class type, but not any of its members. In order to actually have class data serialized, you also need to define member fields.

# Member fields {#serializingObjects_b}
Member fields give the RTTI type a way to access (retrieve and assign) data from various members in the class the RTTI type describes. Let's imagine our *MyComponent* class had a few data members:
~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	UINT32 myInt;
	float myFloat;
	String myString;

	// ...
};
~~~~~~~~~~~~~

Its field definition within the RTTI type would look like so:
~~~~~~~~~~~~~{.cpp}
class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	BS_BEGIN_RTTI_MEMBERS
		BS_RTTI_MEMBER_PLAIN(myInt, 0)
		BS_RTTI_MEMBER_PLAIN(myFloat, 1)
		BS_RTTI_MEMBER_PLAIN(myString, 2)
	BS_END_RTTI_MEMBERS

	// ... 
};
~~~~~~~~~~~~~

Field definition portion of the RTTI type always begins with the @ref bs::BS_BEGIN_RTTI_MEMBERS "BS_BEGIN_RTTI_MEMBERS" macro, and ends with the @ref bs::BS_END_RTTI_MEMBERS "BS_END_RTTI_MEMBERS".

The field members themselves are defined by calling macros starting with BS_RTTI_MEMBER_*. The macro expects the name of the field it describes, as well as a unique ID of the field. The suffix of the BS_RTTI_MEMBER_* macro depends on the type of the field being added. There are three different types:
 - @ref bs::BS_RTTI_MEMBER_PLAIN "BS_RTTI_MEMBER_PLAIN" - Field containing basic data types like ints, floats, strings or other types that can be just trivially copied during serialization/deserialization.
 - @ref bs::BS_RTTI_MEMBER_REFL "BS_RTTI_MEMBER_REFL" - Field containing objects deriving from **IReflectable** (i.e. classes that have RTTI). The main advantage of using **IReflectable** types over plain ones is that you are allowed to add or remove fields from **IReflectable**%s RTTI type, and it wont break any data that was previously serialized. This is very important as you make changes to your components or resources, so you can still load previously saved data (e.g. imagine saving a level, changing a component, and then being unable to load the level). Plain types on the other hand must always keep the same structure, otherwise the serialized data will be broken.
 - @ref bs::BS_RTTI_MEMBER_REFLPTR "BS_RTTI_MEMBER_REFLPTR" - Fields containing pointers to objects deriving from **IReflectable**. Same as **BS_RTTI_MEMBER_REFL**, except that multiple fields can point to the same object. This ensures the object won't be serialized multiple times, wasting space and performance, and also ensures that the system can properly restore all references to the object when it's deserialized.
 
~~~~~~~~~~~~~{.cpp}
// Component definition with more complex field types
class MyComponent : public Component
{
public:
	// ...

	UINT32 myInt;
	float myFloat;
	String myString;
	SPtr<MyClass> myPtrClass;
	MyClass myClass;
	HRenderable renderable; // Component handle
	HMesh mesh; // Resource handle

	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	BS_BEGIN_RTTI_MEMBERS
		BS_RTTI_MEMBER_PLAIN(myInt, 0)
		BS_RTTI_MEMBER_PLAIN(myFloat, 1)
		BS_RTTI_MEMBER_PLAIN(myString, 2)
		BS_RTTI_MEMBER_REFLPTR(myPtrClass, 3)
		BS_RTTI_MEMBER_REFL(myClass, 4)
		BS_RTTI_MEMBER_REFL(renderable, 5)
		BS_RTTI_MEMBER_REFL(mesh, 6)
	BS_END_RTTI_MEMBERS

	// ... 
};
~~~~~~~~~~~~~

> Note that component and resource handles are considered **IReflectable**, and are therefore serialized by using **BS_RTTI_MEMBER_REFL**.

Each field must have an ID unique within the RTTI type. If you remove members from the RTTI type, you should not re-use their IDs for other members. Additionally, if the type of a specific field changes, you should assign it a new ID. The IDs allow the system to map previously serialized data to the current structure of the object.

## Arrays {#serializingObjects_b_a}
Array field types are also supported:
 - @ref bs::BS_RTTI_MEMBER_PLAIN_ARRAY "BS_RTTI_MEMBER_PLAIN_ARRAY"
 - @ref bs::BS_RTTI_MEMBER_REFL_ARRAY "BS_RTTI_MEMBER_REFL_ARRAY"
 - @ref bs::BS_RTTI_MEMBER_REFLPTR_ARRAY "BS_RTTI_MEMBER_REFLPTR_ARRAY"
 
They perform the same function as their non-array counterparts, except the fields are expected to contain a **Vector<T>** instead of a single entry.

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	Vector<UINT32> myInts;
	Vector<SPtr<MyClass>> myPtrClasses;
	Vector<HMesh> meshes;

	// ...
};

class MyComponentRTTI : public RTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	BS_BEGIN_RTTI_MEMBERS
		BS_RTTI_MEMBER_PLAIN_ARRAY(myInts, 0)
		BS_RTTI_MEMBER_REFLPTR_ARRAY(myPtrClasses, 1)
		BS_RTTI_MEMBER_REFL_ARRAY(meshes, 2)
	BS_END_RTTI_MEMBERS

	// ... 
};
~~~~~~~~~~~~~

# Using RTTI {#serializingObjects_c}
Once the RTTI has been created, in most cases it will be used automatically. In the case of components it will be used when saving/loading a scene, and in the case of resources it will be used when saving/loading a resource. But for any other class you will want to know how to utilize it manually.

To manually serialize an object you can use the @ref bs::FileEncoder "FileEncoder" class. Create the file encoder with a path to the output file, followed by a call to @ref bs::FileEncoder::encode "FileEncoder::encode()" with the object to encode as the parameter. The system will encode the provided object, as well as any other referenced **IReflectable** objects. 

~~~~~~~~~~~~~{.cpp}
SPtr<IReflectable> myObject = bs_shared_ptr_new<MyClass>();

FileEncoder fe("Path\To\My\File.asset");
fe.encode(myObject.get());
~~~~~~~~~~~~~

Once ready to decode, create a @ref bs::FileDecoder "FileDecoder" with the same file path. Then call @ref bs::FileDecoder::decode "FileDecoder::decode()".

~~~~~~~~~~~~~{.cpp}
FileDecoder fd("Path\To\My\File.asset");
SPtr<IReflectable> myObjectCopy = fd.decode();
~~~~~~~~~~~~~

You can also encode/decode to/from memory by using @ref bs::MemorySerializer "MemorySerializer".
~~~~~~~~~~~~~{.cpp}
SPtr<IReflectable> myObject = bs_shared_ptr_new<MyClass>();

MemorySerializer ms;
UINT32 size;
UINT8* data = ms.encode(myObject.get(), size);
SPtr<IReflectable> myObjectCopy2 = ms.decode(data, size);

bs_free(data);
~~~~~~~~~~~~~
