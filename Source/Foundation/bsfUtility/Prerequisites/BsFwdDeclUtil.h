//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Contains forward declares used as a convenience

namespace bs
{
// Script binding defines

/**
 * @page scriptBindingMacro Script binding exports
 *
 * Marks the specific type or a method to be exported to the scripting API. Supports a variety of options which can
 * be specified in the "option:value" format, where multiple options are separated by commas, with no whitespace.
 *
 * Supported options:
 *  - n - Specify a different name for the type in the scripting API (e.g. "n:MyName"). Usable on types and methods.
 *  - v - Specify a different visibility (default is public). Supported values are "public", "internal" and "private".
 *		  Usable on types and methods.
 *  - f - Specify the name of the output file(s) for the script object and its potential wrappers. If not specified
 *		  the name of the type will be used for the file. Usable on types only.
 *	- pl - Specify whether the type is plain or not (default is false). Supported values are "true" or "false". Plain
 *		  types don't have script interop objects generated, instead they are generated in script code as plain data
 *		  types. No methods are exposed, but all data members and constructors are copied. Usable on types only.
 *	- e - Specify that a method is external and is to be appended to some script class. Such methods must be static
 *		  and as the first parameter accept the instance of the class they operate on. Value of this option should be
 *		  the name of the class to attach this method to. Methods with this parameter must also be part of a class
 *		  with this option. Usable on types and methods.
 *	- ec - Similar to "e", but specifies an external constructor. Such method must have a return value that returns
 *		   an instance of the class its registered for. Value of this option should be the name of the class to attach
 *		   this method to. Methods with this parameter must also be part of a class with the "e" option. Usable on methods
 *		   only.
 *	- pr - Specify the method should be exported as a property in script code. Supported values are "getter" or "setter".
 *		  Getter methods must return a single value and accept no parameters, while setter methods must accept one
 *		  parameter and return no values. Usable on methods only.
 *	- ed - Specify that a type should be exported for use in the editor only. Supported values are "true" or "false".
 *		   Usable on types only.
 *  - ex - Excludes an enum or struct member from being generated in script code. Supported values are "true" or "false".
 *		   By default all struct & enum members are exported.
 *  - in - When enabled ensures only the interop C# method is generated, but not a public one. It is instead expected
 *		   the user will manually implement the public method. Supported values are "true" or "false". Default is "false".
 *		   Only supported on methods.
 *  - m  - Specifies the name of the module to place the entry in. This determines the documentation group, and may also
 *		   determine namespace and/or module (e.g. m:Animation to place it in the Animation module). Usable on types.
 */

#if BS_COMPILER == BS_COMPILER_CLANG
	/** @ref scriptBindingMacro */
	#define BS_SCRIPT_EXPORT(...) __attribute__((annotate("se," #__VA_ARGS__)))

	/**
	 * When applied to a parameter, makes it a variable argument parameter in the scripting interface (if supported
	 * by the scripting language.
	 */
	#define BS_PARAMS __attribute__((annotate("params")))

	/**
	 * When applied to a parameter or a field of ResourceHandle type, makes that element be exported as a raw resource in
	 * script code.
	 */
	#define BS_NORREF __attribute__((annotate("norref")))
#else
	/** @ref scriptBindingMacro */
	#define BS_SCRIPT_EXPORT(...)

	/**
	 * When applied to a parameter, makes it a variable argument parameter in the scripting interface (if supported
	 * by the scripting language).
	 */
	#define BS_PARAMS

	/**
	 * When applied to a parameter or a field of ResourceHandle type, makes that element be exported as a raw resource in
	 * script code.
	 */
	#define BS_NORREF
#endif

	/** @addtogroup Math
	 *  @{
	 */

	/** Values that represent in which order are euler angles applied when used in transformations. */
	enum class BS_SCRIPT_EXPORT() EulerAngleOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	/** Enum used for object construction specifying the object should be zero initialized. */
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

	template<class T>
	struct RTTIPlainType;

	enum TypeID_Utility
	{
		TID_Bool = 0,
		TID_Int32 = 1,
		TID_UInt32 = 2,
		// First 16 entries reserved for builtin types
		
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
		TID_IReflectable = 69,
		TID_DataBlob = 70,
		TID_ColorGradient = 71,
		TID_SerializationContext = 72,
		TID_List = 73,
		TID_SmallVector = 74,
		TID_ColorGradientHDR = 75,
		TID_RTTISchema = 76,
		TID_RTTIFieldSchema = 77,
		TID_BitLength = 78,
		TID_RTTIFieldInfo = 79
	};
}
