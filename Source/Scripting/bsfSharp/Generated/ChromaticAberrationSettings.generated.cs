//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Settings that control the chromatic aberration effect.</summary>
	[ShowInInspector]
	public partial class ChromaticAberrationSettings : ScriptObject
	{
		private ChromaticAberrationSettings(bool __dummy0) { }

		public ChromaticAberrationSettings()
		{
			Internal_ChromaticAberrationSettings(this);
		}

		/// <summary>
		/// Optional texture to apply to generate the channel shift fringe, allowing you to modulate the shifted colors. This 
		/// texture should be 3x1 size, where the first pixel modules red, second green and third blue channel. If using the 
		/// complex aberration effect you can use a larger texture, Nx1 size.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> FringeTexture
		{
			get { return Internal_GetFringeTexture(mCachedPtr); }
			set { Internal_SetFringeTexture(mCachedPtr, value); }
		}

		/// <summary>Enables or disables the effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Enabled
		{
			get { return Internal_GetEnabled(mCachedPtr); }
			set { Internal_SetEnabled(mCachedPtr, value); }
		}

		/// <summary>Type of algorithm to use for rendering the effect.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ChromaticAberrationType Type
		{
			get { return Internal_GetType(mCachedPtr); }
			set { Internal_SetType(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the brightness of the lens flare effect. Value of 1 means the lens flare will be displayed at the same 
		/// intensity as the scene it was derived from. In range [0, 1], default being 0.05.
		/// </summary>
		[ShowInInspector]
		[Range(0f, 1f, false)]
		[NativeWrapper]
		public float ShiftAmount
		{
			get { return Internal_GetShiftAmount(mCachedPtr); }
			set { Internal_SetShiftAmount(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ChromaticAberrationSettings(ChromaticAberrationSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetFringeTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFringeTexture(IntPtr thisPtr, RRef<Texture> value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetEnabled(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnabled(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ChromaticAberrationType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetType(IntPtr thisPtr, ChromaticAberrationType value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetShiftAmount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetShiftAmount(IntPtr thisPtr, float value);
	}
}
