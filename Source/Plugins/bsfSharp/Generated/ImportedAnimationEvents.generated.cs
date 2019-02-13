//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>A set of animation events that will be added to an animation clip during animation import.</summary>
	[ShowInInspector]
	public partial class ImportedAnimationEvents : ScriptObject
	{
		private ImportedAnimationEvents(bool __dummy0) { }

		public ImportedAnimationEvents()
		{
			Internal_ImportedAnimationEvents(this);
		}

		[ShowInInspector]
		[NativeWrapper]
		public string Name
		{
			get { return Internal_getname(mCachedPtr); }
			set { Internal_setname(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public AnimationEvent[] Events
		{
			get { return Internal_getevents(mCachedPtr); }
			set { Internal_setevents(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ImportedAnimationEvents(ImportedAnimationEvents managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getname(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setname(IntPtr thisPtr, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationEvent[] Internal_getevents(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setevents(IntPtr thisPtr, AnimationEvent[] value);
	}

	/** @} */
#endif
}
