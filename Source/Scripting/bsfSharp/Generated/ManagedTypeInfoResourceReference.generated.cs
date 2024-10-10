//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a reference to a resource.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfoResourceReference : ManagedTypeInfo
	{
		private ManagedTypeInfoResourceReference(bool __dummy0) { }
		protected ManagedTypeInfoResourceReference() { }

		[ShowInInspector]
		[NativeWrapper]
		public ManagedTypeInfo ResourceType
		{
			get { return Internal_GetResourceType(mCachedPtr); }
			set { Internal_SetResourceType(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ManagedTypeInfo Internal_GetResourceType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetResourceType(IntPtr thisPtr, ManagedTypeInfo value);
	}
}
