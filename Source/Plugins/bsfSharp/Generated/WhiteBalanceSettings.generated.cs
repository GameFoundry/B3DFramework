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
		[NativeWrapper]
		public float Temperature
		{
			get { return Internal_gettemperature(mCachedPtr); }
			set { Internal_settemperature(mCachedPtr, value); }
		}

		/// <summary>
		/// Additional tint to be applied during white balancing. Can be used to further tweak the white balancing effect by 
		/// modifying the tint of the light. The tint is chosen on the Planckian locus isothermal, depending on the light 
		/// temperature specified by #temperature.
		///
		/// In range [-1.0f, 1.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Tint
		{
			get { return Internal_gettint(mCachedPtr); }
			set { Internal_settint(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WhiteBalanceSettings(WhiteBalanceSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gettemperature(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settemperature(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gettint(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_settint(IntPtr thisPtr, float value);
	}

	/** @} */
}
