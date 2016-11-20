//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

// Contains forward declares used as a convenience

namespace bs 
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Values that represent in which order are euler angles applied when used in transformations. */
	enum class EulerAngleOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	/** Enum used for object construction specifying the object should be zero initializes. */
	enum BS_ZERO { BsZero };

	/** Enum used for matrix/quaternion constructor specifying it should be initialized with an identity value. */
	enum BS_IDENTITY { BsIdentity };

	/** @} */

	/** @addtogroup Platform-Utility
	 *  @{
	 */

	/**	Enum that defines possible window border styles. */
	enum class WindowBorder
	{
		Normal,
		None,
		Fixed
	};

	/** @} */

	class Angle;
	class AABox;
	class Degree;
	class Math;
	class Matrix3;
	class Matrix4;
	class Plane;
	class Quaternion;
	class Radian;
	class Ray;
	class Capsule;
	class Sphere;
	class Vector2;
	class Vector3;
	class Vector4;
	struct Vector2I;
	class Rect2I;
	class Rect2;
	class Rect3;
	class Color;
	class DynLib;
	class DynLibManager;
	class DataStream;
	class MemoryDataStream;
	class FileDataStream;
	class MeshData;
	class FileSystem;
	class Timer;
	class Task;
	class GpuResourceData;
	class PixelData;
	class HString;
	class StringTable;
	struct LocalizedStringData;
	class Path;
	class HThread;
	class TestSuite;
	class TestOutput;
	class AsyncOpSyncData;
	struct RTTIField;
	struct RTTIReflectablePtrFieldBase;
	struct SerializedObject;
	struct SerializedInstance;
	class FrameAlloc;
	class LogEntry;
	// Reflection
	class IReflectable;
	class RTTITypeBase;
	// Serialization
	class ISerializable;
	class SerializableType;

	typedef List<SPtr<DataStream>> DataStreamList;
	typedef SPtr<DataStreamList> DataStreamListPtr;

	enum TypeID_Utility
	{
		TID_Abstract = 50, // Special type ID used for Abstract classes. Only type ID that may be used by more than one class.
		TID_WString = 51,
		TID_Path = 52,
		TID_Vector = 53,
		TID_Map = 54,
		TID_UnorderedMap = 55,
		TID_Pair = 56,
		TID_Set = 57,
		TID_StringID = 58,
		TID_SerializedInstance = 59,
		TID_SerializedField = 60,
		TID_SerializedObject = 61,
		TID_SerializedArray = 62,
		TID_SerializedEntry = 63,
		TID_SerializedArrayEntry = 64,
		TID_SerializedSubObject = 65,
		TID_UnorderedSet = 66,
		TID_SerializedDataBlock = 67,
		TID_Flags = 68,
		TID_IReflectable = 69
	};
}
