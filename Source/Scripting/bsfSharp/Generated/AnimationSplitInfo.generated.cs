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
			get { return Internal_GetName(mCachedPtr); }
			set { Internal_SetName(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int StartFrame
		{
			get { return Internal_GetStartFrame(mCachedPtr); }
			set { Internal_SetStartFrame(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public int EndFrame
		{
			get { return Internal_GetEndFrame(mCachedPtr); }
			set { Internal_SetEndFrame(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public bool IsAdditive
		{
			get { return Internal_GetIsAdditive(mCachedPtr); }
			set { Internal_SetIsAdditive(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AnimationSplitInfo(AnimationSplitInfo managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AnimationSplitInfo0(AnimationSplitInfo managedInstance, string name, int startFrame, int endFrame, bool isAdditive);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetName(IntPtr thisPtr, string value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetStartFrame(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetStartFrame(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetEndFrame(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEndFrame(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetIsAdditive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIsAdditive(IntPtr thisPtr, bool value);
	}

	/** @} */
#endif
}
