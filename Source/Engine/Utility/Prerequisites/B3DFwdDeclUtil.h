//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Contains forward declares used as a convenience

namespace b3d
{
	// Script binding defines

	/**
	 * @page scriptBindingMacro Script binding exports
	 *
	 * Marks the specific type or a method to be exported to the scripting API. Supports a variety of options which can
	 * be specified in the "ParameterName(Value)" format, where multiple options are separated by commas.
	 *
	 * Supported options:
	 *  - ExportName(Name) - Specify a different name for the type in the scripting API. Usable on types and methods.
	 *  - Visibility(Public|Internal|Private) - Specify a different visibility (default is Public). Usable on types and methods.
	 *  - ExportFile(Name) - Specify the name of the output file(s) for the script object and its potential wrappers. If not
	 *		  specified the name of the type will be used for the file. Usable on types only.
	 *  - ExportAsStruct(true|false) - Specify whether the type should be exported as a plain struct (default is false). Plain
	 *		  types don't have script interop objects generated, instead they are generated in script code as plain data
	 *		  types. No methods are exposed, but all data members and constructors are copied. Usable on types only.
	 *  - ExtensionMethodForType(TypeName) - Specify that a method is external and is to be appended to the named script class.
	 *		  Such methods must be static and as the first parameter accept the instance of the class they operate on.
	 *		  Usable on methods. Methods with this parameter must also be part of a class with ExtensionClassForType.
	 *  - ExtensionConstructorForType(TypeName) - Similar to ExtensionMethodForType, but specifies an external constructor.
	 *		  Such method must have a return value that returns an instance of the class it is registered for. Usable on methods only.
	 *  - ExtensionClassForType(TypeName) - Marks a class as containing extension methods/constructors for the named type.
	 *		  Usable on types only.
	 *  - Property(Getter|Setter) - Specify the method should be exported as a property in script code. Getter methods must
	 *		  return a single value and accept no parameters, while setter methods must accept one parameter and return no values.
	 *		  Usable on methods only.
	 *  - API(Framework|Engine|Editor) - Specify which assembly to export to. Multiple API parameters can be specified. Usable on types only.
	 *  - Exclude(true|false) - Excludes an enum or struct member from being generated in script code. By default all
	 *		  struct & enum members are exported.
	 *  - InteropOnly(true|false) - When enabled ensures only the interop C# method is generated, but not a public one.
	 *		  It is instead expected the user will manually implement the public method. Default is false. Only supported on methods.
	 *  - DocumentationGroup(Name) - Specifies the documentation group/module to place the entry in. Usable on types.
	 *  - Singleton(GetterName) - Marks a class as a singleton with the specified getter function. Usable on types only.
	 *  - PassByCopy(true|false) - Pass struct by copy instead of by reference when crossing the C++/C# boundary.
	 *  - LoadOnAssign(true|false) - Automatically load a resource when it is assigned. Usable on fields.
	 *  - ApplyOnDirty(true|false) - Apply changes when the property is marked dirty. Usable on properties.
	 *  - NotNullable(true|false) - Mark a parameter or field as non-nullable.
	 *  - UI(Hide|Show|AsSlider|AsLayerMask|IsHDRColor|AsQuaternion|Inline) - Inspector UI hints for the field.
	 *  - UIValueRange([min, max]) - Clamp value to a range in the inspector.
	 *  - UIIncrementStep(value) - Inspector increment step size.
	 *  - UIOrder(value) - Inspector display order.
	 *  - UICategory(name) - Inspector category grouping.
	 */

#if B3D_COMPILER_CLANG
/** @ref scriptBindingMacro */
#	define B3D_SCRIPT_EXPORT(...) __attribute__((annotate("se," #	__VA_ARGS__)))

/**
 * When applied to a parameter, makes it a variable argument parameter in the scripting interface (if supported
 * by the scripting language.
 */
#	define B3D_PARAMS __attribute__((annotate("params")))

/**
 * When applied to a parameter or a field of ResourceHandle type, makes that element be exported as a raw resource in
 * script code.
 */
#	define B3D_NO_RREF __attribute__((annotate("norref")))
#else
/** @ref scriptBindingMacro */
#	define B3D_SCRIPT_EXPORT(...)

/**
 * When applied to a parameter, makes it a variable argument parameter in the scripting interface (if supported
 * by the scripting language).
 */
#	define B3D_PARAMS

/**
 * When applied to a parameter or a field of ResourceHandle type, makes that element be exported as a raw resource in
 * script code.
 */
#	define B3D_NO_RREF
#endif

	/** @addtogroup Math
	 *  @{
	 */

	/** Values that represent in which order are euler angles applied when used in transformations. */
	enum class B3D_SCRIPT_EXPORT() EulerAngleOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	/** Enum used for object construction specifying the object should be zero initialized. */
	enum ZeroTag
	{
		kZeroTag
	};

	/** Enum used for matrix/quaternion constructor specifying it should be initialized with an identity value. */
	enum IdentityTag
	{
		kIdentityTag
	};

	template<typename T> struct TVector3I;
	using Vector3I = TVector3I<i32>;
	using Vector3UI = TVector3I<u32>;

	template<typename T> struct TVector4I;
	using Vector4I = TVector4I<i32>;
	using Vector4UI = TVector4I<u32>;

	template<typename T> struct TVector2;
	using Vector2 = TVector2<float>;
	using Vector2F = TVector2<float>;
	using Vector2D = TVector2<double>;
	using Vector2I = TVector2<i32>;
	using Vector2UI = TVector2<u32>;

	template<typename T> struct TVector3;
	using Vector3 = TVector3<float>;
	using Vector3F = TVector3<float>;
	using Vector3D = TVector3<double>;

	template<typename T> struct TVector4;
	using Vector4 = TVector4<float>;
	using Vector4F = TVector4<float>;
	using Vector4D = TVector4<double>;

	template<typename T> struct TMatrix4;
	using Matrix4 = TMatrix4<float>;
	using Matrix4F = TMatrix4<float>;
	using Matrix4D = TMatrix4<double>;

	template<typename T> struct TMatrix3;
	using Matrix3 = TMatrix3<float>;
	using Matrix3F = TMatrix3<float>;
	using Matrix3D = TMatrix3<double>;

	template<typename PositionType, typename SizeType> struct TArea2;
	using Area2 = TArea2<float, float>;
	using Area2I = TArea2<i32, u32>;

	template<typename T> struct TQuaternion;
	using Quaternion = TQuaternion<float>;
	using QuaternionF = TQuaternion<float>;
	using QuaternionD = TQuaternion<double>;

	template<typename T> class TRadian;
	using Radian = TRadian<float>;
	using RadianF = TRadian<float>;
	using RadianD = TRadian<double>;

	template<typename T> class TDegree;
	using Degree = TDegree<float>;
	using DegreeF = TDegree<float>;
	using DegreeD = TDegree<double>;

	template<typename T> struct TPlane;
	using Plane = TPlane<float>;
	using PlaneF = TPlane<float>;
	using PlaneD = TPlane<double>;

	template<typename T> struct TAABox;
	using AABox = TAABox<float>;
	using AABoxF = TAABox<float>;
	using AABoxD = TAABox<double>;

	template<typename T> struct TSphere;
	using Sphere = TSphere<float>;
	using SphereF = TSphere<float>;
	using SphereD = TSphere<double>;

	template<typename T> struct TRay;
	using Ray = TRay<float>;
	using RayF = TRay<float>;
	using RayD = TRay<double>;

	template<typename T> struct TBounds;
	using Bounds = TBounds<float>;
	using BoundsF = TBounds<float>;
	using BoundsD = TBounds<double>;

	template<typename T> struct TCapsule;
	using Capsule = TCapsule<float>;
	using CapsuleF = TCapsule<float>;
	using CapsuleD = TCapsule<double>;

	template<typename T> struct TLineSegment3;
	using LineSegment3 = TLineSegment3<float>;
	using LineSegment3F = TLineSegment3<float>;
	using LineSegment3D = TLineSegment3<double>;


	template<typename T> struct TConvexVolume;
	using ConvexVolume = TConvexVolume<float>;
	using ConvexVolumeF = TConvexVolume<float>;
	using ConvexVolumeD = TConvexVolume<double>;

	/** @} */

	template<typename T> class TTransform;
	using Transform = TTransform<float>;
	using TransformF = TTransform<float>;
	using TransformD = TTransform<double>;

	class Math;
	class Rect3;
	class Color;
	class CommandLine;
	class DynamicLibrary;
	class DynamicLibraryManager;
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
	class PooledThread;
	class TestSuite;
	class TestOutput;
	class AsyncOpSyncData;
	struct RTTIField;
	struct RTTIReflectablePtrFieldBase;
	struct SerializedObject;
	struct ISerialized;
	class FrameAllocator;
	class LogEntry;
	// Reflection
	class IReflectable;
	class RTTIType;
	// Serialization
	class ISerializable;
	class SerializableType;

	template <class T>
	struct RTTIPlainType;

	enum TypeID_Utility
	{
		TID_Bool = 0,
		TID_Int32 = 1,
		TID_UInt32 = 2,
		TID_Float = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_Double = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_Int8 = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_UInt8 = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_Int16 = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_UInt16 = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_Int64 = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		TID_UInt64 = 0, // TODO - Keeping ID 0 for compatibility. This should be updated with a proper id.
		// First 16 entries reserved for builtin types

		TID_String = 20,
		TID_Abstract = 50, // Special type ID used for Abstract classes. Only type ID that may be used by more than one class.
		TID_WString = 51,
		TID_Path = 52,
		TID_Vector = 53,
		TID_Map = 54,
		TID_UnorderedMap = 55,
		TID_Pair = 56,
		TID_Set = 57,
		TID_StringID = 58,
		TID_ISerialized = 59,
		TID_SerializedPlainData = 60,
		TID_SerializedObject = 61,
		TID_SerializedArray = 62,
		TID_SerializedField = 63,
		TID_SerializedArrayEntryDelta = 64,
		TID_SerializedSubObject = 65,
		TID_UnorderedSet = 66,
		TID_SerializedDataBlock = 67,
		TID_Flags = 68,
		TID_IReflectable = 69,
		TID_DataBlob = 70,
		TID_ColorGradient = 71,
		TID_RTTIOperationContext = 72,
		TID_List = 73,
		TID_TInlineArray = 74,
		TID_ColorGradientHDR = 75,
		TID_RTTISchema = 76,
		TID_RTTIFieldSchema = 77,
		TID_BitLength = 78,
		TID_RTTIFieldInfo = 79,
		TID_Optional = 80,
		TID_TArray = 81,
		TID_Bitfield = 82,
		TID_RTTIFieldDataTypeSchema = 83,
		TID_SerializedTuple = 84,
		TID_SerializedMap = 85,
		TID_Size2 = 86,
		TID_Size2UI = 87,
		TID_Array = 88,
		TID_SerializedTupleDelta = 89,
		TID_SerializedTupleEntryDelta = 90,
		TID_SerializedArrayDelta = 91,
		TID_SerializedMapDelta = 92,
		TID_SerializedMapEntryDelta = 93,
		TID_U32String = 94,
		TID_Size2I = 95,
		TID_AABox = 96,
		TID_Bounds = 97,
		TID_Degree = 98,
		TID_Radian = 99,
		TID_Matrix3 = 100,
		TID_Matrix4 = 101,
		TID_Quaternion = 102,
		TID_Plane = 103,
		TID_Area2 = 104,
		TID_Sphere = 105,
		TID_Vector2 = 106,
		TID_Vector2I = 107,
		TID_Vector3 = 108,
		TID_Vector3I = 109,
		TID_Vector4 = 110,
		TID_Vector4I = 111,
		TID_CharRange = 112,
		TID_RenderTargetBlendStateInformation = 113,
		TID_LightProbeSHCoefficient = 114,
		TID_SubMesh = 115,
		TID_IndexType = 116,
		TID_MorphVertex = 117,
		TID_NVGVertex = 118,
		TID_SamplerStateInformation = 119,
		TID_LocalizedStringParameterOffset = 120,
		TID_VertexElement = 121,
		TID_Color = 122,
		TID_RectOffset = 123,
		TID_UUID = 124,
		TID_SettingsPrimitivevalue = 125,
		TID_SettingsKeyInfo = 126,

		TID_Transform = 1139, // Moved from TypeID_Core, ID preserved for compatibility
	};
} // namespace b3d
