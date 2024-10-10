//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed List.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoList : ManagedTypeInfo
	{
		private ManagedTypeInfoList(bool __dummy0) { }
		protected ManagedTypeInfoList() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfo ElementType
		{
			get { return Internal_GetElementType(mCachedPtr); }
			set { Internal_SetElementType(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetElementType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetElementType(IntPtr thisPtr, ManagedTypeInfo value);
	}
}
