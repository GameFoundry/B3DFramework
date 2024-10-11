//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>
	/// Contains data about serializable fields of a managed object, and the object&apos;s class hierarchy if it belongs to 
	/// one. All public fields are by default serializable if their type support serialization. Type is serializable if it has 
	/// the SerializeObject attribute, or is one of the built-in supported serializable types (primitive such as int or bool, 
	/// game object, resource or resource reference). Array/List/Dictionary of serializable types is also considered 
	/// serializable. Public field can be made non-serializable via the DontSerializeField attribute. 
	/// Private/protected/internal field can be made serializable by specifying the SerializeField attribute.
	/// </summary>
	[ShowInInspector]
	public partial class ManagedObjectInfo : ScriptObject
	{
		private ManagedObjectInfo(bool __dummy0) { }
		protected ManagedObjectInfo() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfoObject TypeInfo
		{
			get { return Internal_GetTypeInfo(mCachedPtr); }
			set { Internal_SetTypeInfo(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ManagedMemberInfo[] Members
		{
			get { return Internal_GetMembers(mCachedPtr); }
			set { Internal_SetMembers(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ManagedObjectInfo BaseClass
		{
			get { return Internal_GetBaseClass(mCachedPtr); }
			set { Internal_SetBaseClass(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfoObject Internal_GetTypeInfo(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTypeInfo(IntPtr thisPtr, ManagedTypeInfoObject value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedMemberInfo[] Internal_GetMembers(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMembers(IntPtr thisPtr, ManagedMemberInfo[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedObjectInfo Internal_GetBaseClass(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetBaseClass(IntPtr thisPtr, ManagedObjectInfo value);
	}
}
