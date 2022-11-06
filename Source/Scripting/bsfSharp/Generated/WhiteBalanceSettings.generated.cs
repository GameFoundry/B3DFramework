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

	/// <summary>Settings that control white balance post-process.</summary>
	[ShowInInspector]
	public partial class WhiteBalanceSettings : ScriptObject
	{
		private WhiteBalanceSettings(bool __dummy0) { }

		public WhiteBalanceSettings()
		{
			Internal_WhiteBalanceSettings(this);
		}

		/// <summary>
		/// Temperature used for white balancing, in Kelvins.
		///
		/// Moves along the Planckian locus. In range [1500.0f, 15000.0f].
		/// </summary>
		[ShowInInspector]
		[Range(1500f, 15000f, true)]
		[NativeWrapper]
		public float Temperature
		{
			get { return Internal_GetTemperature(mCachedPtr); }
			set { Internal_SetTemperature(mCachedPtr, value); }
		}

		/// <summary>
		/// Additional tint to be applied during white balancing. Can be used to further tweak the white balancing effect by 
		/// modifying the tint of the light. The tint is chosen on the Planckian locus isothermal, depending on the light 
		/// temperature specified by #temperature.
		///
		/// In range [-1.0f, 1.0f].
		/// </summary>
		[ShowInInspector]
		[Range(-1f, 1f, true)]
		[NativeWrapper]
		public float Tint
		{
			get { return Internal_GetTint(mCachedPtr); }
			set { Internal_SetTint(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WhiteBalanceSettings(WhiteBalanceSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTemperature(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTemperature(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetTint(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTint(IntPtr thisPtr, float value);
	}

	/** @} */
}
