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

	/// <summary>Information about how to split an AnimationClip into multiple separate clips.</summary>
	[ShowInInspector]
	public partial class AnimationSplitInfo : ScriptObject
	{
		private AnimationSplitInfo(bool __dummy0) { }

		public AnimationSplitInfo()
		{
			Internal_AnimationSplitInfo(this);
		}

		public AnimationSplitInfo(string name, int startFrame, int endFrame, bool isAdditive = false)
		{
			Internal_AnimationSplitInfo0(this, name, startFrame, endFrame, isAdditive);
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
		public int StartFrame
		{
			get { return Internal_getstartFrame(mCachedPtr); }
			set { Internal_setstartFrame(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int EndFrame
		{
			get { return Internal_getendFrame(mCachedPtr); }
			set { Internal_setendFrame(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool IsAdditive
		{
			get { return Internal_getisAdditive(mCachedPtr); }
			set { Internal_setisAdditive(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AnimationSplitInfo(AnimationSplitInfo managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AnimationSplitInfo0(AnimationSplitInfo managedInstance, string name, int startFrame, int endFrame, bool isAdditive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getname(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setname(IntPtr thisPtr, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getstartFrame(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setstartFrame(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getendFrame(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setendFrame(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getisAdditive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setisAdditive(IntPtr thisPtr, bool value);
	}

	/** @} */
#endif
}
