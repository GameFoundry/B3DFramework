---
title: Advanced RTTI
---

This manual is a continuation of the [Serializing objects](06_Gameplay/01_serializingObjects.md) manual, focusing on advanced features of the RTTI system.

# Manually defining fields

Previously we have shown how to define RTTI member fields using the B3D_RTTI_MEMBER macros. While this approach is recommended for most use cases, sometimes you need more advanced functionality. The macros are limited to referencing class fields directly, but sometimes you might want to access data returned by a method, process data during serialization (e.g., compression), or work with non-standard data structures.

You can achieve this by manually defining fields using the @b3d::TRTTIType::AddField method. This method accepts:
- A field name and unique ID
- Getter/setter methods that use RTTI iterators
- Optional field information (@b3d::RTTIFieldInfo)

## RTTI Iterators

The RTTI system uses @b3d::TRTTIIterator<DataType, IsContainer> for accessing field data. Iterators provide a uniform interface for reading and writing both single values and containers.

The `IsContainer` template parameter determines the iterator's behavior:
- `false` - Iterator treats the data type as a single value (acts as a faux single-element iterator)
- `true` - Iterator treats the data type as a container with multiple elements

## Manual field example

Here's how to manually define a field with custom getter/setter logic:

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	MyComponent(const HSceneObject& parent)
		: Component(parent)
	{}

	u32 myInt;
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	// Getter: Creates an iterator for the field
	UPtrRTTIIterator<u32, false> GetMyIntIterator(MyComponent& object, FrameAllocator& allocator)
	{
		return CreateRTTIIterator<u32, false>(allocator, object.myInt);
	}

	// Getter: Returns the value at the iterator position
	const u32& GetMyIntValue(MyComponent& object, FrameAllocator& allocator, TRTTIIterator<u32, false>& iterator)
	{
		return *iterator;
	}

	// Setter: Sets the value at the iterator position
	void SetMyIntValue(MyComponent& object, FrameAllocator& allocator, TRTTIIterator<u32, false>& iterator, const u32& value)
	{
		iterator = value;
	}

	B3D_RTTI_BEGIN_MEMBERS
		// Normal macro-based field for comparison
		// B3D_RTTI_MEMBER(myInt, 0)
	B3D_RTTI_END_MEMBERS

public:
	MyComponentRTTI()
	{
		// Manually add the field using AddField method
		AddField<MyComponentRTTI, MyComponent, u32>(
			"myInt", 0,
			&MyComponentRTTI::GetMyIntIterator,
			&MyComponentRTTI::GetMyIntValue,
			&MyComponentRTTI::SetMyIntValue);
	}

	const String& GetRttiName() override
	{
		static String name = "MyComponent";
		return name;
	}

	u32 GetRttiId() const override
	{
		return TID_MyComponent;
	}

	TShared<IReflectable> NewRttiObject() override
	{
		return SceneObject::CreateEmptyComponent<MyComponent>();
	}
};
~~~~~~~~~~~~~

## Manual container field example

For containers, use `IsContainer = true` to allow iteration over elements:

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	Vector<u32> myInts;
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	UPtrRTTIIterator<Vector<u32>, true> GetMyIntsIterator(MyComponent& object, FrameAllocator& allocator)
	{
		return CreateRTTIIterator<Vector<u32>, true>(allocator, object.myInts);
	}

	const u32& GetMyIntsValue(MyComponent& object, FrameAllocator& allocator, TRTTIIterator<Vector<u32>, true>& iterator)
	{
		return *iterator;
	}

	void SetMyIntsValue(MyComponent& object, FrameAllocator& allocator, TRTTIIterator<Vector<u32>, true>& iterator, const u32& value)
	{
		iterator = value;
	}

	B3D_RTTI_BEGIN_MEMBERS
	B3D_RTTI_END_MEMBERS

public:
	MyComponentRTTI()
	{
		AddField<MyComponentRTTI, MyComponent, Vector<u32>>(
			"myInts", 0,
			&MyComponentRTTI::GetMyIntsIterator,
			&MyComponentRTTI::GetMyIntsValue,
			&MyComponentRTTI::SetMyIntsValue);
	}

	// ... GetRttiName, GetRttiId, NewRttiObject ...
};
~~~~~~~~~~~~~

## Custom processing example

Manual fields allow you to transform data during serialization:

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	String GetCompressedData() const;
	void SetCompressedData(const String& data);

private:
	String mInternalData;
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	String mTempData;

	UPtrRTTIIterator<String, false> GetDataIterator(MyComponent& object, FrameAllocator& allocator)
	{
		// Store compressed data in temporary field during serialization
		mTempData = object.GetCompressedData();
		return CreateRTTIIterator<String, false>(allocator, mTempData);
	}

	const String& GetDataValue(MyComponent& object, FrameAllocator& allocator, TRTTIIterator<String, false>& iterator)
	{
		return *iterator;
	}

	void SetDataValue(MyComponent& object, FrameAllocator& allocator, TRTTIIterator<String, false>& iterator, const String& value)
	{
		// Decompress when setting
		object.SetCompressedData(value);
	}

	B3D_RTTI_BEGIN_MEMBERS
	B3D_RTTI_END_MEMBERS

public:
	MyComponentRTTI()
	{
		AddField<MyComponentRTTI, MyComponent, String>(
			"compressedData", 0,
			&MyComponentRTTI::GetDataIterator,
			&MyComponentRTTI::GetDataValue,
			&MyComponentRTTI::SetDataValue);
	}

	// ... GetRttiName, GetRttiId, NewRttiObject ...
};
~~~~~~~~~~~~~

> By using manual field definitions, you gain full control over how data is accessed and stored during serialization.

# Plain types vs IReflectable types

The RTTI system supports two main categories of serializable data: **plain types** and **IReflectable types**. Understanding the differences between them is crucial for choosing the right approach for your data.

## Plain types

Plain types are simple, value-based types that are serialized directly to a binary stream. They include:
- Built-in types (int, float, bool, etc.)
- POD (Plain Old Data) structs
- Standard containers (Vector, Map, String) containing plain types
- Custom types with RTTIPlainType specialization

**Advantages:**
- **Faster serialization** - Direct binary encoding without metadata overhead
- **Smaller serialized size** - No type information or versioning data
- **Simple implementation** - Just implement ToMemory/FromMemory/GetSize methods

**Limitations:**
- **No reference tracking** - Shared pointers, resource handles, and game object handles are not properly handled
- **Explicit versioning** - Changing the type structure breaks previously serialized data unless explicitly handled
- **Plain types only** - Can only contain other plain types, not IReflectable objects
- **No polymorphism** - Cannot handle derived types differently

~~~~~~~~~~~~~{.cpp}
// Plain type - Fast but limited
struct PlayerStats
{
    i32 health;
    i32 mana;
    float speed;
    // Can only contain plain types!
    Vector<i32> inventory; // OK - Vector of plain type
};

B3D_ALLOW_MEMCPY_SERIALIZATION(PlayerStats)

// This will NOT work correctly with plain types:
struct BrokenExample
{
    TShared<Texture> texture; // BROKEN - shared pointer not handled
    HMesh mesh;            // BROKEN - resource handle not preserved
    HRenderable component; // BROKEN - component handle not preserved
};
~~~~~~~~~~~~~

## IReflectable types

IReflectable types are full RTTI objects that derive from @b3d::IReflectable. They include:
- Components
- Resources
- Scene objects
- Any custom class deriving from IReflectable

**Advantages:**
- **Reference tracking** - Shared pointers, resource handles, and game object handles are properly serialized and restored
- **Versioning** - Field IDs allow adding/removing fields without breaking old data
- **Polymorphism** - Base class pointers correctly deserialize to derived types
- **Can contain anything** - Can contain both plain types and other IReflectable objects
- **Operation notifications** - OnOperationStarted/OnOperationEnded callbacks for custom logic

**Limitations:**
- **Slower serialization** - Additional metadata and reference tracking overhead
- **Larger serialized size** - Stores type information and field IDs
- **More complex** - Requires full RTTI class implementation

~~~~~~~~~~~~~{.cpp}
// IReflectable type - Full featured
class GameEntity : public IReflectable
{
public:
    // Can contain plain types
    String name;
    Vector3 position;

    // Can contain shared pointers - properly tracked
    TShared<Material> material;

    // Can contain resource handles - properly preserved
    HMesh mesh;
    HTexture texture;

    // Can contain other IReflectable objects
    TShared<PhysicsData> physics;

    // References are maintained after deserialization!
    static RTTIType* GetRttiStatic();
    RTTIType* GetRtti() const override;
};
~~~~~~~~~~~~~

## When to use plain types

Use plain types when:
- The data is simple and self-contained
- No shared pointers or handles are involved
- Performance is critical (networking, frequent saves)
- The data structure is stable and won't change
- You don't need versioning

~~~~~~~~~~~~~{.cpp}
// Good use cases for plain types:
struct Vector3 { float x, y, z; };
struct Color { u8 r, g, b, a; };
struct NetworkPacketHeader { u32 id; u16 size; };
struct ConfigSettings { i32 width; i32 height; bool fullscreen; };
~~~~~~~~~~~~~

## When to use IReflectable types

Use IReflectable types when:
- The data contains shared pointers or handles
- You need to preserve references between objects
- The data structure may change over time (versioning needed)
- The type is part of the scene hierarchy (components, resources)

## Mixing plain and IReflectable types

You can use plain types within IReflectable types, but **not** the other way around:

~~~~~~~~~~~~~{.cpp}
// ✓ OK - IReflectable containing plain types
class MyComponent : public Component
{
    Vector3 position;      // Plain type
    String name;           // Plain type
    HMesh mesh;            // Handle (tracked by IReflectable system)
};

// ✗ BROKEN - Plain type containing IReflectable
struct BrokenStruct
{
    TShared<MyComponent> component; // Will NOT work - reference lost!
};

// ✓ OK - Use IReflectable wrapper instead
class WorkingWrapper : public IReflectable
{
    TShared<MyComponent> component; // Works correctly
};
~~~~~~~~~~~~~

# Specializing plain types

Although plain fields are primarily intended for simple built-in types, sometimes they need to be used for complex types. For example, **std::string** is often used as a field type, but it is not a simple built-in type, nor can we make it derive from **IReflectable**. For these purposes, you can specialize @b3d::RTTIPlainType<T>.

Once you specialize this template for your type, implementing all the required methods, you will be able to use your type in plain fields. Without this specialization, the system will refuse to compile the RTTI type.

The specialization involves implementing methods for serialization/deserialization and retrieving object size. 

## Basic RTTIPlainType specialization

For example, if we wanted to serialize a simple struct:

~~~~~~~~~~~~~{.cpp}
struct MyStruct
{
	i32 a;
	float b;
	bool c;
};

template<>
struct RTTIPlainType<MyStruct>
{
	enum { id = 200000 }; // Provide unique ID
	enum { hasDynamicSize = 0 }; // Flag whether the size is dynamic

	static BitLength ToMemory(const MyStruct& data, Bitstream& stream, const RTTIFieldInfo& info, bool compress)
	{
		BitLength size;

		// Serialize only the first two fields
		size += stream.WriteBytes(data.a);
		size += stream.WriteBytes(data.b);

		return size;
	}

	static BitLength FromMemory(MyStruct& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
	{
		BitLength size;

		size += stream.ReadBytes(data.a);
		size += stream.ReadBytes(data.b);

		return size;
	}

	static BitLength GetSize(const MyStruct& data, const RTTIFieldInfo& fieldInfo, bool compress)
	{
		return sizeof(data.a) + sizeof(data.b);
	}
};
~~~~~~~~~~~~~

Each specialization must implement all three **ToMemory()**, **FromMemory()**, and **GetSize()** methods. It must also provide a unique **id** that identifies the type, as well as a **hasDynamicSize** flag.

### Method parameters

**ToMemory()** and **FromMemory()** have similar signatures:
- **data** - Object to write (**ToMemory()**) or object to receive read results (**FromMemory()**)
- **stream** - @b3d::Bitstream object used for reading/writing serialized data
- **info** - Optional additional information about the field being serialized (@b3d::RTTIFieldInfo)
- **compress** - If true, you can provide space-efficient encoding. Used for networking where data size matters. When false, all data sizes must be in multiples of bytes. When true, data can be sub-byte sized (e.g., boolean as 1 bit).
- **return** - Total size of data written/read as @b3d::BitLength (full bytes + bits in the last byte)

**GetSize()** accepts similar parameters without the **stream** parameter. It should calculate and return the data size as **BitLength**.

### Type ID

You should set **id** to a unique number not used by existing types. Avoid clashing with built-in type IDs which are listed as enum values starting with **TID_** (e.g., **TID_Texture = 1001**). Use values >= 200000 for custom types.

After the specialization is implemented, you can use the type in plain fields as you would *int* or *float*. Note that the framework already provides many such specializations, including ones for strings, vectors, and maps.

## Dynamic size types

If your structure has dynamic size or is fixed size larger than 256 bytes, you must set the **hasDynamicSize** flag to 1. The size should be returned from the **GetSize()** method. For dynamic size types, you must also encode the size as a header before all data in **ToMemory()**.

Fields with dynamic size must write the actual size as a header before encoded data. You can use helper methods:
- @b3d::B3DRTTIWriteWithSizeHeader - Write data with a header
- @b3d::B3DRTTIReadSizeHeader - Read size from the header
- @b3d::B3DRTTIAddHeaderSize - Calculate header size

For example, serializing a string:

~~~~~~~~~~~~~{.cpp}
template<>
struct RTTIPlainType<String>
{
	enum { id = TID_String };
	enum { hasDynamicSize = 1 }; // Dynamic size

	static BitLength ToMemory(const String& data, Bitstream& stream, const RTTIFieldInfo& info, bool compress)
	{
		return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
		{
			stream.WriteBytes((u8*)data.data(), data.size());
			return data.size();
		});
	}

	static BitLength FromMemory(String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
	{
		BitLength size;
		BitLength headerSize = B3DRTTIReadSizeHeader(stream, compress, size);

		// 'size' includes the header, so subtract it
		BitLength stringSize = size - headerSize;
		data = String(stream.Cursor(), stringSize.Bytes);
		stream.Skip(stringSize);

		return size;
	}

	static BitLength GetSize(const String& data, const RTTIFieldInfo& fieldInfo, bool compress)
	{
		BitLength dataSize = data.size();
		B3DRTTIAddHeaderSize(dataSize, compress);
		return dataSize;
	}
};
~~~~~~~~~~~~~

## RTTIPlainTypeHelper

For most cases, you should use @b3d::RTTIPlainTypeHelper instead of manually implementing RTTIPlainType. This helper simplifies the implementation and provides built-in versioning support.

RTTIPlainTypeHelper allows you to define plain types by simply enumerating the fields to serialize:

~~~~~~~~~~~~~{.cpp}
struct PlayerData
{
	String name;
	i32 level;
	float health;
	Vector3 position;
};

template<>
struct RTTIPlainType<PlayerData> : RTTIPlainTypeHelper<PlayerData, TID_PlayerData>
{
	template <class Processor>
	static void RTTIEnumerateFields(PlayerData& data, Processor& processor)
	{
		processor(data.name);
		processor(data.level);
		processor(data.health);
		processor(data.position);
	}
};
~~~~~~~~~~~~~

The helper takes care of:
- Implementing ToMemory/FromMemory/GetSize methods
- Adding size headers for dynamic data
- Handling versioning (optional)

### RTTIPlainTypeHelper template parameters

The template accepts up to 4 parameters:

~~~~~~~~~~~~~{.cpp}
RTTIPlainTypeHelper<SerializedObjectType, TypeId, Version, HasDynamicSize>
~~~~~~~~~~~~~

- **SerializedObjectType** - Your data type
- **TypeId** - Unique ID for the type (required)
- **Version** - Current version number (default: 255 = no versioning)
- **HasDynamicSize** - 1 for dynamic size, 0 for fixed size (default: 1)

### Versioning with RTTIPlainTypeHelper

To enable versioning, provide a version number and handle different versions in RTTIEnumerateFields:

~~~~~~~~~~~~~{.cpp}
struct PlayerData
{
	String name;
	i32 level;
	float health;
	Vector3 position;
	i32 mana; // Added in version 1
};

template<>
struct RTTIPlainType<PlayerData> : RTTIPlainTypeHelper<PlayerData, TID_PlayerData, 1>
{
	template <class Processor>
	static void RTTIEnumerateFields(PlayerData& data, Processor& processor, u8 version)
	{
		// Fields present in all versions
		processor(data.name);
		processor(data.level);
		processor(data.health);
		processor(data.position);

		// Field added in version 1
		if (version >= 1)
			processor(data.mana);
	}
};
~~~~~~~~~~~~~

When deserializing old data (version 0), the `mana` field will be skipped and retain its default value. The version is automatically encoded in the stream.

### Fixed size optimization

If your type has a fixed size ≤ 256 bytes and doesn't contain dynamic data, set HasDynamicSize to 0 for better performance:

~~~~~~~~~~~~~{.cpp}
struct Vector3Data
{
	float x, y, z;
};

template<>
struct RTTIPlainType<Vector3Data> : RTTIPlainTypeHelper<Vector3Data, TID_Vector3Data, 255, 0>
{
	template <class Processor>
	static void RTTIEnumerateFields(Vector3Data& data, Processor& processor)
	{
		processor(data.x);
		processor(data.y);
		processor(data.z);
	}
};
~~~~~~~~~~~~~

> **Note:** With `HasDynamicSize = 0`, no size header is written, saving 4 bytes per instance.

## Plain old data types

For very simple structures with no dynamic data, you can use the @B3D_ALLOW_MEMCPY_SERIALIZATION macro as a shortcut. It creates a basic **RTTIPlainType<T>** specialization that uses *memcpy()* and *sizeof()*.

~~~~~~~~~~~~~{.cpp}
// Simple plain old data type
struct SimpleData
{
	i32 some;
	float data;
	float here;
};

B3D_ALLOW_MEMCPY_SERIALIZATION(SimpleData, TID_SimpleData)
~~~~~~~~~~~~~

> **Warning:** Only use this macro for trivially copyable types without dynamic data (no strings, vectors, pointers, etc.). For anything more complex, use RTTIPlainTypeHelper instead.

## Helper methods

**RTTIPlainType** specializations can be used as a traditional form of serialization if you find the RTTI system overkill. For example, for network data transfer without advanced versioning. Helper methods make this easy:
- @b3d::B3DRTTIRead - Deserialize from stream, advance cursor, return bytes/bits read
- @b3d::B3DRTTIWrite - Serialize to stream, advance cursor, return bytes/bits written
- @b3d::B3DRTTISize - Return object size as **BitLength**

~~~~~~~~~~~~~{.cpp}
// Assuming Vector has an RTTIPlainType<T> specialization (which it does by default)

Vector<SimpleData> myData;
// Fill out myData...

// Serialize the entire vector and all of its contents
BitLength size = B3DRTTISize(myData);

Bitstream stream(size.Bytes);
B3DRTTIWrite(myData, stream);

// Deserialize the data
stream.Seek(0); // Reset cursor to beginning
Vector<SimpleData> myDataCopy;
B3DRTTIRead(myDataCopy, stream);
~~~~~~~~~~~~~

# File encoders and binary serializers

The basic serialization manual showed how to use @b3d::FileEncoder and @b3d::BinarySerializer for simple cases. This section covers advanced usage.

## BinarySerializer options

@b3d::BinarySerializer supports various flags through @b3d::BinarySerializerFlags:

~~~~~~~~~~~~~{.cpp}
BinarySerializer bs;
TShared<IReflectable> myObject = B3DMakeShared<MyClass>();
TShared<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();

// Shallow serialization - don't encode referenced objects
// Referenced objects will become null when deserialized
bs.Encode(myObject.get(), stream, BinarySerializerFlag::Shallow);

// Compressed serialization - use sub-byte encoding for smaller size
// Suitable for networking. Plain types can use bit-level encoding.
bs.Encode(myObject.get(), stream, BinarySerializerFlag::Compress);

// No metadata - smaller size but requires identical RTTI types for decoding
// The decoder must have the exact same RTTI structure (same fields, IDs, types)
bs.Encode(myObject.get(), stream, BinarySerializerFlag::NoMeta);

// Combine flags
bs.Encode(myObject.get(), stream,
	BinarySerializerFlag::Compress | BinarySerializerFlag::NoMeta);
~~~~~~~~~~~~~

### Shallow serialization

Use **Shallow** when you want to serialize an object without its referenced objects:

~~~~~~~~~~~~~{.cpp}
class MyClass : public IReflectable
{
	TShared<Texture> texture; // Won't be serialized with Shallow flag
	i32 someValue; // Will be serialized
};

BinarySerializer bs;
bs.Encode(myObject.get(), stream, BinarySerializerFlag::Shallow);
// After deserialization, texture will be null
~~~~~~~~~~~~~

### Compressed serialization

Use **Compress** for network transmission or to reduce file size:

~~~~~~~~~~~~~{.cpp}
// Encode with compression
bs.Encode(myObject.get(), stream, BinarySerializerFlag::Compress);

// Must decode with the same flag
TShared<IReflectable> decoded = bs.Decode(stream, stream->Size(), BinarySerializerFlag::Compress);
~~~~~~~~~~~~~

When **Compress** is true, the `compress` parameter in **RTTIPlainType::ToMemory()/FromMemory()** will be true, allowing those types to use bit-level encoding.

### No metadata serialization

Use **NoMeta** when you need minimal serialization size and can guarantee RTTI types are identical:

~~~~~~~~~~~~~{.cpp}
// Encode without metadata
bs.Encode(myObject.get(), stream, BinarySerializerFlag::NoMeta);

// Decode without metadata - RTTI types must match exactly
TShared<IReflectable> decoded = bs.Decode(stream, stream->Size(), BinarySerializerFlag::NoMeta);
~~~~~~~~~~~~~

> **Warning**: NoMeta is fragile. If you add, remove, or reorder fields in the RTTI type, deserialization will fail or produce corrupted data. Only use this for temporary serialization (e.g., networking within the same game version).

## Using RTTI schemas

When using **NoMeta**, you can optionally provide a saved @b3d::RTTISchema that describes the types as they were when data was encoded. This allows decoding even if RTTI types have changed:

~~~~~~~~~~~~~{.cpp}
BinarySerializer bs;

// Encode without metadata
bs.Encode(myObject.get(), stream, BinarySerializerFlag::NoMeta);

// Save the current RTTI schema
TShared<RTTISchema> schema = myObject->GetRtti()->GetSchema();

// Later, decode using the saved schema
TShared<IReflectable> decoded = bs.Decode(stream, stream->Size(),
	BinarySerializerFlag::NoMeta, nullptr, schema);
~~~~~~~~~~~~~

## Serialization contexts

You can pass custom context objects to serialization operations to share state between RTTI types:

~~~~~~~~~~~~~{.cpp}
// Create a custom context
struct MySerializationContext : RTTIOperationContext
{
	bool SomeFlag = true;
	String SharedData = "test";
	UnorderedMap<UUID, TShared<Resource>> ResourceCache;
};

MySerializationContext context;

// Pass context to encode
BinarySerializer bs;
bs.Encode(myObject.get(), stream, context);

// Access context in RTTI class:
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	void OnOperationStarted(MyComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		auto* myContext = context.As<MySerializationContext>();
		if (myContext && myContext->SomeFlag)
		{
			// Access shared data, caches, etc.
			TShared<Resource> resource = myContext->ResourceCache[someId];
		}
	}
};
~~~~~~~~~~~~~

Contexts are useful for:
- Sharing caches between objects during serialization
- Passing configuration flags to control serialization behavior
- Collecting statistics or diagnostic information
- Coordinating between multiple RTTI types

## FileEncoder

@b3d::FileEncoder/Decoder work similarly to BinarySerializer but handle file I/O directly:

~~~~~~~~~~~~~{.cpp}
// Encode to file
FileEncoder encoder("Path/To/My/File.asset");
encoder.Encode(myObject.get());

// Decode from file
FileDecoder decoder("Path/To/My/File.asset");
TShared<IReflectable> myObjectCopy = decoder.Decode();
~~~~~~~~~~~~~

FileEncoder doesn't support the same flag system as BinarySerializer, but it automatically handles file creation, buffering, and error checking.

# Operation notifications

RTTI types can override notification methods to be called when serialization operations begin and end:

~~~~~~~~~~~~~{.cpp}
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	void OnOperationStarted(MyComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		// Called before serialization/deserialization
		if (operationType.IsSet(RTTIOperationType::Serialization))
		{
			// Prepare for serialization (e.g., collect data)
		}
		else if (operationType.IsSet(RTTIOperationType::Deserialization))
		{
			// Prepare for deserialization (e.g., allocate resources)
		}
		else if (operationType.IsSet(RTTIOperationType::DeltaGenerate))
		{
			// Preparing to generate delta between objects
		}
	}

	void OnOperationEnded(MyComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		// Called after serialization/deserialization
		if (operationType.IsSet(RTTIOperationType::Deserialization))
		{
			// Finalize deserialization (e.g., register with managers, initialize)
			object.Initialize();
		}
	}
};
~~~~~~~~~~~~~

## Operation types

@b3d::RTTIOperationType includes:
- **Serialization** - Reading object data for encoding
- **Deserialization** - Writing data to create/update an object
- **DeltaGenerate** - Generating delta between objects
- **DeltaApply** - Applying delta to an object
- **GatherReferences** - Finding referenced objects
- **Patch** - Updating fields on a pre-existing object

Each operation type has flags:
- **ReadBit** - Operation reads from RTTI fields
- **WriteBit** - Operation writes to RTTI fields
- **PreExistingObjectBit** - Operation targets a pre-existing object (not newly created)

~~~~~~~~~~~~~{.cpp}
void OnOperationStarted(MyComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
{
	if (operationType.IsSet(RTTIOperationType::ReadBit))
	{
		// Any read operation (Serialization, DeltaGenerate, GatherReferences)
	}

	if (operationType.IsSet(RTTIOperationType::WriteBit))
	{
		// Any write operation (Deserialization, DeltaApply, Patch)
	}

	if (operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
	{
		// Object already exists, don't initialize
	}
}
~~~~~~~~~~~~~

## Temporary data storage

During operation notifications and field getter/setter calls, your **RTTIType** instance is unique to the object being processed. You can use it for temporary data storage:

~~~~~~~~~~~~~{.cpp}
class TextureRTTI : public TRTTIType<Texture, Resource, TextureRTTI>
{
	// Temporary storage
	TShared<PixelData> mPixelData;

	UPtrRTTIIterator<TShared<PixelData>, false> GetPixelDataIterator(Texture& object, FrameAllocator& allocator)
	{
		return CreateRTTIIterator<TShared<PixelData>, false>(allocator, mPixelData);
	}

	const TShared<PixelData>& GetPixelDataValue(Texture& object, FrameAllocator& allocator, TRTTIIterator<TShared<PixelData>, false>& iterator)
	{
		// Read from actual texture
		mPixelData = object.GetPixelData();
		return mPixelData;
	}

	void SetPixelDataValue(Texture& obj, FrameAllocator& allocator, TRTTIIterator<TShared<PixelData>, false>& iterator, const TShared<PixelData>& value)
	{
		// Store in temporary field, not directly in texture
		mPixelData = value;
	}

	B3D_RTTI_BEGIN_MEMBERS
	B3D_RTTI_END_MEMBERS

public:
	TextureRTTI()
	{
		AddField<TextureRTTI, Texture, TShared<PixelData>>(
			"mPixelData", 0,
			&TextureRTTI::GetPixelDataIterator,
			&TextureRTTI::GetPixelDataValue,
			&TextureRTTI::SetPixelDataValue);
	}

	void OnOperationEnded(Texture& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		if (operationType.IsSet(RTTIOperationType::Deserialization))
		{
			// Initialize the texture now that all fields are deserialized
			object.Initialize();

			// Validate and write pixel data from our temporary field
			if (mPixelData && mPixelData->IsValid())
				object.WriteData(mPixelData);
		}
	}

	// ... GetRttiName, GetRttiId, NewRttiObject ...
};
~~~~~~~~~~~~~

# Versioning

RTTI provides robust versioning through field IDs. When you modify a class structure, follow these rules:

## Adding fields

Simply add new fields with new unique IDs:

~~~~~~~~~~~~~{.cpp}
// Version 1
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myInt, 0)
		B3D_RTTI_MEMBER(myFloat, 1)
	B3D_RTTI_END_MEMBERS
	// ...
};

// Version 2 - Added new field
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myInt, 0)
		B3D_RTTI_MEMBER(myFloat, 1)
		B3D_RTTI_MEMBER(myString, 2) // New field with new ID
	B3D_RTTI_END_MEMBERS
	// ...
};
~~~~~~~~~~~~~

When deserializing old data, `myString` will be initialized to its default value.

## Removing fields

Never reuse field IDs of removed fields:

~~~~~~~~~~~~~{.cpp}
// Version 1
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myInt, 0)
		B3D_RTTI_MEMBER(myFloat, 1)
		B3D_RTTI_MEMBER(deprecated, 2)
	B3D_RTTI_END_MEMBERS
	// ...
};

// Version 2 - Removed 'deprecated' field
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myInt, 0)
		B3D_RTTI_MEMBER(myFloat, 1)
		// ID 2 is retired, never reuse it
		B3D_RTTI_MEMBER(myNewField, 3) // Use new ID
	B3D_RTTI_END_MEMBERS
	// ...
};
~~~~~~~~~~~~~

When deserializing old data with the removed field, it will be ignored.

## Changing field types

If you change the type of a field, assign it a new ID:

~~~~~~~~~~~~~{.cpp}
// Version 1
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_MEMBER(myValue, 0) // u32
	B3D_RTTI_END_MEMBERS
	// ...
};

// Version 2 - Changed myValue from u32 to float
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	B3D_RTTI_BEGIN_MEMBERS
		// ID 0 is retired
		B3D_RTTI_MEMBER(myValue, 1) // Now float, new ID
	B3D_RTTI_END_MEMBERS
	// ...
};
~~~~~~~~~~~~~

## Custom migration

For complex migrations, use operation notifications:

~~~~~~~~~~~~~{.cpp}
class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	// Temporary field to read old data
	u32 mOldValue;

	B3D_RTTI_BEGIN_MEMBERS
		// Read old field into temporary storage
		B3D_RTTI_GENERATED_MEMBER(mOldValue, 0)
		// New field
		B3D_RTTI_MEMBER(myNewValue, 1)
	B3D_RTTI_END_MEMBERS

public:
	void OnOperationEnded(MyComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		if (operationType.IsSet(RTTIOperationType::Deserialization))
		{
			// Migrate old value to new format if it was present
			if (mOldValue != 0)
			{
				object.myNewValue = ConvertOldToNew(mOldValue);
			}
		}
	}

	// ... GetRttiName, GetRttiId, NewRttiObject ...
};
~~~~~~~~~~~~~

# Querying RTTI information

You can manually query class hierarchy and fields from the RTTI type object.

## RTTIType queries

@b3d::RTTIType queries:
- @b3d::RTTIType::GetBaseClass - Returns the **RTTIType** object of the base class
- @b3d::RTTIType::GetDerivedClasses - Returns a list of **RTTIType** objects for all derived classes
- @b3d::RTTIType::GetFieldCount - Returns the number of member fields
- @b3d::RTTIType::GetField - Returns field information from its sequential index (@b3d::RTTIField)
- @b3d::RTTIType::FindField - Searches for a field by name and returns **RTTIField**

~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

RTTIType* rttiType = myObject->GetRtti();

// Query type hierarchy
RTTIType* baseType = rttiType->GetBaseClass();
Vector<RTTIType*>& derivedTypes = rttiType->GetDerivedClasses();

// Query fields
u32 fieldCount = rttiType->GetFieldCount();
for (u32 i = 0; i < fieldCount; i++)
{
	RTTIField* field = rttiType->GetField(i);
	B3D_LOG(Info, LogGeneric, "Field: {0}", field->GetName());
}

// Find specific field
RTTIField* field = rttiType->FindField("myInt");
~~~~~~~~~~~~~

## RTTIField queries

@b3d::RTTIField queries:
- @b3d::RTTIField::IsPlainType - Checks if field contains plain data
- @b3d::RTTIField::IsReflectableType - Checks if field contains **IReflectable** data
- @b3d::RTTIField::IsReflectablePtrType - Checks if field contains **IReflectable** pointer
- @b3d::RTTIField::IsArray - Checks if the field contains an array or single value
- @b3d::RTTIField::GetFlags - Returns field flags (@b3d::RTTIFieldFlags)

~~~~~~~~~~~~~{.cpp}
RTTIField* field = rttiType->FindField("myInt");

if (field->IsPlainType())
{
	// Can cast to RTTIPlainFieldBase for plain operations
}

if (field->IsReflectableType())
{
	// Can cast to RTTIReflectableFieldBase
}

if (field->IsArray())
{
	// Field is a container
}

RTTIFieldFlags flags = field->GetFlags();
if (flags.IsSet(RTTIFieldFlag::WeakRef))
{
	// Field is a weak reference
}
~~~~~~~~~~~~~

## Global RTTI queries

Global helper functions:
- @b3d::B3DRTTIIsOfType - Checks if object is exactly type *T*
- @b3d::B3DRTTIIsSubclass - Checks if object is derived from type *T*
- @b3d::B3DRTTICreate - Creates new object from type ID
- @b3d::B3DRTTICast - Casts object to specified type (returns null if invalid)

~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

// Type checking
if (B3DRTTIIsOfType<Texture>(myObject))
{
	// Object is exactly a Texture
}

if (B3DRTTIIsSubclass<Resource>(myObject))
{
	// Object is a Resource or derived from Resource
}

// Create object from type ID
TShared<IReflectable> newObject = B3DRTTICreate(TID_Texture);

// Safe casting
Texture* texture = B3DRTTICast<Texture>(newObject);
if (texture)
{
	// Cast succeeded
}
~~~~~~~~~~~~~

## IReflectable queries

@b3d::IReflectable provides direct queries:
- @b3d::IReflectable::GetTypeName - Gets type name
- @b3d::IReflectable::GetTypeId - Gets type ID
- @b3d::IReflectable::GetRtti - Gets **RTTIType** object
- @b3d::IReflectable::IsDerivedFrom - Checks inheritance

~~~~~~~~~~~~~{.cpp}
IReflectable* myObject = ...;

String typeName = myObject->GetTypeName();
u32 typeId = myObject->GetTypeId();
RTTIType* rttiType = myObject->GetRtti();

if (myObject->IsDerivedFrom(Texture::GetRttiStatic()))
{
	// Object derives from Texture
}
~~~~~~~~~~~~~

# Data block fields

For raw binary data, use data block fields with @b3d::TRTTIType::AddDataBlockField. These fields are optimized for large binary blobs and provide streaming capabilities:

~~~~~~~~~~~~~{.cpp}
class MyComponent : public Component
{
public:
	Vector<u8> RawData;
	u32 DataSize;
	u32 DataOffset;
};

class MyComponentRTTI : public TRTTIType<MyComponent, Component, MyComponentRTTI>
{
	TShared<DataStream> GetRawData(MyComponent* object, u32& size)
	{
		size = (u32)object->RawData.size();
		return B3DMakeShared<MemoryDataStream>(object->RawData.data(), size);
	}

	void SetRawData(MyComponent* object, const TShared<DataStream>& data, u32 size)
	{
		obj->RawData.resize(size);
		data->Read(obj->RawData.data(), size);
	}

	B3D_RTTI_BEGIN_MEMBERS
	B3D_RTTI_END_MEMBERS

public:
	MyComponentRTTI()
	{
		AddDataBlockField("rawData", 0,
			&MyComponentRTTI::GetRawData,
			&MyComponentRTTI::SetRawData);
	}

	// ... GetRttiName, GetRttiId, NewRttiObject ...
};
~~~~~~~~~~~~~

The setter receives a @b3d::DataStream that can be read from or cloned for later use. When deserializing streaming resources (like audio clips), you can store the stream directly instead of reading all data immediately. The stream maintains its read position, allowing you to read incrementally or defer loading:

~~~~~~~~~~~~~{.cpp}
class AudioClipRTTI : public TRTTIType<AudioClip, Resource, AudioClipRTTI>
{
	void SetData(AudioClip* object, const TShared<DataStream>& stream, u32 size)
	{
		// Clone the stream to avoid modifying the deserializer's stream
		object->StreamData = stream->Clone();
		object->StreamSize = size;

		// Store the current position for later streaming
		object->StreamOffset = (u32)stream->Tell();

		// Data can now be read on-demand during playback
	}
};
~~~~~~~~~~~~~

This approach is particularly useful for resources that don't need all data in memory at once (streaming audio, texture mipmaps, large meshes). The cloned stream can be read from incrementally during runtime without blocking deserialization.

# ECS fragment fields

The RTTI system supports native serialization of ECS fragments through @b3d::TRTTIECSField. This field type allows RTTI to read and write ECS component data directly from the registry, without requiring the owning object to store a separate copy.

The field works with two kinds of ECS data:
 - **Data fragments** — Regular component structs stored per-entity (e.g., `ecs::Renderable`, `ecs::Light`)
 - **Tag groups** — Compact bitfield types that represent sets of boolean tags on an entity (e.g., mobility tags)

The distinction is detected at compile time — you use the same field type for both.

## Requirements

The owning RTTI type must implement the @b3d::ecs::IECSEntityOwner interface, which provides access to the ECS registry and entity handle:

~~~~~~~~~~~~~{.cpp}
class MyObject : public IReflectable, public ecs::IECSEntityOwner
{
	Registry* GetECSRegistry() const override { return mRegistry; }
	Entity GetECSEntity() const override { return mEntity; }
	void CreateECSEntity(Registry* registry) override
	{
		mRegistry = registry;
		mEntity = registry->CreateEntity();
		// Add default fragments here
	}

	// ... other members ...

private:
	Registry* mRegistry = nullptr;
	Entity mEntity;
};
~~~~~~~~~~~~~

## Registering ECS fields

Use the @B3D_RTTI_MEMBER_ECS macro in your RTTI type to register an ECS fragment field:

~~~~~~~~~~~~~{.cpp}
class MyObjectRTTI : public TRTTIType<MyObject, IReflectable, MyObjectRTTI>
{
	B3D_RTTI_MEMBER_ECS(Renderable, 0)
	B3D_RTTI_MEMBER_ECS(MobilityTags, 1)

	// ... standard RTTI boilerplate ...
};
~~~~~~~~~~~~~

The first parameter is the ECS fragment type (looked up in the `ecs` namespace), and the second is the unique field ID. The framework handles serialization and deserialization automatically — data fragments are serialized in full, while tag groups are serialized as a single integer bitfield.

## Entity creation during deserialization

ECS fragment fields write directly into the registry, so the entity must exist before the fields are deserialized. Your RTTI type must call @b3d::ecs::IECSEntityOwner::CreateECSEntity in its `OnOperationStarted` override for the deserialization operation. This ensures the entity and its default fragments are ready before the deserializer writes field data into them.

This is the same pattern used by `SceneObjectRTTI`, which creates the ECS entity early so that fragment fields can be populated during deserialization:

~~~~~~~~~~~~~{.cpp}
class MyObjectRTTI : public TRTTIType<MyObject, IReflectable, MyObjectRTTI>
{
	B3D_RTTI_MEMBER_ECS(Renderable, 0)
	B3D_RTTI_MEMBER_ECS(MobilityTags, 1)

	void OnOperationStarted(MyObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
	{
		if(operationType.IsSet(RTTIOperationType::Deserialize))
		{
			if(auto* engineContext = context.As<RTTIOperationEngineContext>())
			{
				// Create the ECS entity before fragment fields are deserialized
				if(engineContext->GameObjectCollection != nullptr)
				{
					ecs::Registry& registry = engineContext->GameObjectCollection->GetECSRegistry();
					object.CreateECSEntity(&registry);
				}
			}
		}
	}

	// ... standard RTTI boilerplate ...
};
~~~~~~~~~~~~~

If the entity is not created before deserialization, the ECS fragment fields will have no valid entity to write to and deserialization will fail.
