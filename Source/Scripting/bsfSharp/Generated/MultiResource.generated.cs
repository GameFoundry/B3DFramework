//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>Contains a group of resources imported from a single source file.</summary>
	[ShowInInspector]
	public partial class MultiResource : ScriptObject
	{
		private MultiResource(bool __dummy0) { }

		public MultiResource()
		{
			Internal_MultiResource(this);
		}

		public MultiResource(SubResource[] entries)
		{
			Internal_MultiResource0(this, entries);
		}

		[ShowInInspector]
		[NativeWrapper]
		public SubResource[] Entries
		{
			get { return Internal_GetEntries(mCachedPtr); }
			set { Internal_SetEntries(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_MultiResource(MultiResource managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_MultiResource0(MultiResource managedInstance, SubResource[] entries);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SubResource[] Internal_GetEntries(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEntries(IntPtr thisPtr, SubResource[] value);
	}

	/** @} */
#endif
}
