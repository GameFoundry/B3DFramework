//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Provides various utility methods for reflection-like operations on managed objects.</summary>
	[ShowInInspector]
	public partial class ManagedTypeUtility : ScriptObject
	{
		private ManagedTypeUtility(bool __dummy0) { }
		protected ManagedTypeUtility() { }

		/// <summary>
		/// Retrieves information about the object&apos;s type. Object must be a primitive type, scene object, component, 
		/// resource, resource reference type, or a custom type marked with SerializeObject attribute in order for type 
		/// information to be present. Type information can also be retrieved for arrays, lists or dictionaries of the above 
		/// types.
		/// </summary>
		public static ManagedTypeInfo GetTypeInfo(Type objectType)
		{
			return Internal_GetTypeInfo(objectType);
		}

		/// <summary>
		/// Retrieves detailed information about a serializable object. Provided object&apos;s type must be marked with a 
		/// SerializeObject attribute for this information to be available.
		/// </summary>
		public static ManagedObjectInfo GetSerializableObjectInfo(Type objectType)
		{
			return Internal_GetSerializableObjectInfo(objectType);
		}

		/// <summary>
		/// Deduces the RTTI ID of the native object that is wrapped by the provided object type. Returns ~0u for types that do 
		/// not wrap native types.
		/// </summary>
		public static int GetRTTITypeId(Type objectType)
		{
			return Internal_GetRTTITypeId(objectType);
		}

		/// <summary>Creates a new instance of a serialized object of the provided type.</summary>
		public static object CreateSerializableObject(ManagedTypeInfoObject typeInfo)
		{
			return Internal_CreateSerializableObject(typeInfo);
		}

		/// <summary>
		/// Creates a new instance of an array of the provided type and size. Size array must have an element for each rank of 
		/// the array type.
		/// </summary>
		public static object CreateArray(ManagedTypeInfoArray typeInfo, int[] arraySizes)
		{
			return Internal_CreateArray(typeInfo, arraySizes);
		}

		/// <summary>Creates a new instance of a list of the provided type with the provided initial capacity.</summary>
		public static object CreateList(ManagedTypeInfoList typeInfo, int size)
		{
			return Internal_CreateList(typeInfo, size);
		}

		/// <summary>Creates a new instance of a dictionary of the provided type.</summary>
		public static object CreateDictionary(ManagedTypeInfoDictionary typeInfo)
		{
			return Internal_CreateDictionary(typeInfo);
		}

		/// <summary>
		/// Clones the specified object. Non-serializable types and fields are ignored in clone. A deep copy is performed on all 
		/// serializable elements except for resources or game objects.
		/// </summary>
		/// <param name="original">Non-null reference to the object to clone. Object type must be serializable.</param>
		/// <returns>Deep copy of the original object</returns>
		public static object CloneObject(object original)
		{
			return Internal_CloneObject(original);
		}

		/// <summary>Creates an uninitialized object of the specified type.</summary>
		/// <param name="type">Type of the object to create. Must be serializable</param>
		/// <returns>New instance of the specified type, or null if the type is not serializable.</returns>
		public static object CreateObjectOfType(Type type)
		{
			return Internal_CreateObjectOfType(type);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetTypeInfo(Type objectType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedObjectInfo Internal_GetSerializableObjectInfo(Type objectType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetRTTITypeId(Type objectType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CreateSerializableObject(ManagedTypeInfoObject typeInfo);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CreateArray(ManagedTypeInfoArray typeInfo, int[] arraySizes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CreateList(ManagedTypeInfoList typeInfo, int size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CreateDictionary(ManagedTypeInfoDictionary typeInfo);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CloneObject(object original);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CreateObjectOfType(Type type);
	}
}
