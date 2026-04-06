---
title: Run-time type information
---

Run-time type information (RTTI) provides meta-data about components (as well as non-component objects). This meta-data allows for features such as dynamic-casting, type-checking and most importantly, serialization.

Often components will have data you will want to persist across application sessions (for example **Renderable** component needs to remember which **Mesh** and **Material** it references). This persistent data will be automatically saved when a scene is saved, and loaded along with the scene. This process is called data serialization.

In order to make an object serializable you need to set up a RTTI interface that allows the system to query information about the object, retrieve and set its data. In this example we talk primarily about components, but the same interface can be used for resources and normal objects.

Any object that is serializable (and therefore provides RTTI information) must implement the @b3d::IReflectable interface. If you are creating custom components or resources, **Component** and **Resource** base classes already derive from this interface so you don't need to specify it manually. The interface is simple, requiring you to implement two methods:
 - RTTIType* GetRtti() const;
 - static RTTIType* GetRttiStatic();

Implementations of these methods will return an object containing all RTTI for a specific class. In the rest of this manual we'll focus on explaining how to create a RTTI class implementation returned by these methods.

~~~~~~~~~~~~~{.cpp}
// IReflectable implementation for a component
class MyComponent : public Component
{
public:
	MyComponent(const HSceneObject& parent)
		: Component(parent)
	{}

	// ...class members...

	static RTTIType* GetRttiStatic()
	{
		return MyComponentRTTI::Instance();
	}

	RTTIType* GetRtti() const override
	{
		return MyComponent::GetRttiStatic();
	}
};

// IReflectable implementation for a normal class
class MyClass : public IReflectable
{
	// ...class members...

	static RTTIType* GetRttiStatic()
	{
		return MyClassRTTI::Instance();
	}

	RTTIType* GetRtti() const override
	{
		return MyClass::GetRttiStatic();
	}
};
~~~~~~~~~~~~~

# Creating the RTTI type
All RTTI objects must implement the @b3d::TRTTIType<Type, BaseType, MyRTTIType> template interface. The interface accepts three template parameters:
 - *Type* - Class of the object we're creating RTTI for (e.g. *MyClass* or *MyComponent* from example above)
 - *BaseType* - Base type of the object we're creating RTTI for (e.g. *IReflectable* or *Component* from example above)
 - *MyRTTIType* - Name of the RTTI class itself

~~~~~~~~~~~~~{.cpp}
class MyClassRTTI : public TRTTIType<MyClass, IReflectable, MyClassRTTI>
{
	// ...
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	// ...
};
~~~~~~~~~~~~~

The RTTI object must at least implement the following methods:
 - @b3d::RTTIType::GetRttiName() - Returns the name of the class the RTTI describes
 - @b3d::RTTIType::GetRttiId() - Returns an identifier that uniquely identifies the class. This should be a unique integer equal or larger than 200000 (in order to avoid conflict with built-in types).
 - @b3d::RTTIType::NewRttiObject() - Creates a new empty instance of the class the RTTI describes

~~~~~~~~~~~~~{.cpp}
enum TypeIds
{
	TID_MyClass = 200000,
	TID_MyComponent = 200001
};

class MyClassRTTI : public TRTTIType<MyClass, IReflectable, MyClassRTTI>
{
public:
	const String& GetRttiName() override
	{
		static String name = "MyClass";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_MyClass;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		return B3DMakeShared<MyClass>();
	}
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
public:
	const String& GetRttiName() override
	{
		static String name = "MyComponent";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_MyComponent;
	}

	SPtr<IReflectable> NewRttiObject() override
	{
		return SceneObject::CreateEmptyComponent<MyComponent>();
	}
};
~~~~~~~~~~~~~

> Note that when creating new instances of components within the RTTI type class, you must use **SceneObject::CreateEmptyComponent<T>()** method, instead of just creating a normal shared pointer.

This is the minimal amount of work you need to do in order to implement RTTI. The RTTI types above now describe the class type, but not any of its members. In order to actually have class data serialized, you also need to define member fields.

# Member fields
Member fields give the RTTI type a way to access (retrieve and assign) data from various members in the class the RTTI type describes. 

Let's imagine our *MyComponent* class had a few data members:

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	u32 myInt;
	float myFloat;
	String myString;

	// ...
};
~~~~~~~~~~~~~

Its field definition within the RTTI type would look like so:

~~~~~~~~~~~~~{.cpp}
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myInt, 0)
		B3D_RTTI_MEMBER(myFloat, 1)
		B3D_RTTI_MEMBER(myString, 2)
	B3D_RTTI_END_MEMBERS

public:
	// ... GetRttiName, GetRttiId, NewRttiObject ...
};
~~~~~~~~~~~~~

Field definition portion of the RTTI type always begins with the @B3D_RTTI_BEGIN_MEMBERS macro, and ends with the @B3D_RTTI_END_MEMBERS macro. Note that these macros must appear before the `public:` section of the RTTI class.

## Field types
The main macro for defining fields is @B3D_RTTI_MEMBER, which takes the field name and a unique ID. This macro works for:
 - **Plain types**: Basic types like ints, floats, strings, and POD structs
 - **Reflectable types**: Objects deriving from **IReflectable** (either by value or as SPtr)
 - **Resource handles**: HMesh, HTexture, etc.
 - **Component handles**: HRenderable, HCamera, etc.

~~~~~~~~~~~~~{.cpp}
// Component definition with various field types
class MyComponent : public Component
{
public:
	// ...

	u32 myInt;
	float myFloat;
	String myString;
	SPtr<MyClass> myPtrClass;
	MyClass myClass;
	HRenderable renderable; // Component handle
	HMesh mesh; // Resource handle

	// ...
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myInt, 0)
		B3D_RTTI_MEMBER(myFloat, 1)
		B3D_RTTI_MEMBER(myString, 2)
		B3D_RTTI_MEMBER(myPtrClass, 3)
		B3D_RTTI_MEMBER(myClass, 4)
		B3D_RTTI_MEMBER(renderable, 5)
		B3D_RTTI_MEMBER(mesh, 6)
	B3D_RTTI_END_MEMBERS

public:
	// ...
};
~~~~~~~~~~~~~

Each field must have an ID unique within the RTTI type. If you remove members from the RTTI type, you should not re-use their IDs for other members. Additionally, if the type of a specific field changes, you should assign it a new ID. The IDs allow the system to map previously serialized data to the current structure of the object.

## Container fields
For container types (like Vector, Map, etc.), use the @B3D_RTTI_MEMBER_CONTAINER macro:

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	// ...

	Vector<u32> myInts;
	Vector<SPtr<MyClass>> myPtrClasses;
	Vector<HMesh> meshes;

	// ...
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER_CONTAINER(myInts, 0)
		B3D_RTTI_MEMBER_CONTAINER(myPtrClasses, 1)
		B3D_RTTI_MEMBER_CONTAINER(meshes, 2)
	B3D_RTTI_END_MEMBERS

public:
	// ...
};
~~~~~~~~~~~~~

## Field info and flags
You can provide additional metadata about fields using the @B3D_RTTI_MEMBER_INFO and @B3D_RTTI_MEMBER_CONTAINER_INFO macros, which accept a @b3d::RTTIFieldInfo parameter:

~~~~~~~~~~~~~{.cpp}
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		// Skip this field when comparing or copying deltas
		B3D_RTTI_MEMBER_INFO(mPrefabResourceId, 0,
			RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare | RTTIFieldFlag::SkipInDeltaCopy))

		// Normal field
		B3D_RTTI_MEMBER(myInt, 1)
	B3D_RTTI_END_MEMBERS

public:
	// ...
};
~~~~~~~~~~~~~

Available field flags include:
- **SkipInDeltaCompare** - Field won't be compared when generating deltas
- **SkipInDeltaCopy** - Field won't be copied when applying deltas
- **WeakRef** - Field is a weak reference and won't be serialized deeply

## Generated members
Sometimes you need to serialize data that isn't directly stored in the class, but is computed or transformed. Use @B3D_RTTI_GENERATED_MEMBER for such fields:

~~~~~~~~~~~~~{.cpp}
class SceneObjectRTTI : public TRTTIType<SceneObject, GameObject, SceneObjectRTTI>
{
	// These members exist in the RTTI class, not the SceneObject
	Vector<SPtr<SceneObject>> mChildren;
	Vector<SPtr<Component>> mComponents;

	B3D_RTTI_BEGIN_MEMBERS
		// Generated members reference fields in the RTTI class
		B3D_RTTI_GENERATED_MEMBER_CONTAINER(mChildren, 0)
		B3D_RTTI_GENERATED_MEMBER_CONTAINER(mComponents, 1)

		// Regular members reference fields in SceneObject
		B3D_RTTI_MEMBER(mLocalTfrm, 2)
	B3D_RTTI_END_MEMBERS

public:
	void OnOperationStarted(SceneObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		// Populate generated members before serialization
		if (operationType.IsSet(RTTIOperationType::ReadBit))
		{
			mChildren.clear();
			for (const auto& entry : object.mChildren)
				mChildren.push_back(entry.GetShared());
		}
	}
};
~~~~~~~~~~~~~

Note the system guarantees that a unique instance of the RTTI class will be created for each object, so you can safely store references to generated members in the RTTI class.

# Using RTTI
Once the RTTI type class has been created, in most cases it will be used automatically. In the case of components it will be used when saving/loading a scene, and in the case of resources it will be used when saving/loading a resource. But for any other class you will want to know how to utilize it manually.

## Manually serializing

To manually serialize an object you can use the @b3d::FileEncoder class or @b3d::BinarySerializer class.

### Using FileEncoder

@b3d::FileEncoder serializes objects to a file:

~~~~~~~~~~~~~{.cpp}
SPtr<IReflectable> myObject = B3DMakeShared<MyClass>();

FileEncoder encoder("Path/To/My/File.asset");
encoder.Encode(myObject.get());
~~~~~~~~~~~~~

To decode from a file, use @b3d::FileDecoder:

~~~~~~~~~~~~~{.cpp}
FileDecoder decoder("Path/To/My/File.asset");
SPtr<IReflectable> myObjectCopy = decoder.Decode();
~~~~~~~~~~~~~

### Using BinarySerializer

@b3d::BinarySerializer serializes objects to/from memory streams:

~~~~~~~~~~~~~{.cpp}
SPtr<IReflectable> myObject = B3DMakeShared<MyClass>();

BinarySerializer binarySerializer;
SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
binarySerializer.Encode(myObject.get(), stream);

stream->Seek(0);
SPtr<IReflectable> myObjectCopy = binarySerializer.Decode(stream, stream->Size());
~~~~~~~~~~~~~

> For advanced serialization options, contexts, and detailed control over the serialization process, see the [Advanced RTTI](../14_advancedRtti.md) manual.

## Casting & queries
Aside from using RTTI for serialization, you can also use it to manually query various information about objects, as well as create and cast object instances.

Global queries:
 - @b3d::B3DRTTIIsOfType - Checks if a specific object is of type *T*
 - @b3d::B3DRTTIIsSubclass - Checks if a specific object is derived from type *T*
 - @b3d::B3DRTTICreate - Creates a new object from its type ID
 - @b3d::B3DRTTICast - Casts an object to the specified type if the cast is valid, or returns null otherwise

**IReflectable** queries:
 - @b3d::IReflectable::GetTypeName - Gets the name of the object's type
 - @b3d::IReflectable::GetTypeId - Gets the type ID of the object's type

~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

B3DRTTIIsOfType<Texture>(myObject);
B3DRTTIIsSubclass<Texture>(myObject);
B3DRTTICreate(TID_Texture);
Texture* myTexture = B3DRTTICast<Texture>(myObject);

myObject->GetTypeName();
myObject->GetTypeId();
~~~~~~~~~~~~~

# Advanced RTTI

For advanced RTTI features including:
- Manually defining fields without macros using RTTI iterators
- Creating custom RTTIPlainType specializations
- Advanced serialization options (shallow, compressed, no-metadata)
- Operation notifications and versioning
- RTTI querying and introspection
- Data block fields

See the [Advanced RTTI](../14_advancedRtti.md) manual.
