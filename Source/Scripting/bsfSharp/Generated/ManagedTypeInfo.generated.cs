//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Contains information about a type of a managed object.</summary>
	[ShowInInspector]
	public partial class ManagedTypeInfo : ScriptObject
	{
		private ManagedTypeInfo(bool __dummy0) { }
		protected ManagedTypeInfo() { }

		/// <summary>Checks if the current type matches the provided type.</summary>
		public bool Matches(ManagedTypeInfo typeInfo)
		{
			return Internal_Matches(mCachedPtr, typeInfo);
		}

		/// <summary>Checks does the managed type this object represents still exists.</summary>
		public bool IsTypeLoaded()
		{
			return Internal_IsTypeLoaded(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_Matches(IntPtr thisPtr, ManagedTypeInfo typeInfo);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsTypeLoaded(IntPtr thisPtr);
	}
}
