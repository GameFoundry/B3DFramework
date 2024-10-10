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
		/// Deduces information about the object&apos;s type. Object must be serializable in order for type information to be 
		/// present.
		/// </summary>
		public static ManagedTypeInfo GetTypeInfo(object scriptObject)
		{
			return Internal_GetTypeInfo(scriptObject);
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
		private static extern ManagedTypeInfo Internal_GetTypeInfo(object scriptObject);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CloneObject(object original);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern object Internal_CreateObjectOfType(Type type);
	}
}
