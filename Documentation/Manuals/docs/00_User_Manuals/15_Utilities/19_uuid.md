---
title: UUID
---

@b3d::UUID represents a universally unique identifier - a 128-bit value that is (practically) guaranteed to be unique across space and time. UUIDs are commonly used for identifying resources, entities, and assets in a way that doesn't require central coordination.

# Creating UUIDs

## Generating random UUIDs

Use @b3d::UUIDGenerator::GenerateRandom to create a new random UUID:

~~~~~~~~~~~~~{.cpp}
UUID resourceId = UUIDGenerator::GenerateRandom();
UUID entityId = UUIDGenerator::GenerateRandom();
UUID sessionId = UUIDGenerator::GenerateRandom();

B3D_LOG(Info, LogGeneric, "Resource ID: {0}", resourceId.ToString());
~~~~~~~~~~~~~

Random UUIDs are generated using cryptographically secure random number generation, ensuring uniqueness even across different machines and processes.

## Creating from components

You can create a UUID from four 32-bit components:

~~~~~~~~~~~~~{.cpp}
UUID customId(0x12345678, 0x90ABCDEF, 0xFEDCBA09, 0x87654321);
~~~~~~~~~~~~~

## Creating from string

Parse a UUID from its string representation:

~~~~~~~~~~~~~{.cpp}
String uuidString = "550e8400-e29b-41d4-a716-446655440000";
UUID parsedId(uuidString);

if(!parsedId.Empty())
{
	// Successfully parsed
	B3D_LOG(Info, LogGeneric, "Parsed UUID: {0}", parsedId.ToString());
}
~~~~~~~~~~~~~

## Empty UUID

An empty (zero) UUID can be used to represent an invalid or uninitialized identifier:

~~~~~~~~~~~~~{.cpp}
UUID emptyId; // Default constructor creates empty UUID
UUID emptyId2 = UUID::kEmpty; // Explicit empty UUID constant

if(emptyId.Empty())
{
	B3D_LOG(Warning, LogGeneric, "UUID is not initialized");
}
~~~~~~~~~~~~~

# Converting to string

Convert a UUID to its string representation using @b3d::UUID::ToString:

~~~~~~~~~~~~~{.cpp}
UUID resourceId = UUIDGenerator::GenerateRandom();
String uuidString = resourceId.ToString();

// Example output: "550e8400-e29b-41d4-a716-446655440000"
B3D_LOG(Info, LogGeneric, "UUID: {0}", uuidString);
~~~~~~~~~~~~~

# Comparison

UUIDs support all standard comparison operators:

~~~~~~~~~~~~~{.cpp}
UUID id1 = UUIDGenerator::GenerateRandom();
UUID id2 = UUIDGenerator::GenerateRandom();
UUID id3 = id1;

// Equality
bool areEqual = (id1 == id3); // true
bool areDifferent = (id1 != id2); // true

// Ordering (for use in sorted containers)
bool isLess = (id1 < id2);
~~~~~~~~~~~~~

# Using UUIDs in containers

UUIDs can be used as keys in maps and sets:

~~~~~~~~~~~~~{.cpp}
// Map resources by UUID
UnorderedMap<UUID, HTexture> textureCache;

UUID textureId = UUIDGenerator::GenerateRandom();
HTexture texture = GetResources().Load<Texture>("MyTexture.png");
textureCache[textureId] = texture;

// Lookup by UUID
auto iterator = textureCache.find(textureId);
if(iterator != textureCache.end())
{
	HTexture foundTexture = iterator->second;
}

// Ordered set of UUIDs
Set<UUID> activeEntities;
activeEntities.insert(UUIDGenerator::GenerateRandom());
activeEntities.insert(UUIDGenerator::GenerateRandom());
~~~~~~~~~~~~~

# Serialization

UUIDs can be serialized to binary or text formats for storage and transmission:

~~~~~~~~~~~~~{.cpp}
// Save UUID to file
UUID resourceId = UUIDGenerator::GenerateRandom();
String uuidString = resourceId.ToString();

FileStream file("resource_id.txt", FileMode::Write);
file.Write(uuidString);
file.Close();

// Load UUID from file
FileStream loadFile("resource_id.txt", FileMode::Read);
String loadedString;
loadFile.Read(loadedString);
loadFile.Close();

UUID loadedId(loadedString);
~~~~~~~~~~~~~