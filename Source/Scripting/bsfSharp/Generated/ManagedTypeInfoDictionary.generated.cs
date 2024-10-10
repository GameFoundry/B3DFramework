//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed Dictionary.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoDictionary : ManagedTypeInfo
	{
		private ManagedTypeInfoDictionary(bool __dummy0) { }
		protected ManagedTypeInfoDictionary() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfo KeyType
		{
			get { return Internal_GetKeyType(mCachedPtr); }
			set { Internal_SetKeyType(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfo ValueType
		{
			get { return Internal_GetValueType(mCachedPtr); }
			set { Internal_SetValueType(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetKeyType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetKeyType(IntPtr thisPtr, ManagedTypeInfo value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetValueType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetValueType(IntPtr thisPtr, ManagedTypeInfo value);
	}
}
