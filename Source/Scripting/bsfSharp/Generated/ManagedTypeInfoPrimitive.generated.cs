//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed primitive (for example int, float, etc.).</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoPrimitive : ManagedTypeInfo
	{
		private ManagedTypeInfoPrimitive(bool __dummy0) { }
		protected ManagedTypeInfoPrimitive() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedPrimitiveType PrimitiveType
		{
			get { return Internal_GetPrimitiveType(mCachedPtr); }
			set { Internal_SetPrimitiveType(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedPrimitiveType Internal_GetPrimitiveType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPrimitiveType(IntPtr thisPtr, ManagedPrimitiveType value);
	}
}
