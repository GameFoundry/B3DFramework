//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Component wrapper for GUIWidget.</summary>
	[ShowInInspector]
	public partial class GUIWidget : Component
	{
		private GUIWidget(bool __dummy0) { }
		protected GUIWidget() { }

		[NativeWrapper]
		public GUIPanel Panel
		{
			get { return Internal_GetPanel(mCachedPtr); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public byte Depth
		{
			get { return Internal_GetDepth(mCachedPtr); }
			set { Internal_SetDepth(mCachedPtr, value); }
		}

		public bool InBounds(TVector2<TUnitValue<int,PhysicalPixel>> position)
		{
			return Internal_InBounds(mCachedPtr, ref position);
		}

		public TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> GetBounds()
		{
			TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> temp;
			Internal_GetBounds(mCachedPtr, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUIPanel Internal_GetPanel(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern byte Internal_GetDepth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDepth(IntPtr thisPtr, byte depth);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_InBounds(IntPtr thisPtr, ref TVector2<TUnitValue<int,PhysicalPixel>> position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetBounds(IntPtr thisPtr, out TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> __output);
	}

	/** @} */
}
