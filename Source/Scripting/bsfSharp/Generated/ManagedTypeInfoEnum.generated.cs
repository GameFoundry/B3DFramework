//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed enum.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoEnum : ManagedTypeInfo
	{
		private ManagedTypeInfoEnum(bool __dummy0) { }
		protected ManagedTypeInfoEnum() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedPrimitiveType UnderlyingType
		{
			get { return Internal_GetUnderlyingType(mCachedPtr); }
			set { Internal_SetUnderlyingType(mCachedPtr, value); }
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
		private static extern ManagedPrimitiveType Internal_GetUnderlyingType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUnderlyingType(IntPtr thisPtr, ManagedPrimitiveType value);
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
