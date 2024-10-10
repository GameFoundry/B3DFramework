//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed game object or resource.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoReference : ManagedTypeInfo
	{
		private ManagedTypeInfoReference(bool __dummy0) { }
		protected ManagedTypeInfoReference() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedReferenceType ReferenceType
		{
			get { return Internal_GetReferenceType(mCachedPtr); }
			set { Internal_SetReferenceType(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int TypeRTTIId
		{
			get { return Internal_GetTypeRTTIId(mCachedPtr); }
			set { Internal_SetTypeRTTIId(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public string TypeNamespace
		{
			get { return Internal_GetTypeNamespace(mCachedPtr); }
			set { Internal_SetTypeNamespace(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public string TypeName
		{
			get { return Internal_GetTypeName(mCachedPtr); }
			set { Internal_SetTypeName(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedReferenceType Internal_GetReferenceType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetReferenceType(IntPtr thisPtr, ManagedReferenceType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetTypeRTTIId(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTypeRTTIId(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetTypeNamespace(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTypeNamespace(IntPtr thisPtr, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetTypeName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTypeName(IntPtr thisPtr, string value);
	}
}
