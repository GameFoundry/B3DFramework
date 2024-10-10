//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed Array.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoArray : ManagedTypeInfo
	{
		private ManagedTypeInfoArray(bool __dummy0) { }
		protected ManagedTypeInfoArray() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfo ElementType
		{
			get { return Internal_GetElementType(mCachedPtr); }
			set { Internal_SetElementType(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int Rank
		{
			get { return Internal_GetRank(mCachedPtr); }
			set { Internal_SetRank(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetElementType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetElementType(IntPtr thisPtr, ManagedTypeInfo value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetRank(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRank(IntPtr thisPtr, int value);
	}
}
