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

	/// <summary>Various options that control shadow rendering for a specific view.</summary>
	[ShowInInspector]
	public partial class ShadowSettings : ScriptObject
	{
		private ShadowSettings(bool __dummy0) { }

		public ShadowSettings()
		{
			Internal_ShadowSettings(this);
		}

		/// <summary>
		/// Maximum distance that directional light shadows are allowed to render at. Decreasing the distance can yield higher 
		/// quality shadows nearer to the viewer, as the shadow map resolution isn&apos;t being used up on far away portions of 
		/// the scene. In world units (meters).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float DirectionalShadowDistance
		{
			get { return Internal_GetDirectionalShadowDistance(mCachedPtr); }
			set { Internal_SetDirectionalShadowDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Number of cascades to use for directional shadows. Higher number of cascades increases shadow quality as each 
		/// individual cascade has less area to cover, but can significantly increase performance cost, as well as a minor 
		/// increase in memory cost. Valid range is roughly [1, 6].
		/// </summary>
		[ShowInInspector]
		[Range(1f, 6f, true)]
		[NativeWrapper]
		public int NumCascades
		{
			get { return Internal_GetNumCascades(mCachedPtr); }
			set { Internal_SetNumCascades(mCachedPtr, value); }
		}

		/// <summary>
		/// Allows you to control how are directional shadow cascades distributed. Value of 1 means the cascades will be linearly 
		/// split, each cascade taking up the same amount of space. Value of 2 means each subsequent split will be twice the size 
		/// of the previous one (meaning cascades closer to the viewer cover a smaller area, and therefore yield higher 
		/// resolution shadows). Higher values increase the size disparity between near and far cascades at an exponential rate. 
		/// Valid range is roughly [1, 4].
		/// </summary>
		[ShowInInspector]
		[Range(1f, 4f, true)]
		[NativeWrapper]
		public float CascadeDistributionExponent
		{
			get { return Internal_GetCascadeDistributionExponent(mCachedPtr); }
			set { Internal_SetCascadeDistributionExponent(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the number of samples used for percentage closer shadow map filtering. Higher values yield higher quality 
		/// shadows, at the cost of performance. Valid range is [1, 4].
		/// </summary>
		[ShowInInspector]
		[Range(1f, 4f, true)]
		[NativeWrapper]
		public int ShadowFilteringQuality
		{
			get { return Internal_GetShadowFilteringQuality(mCachedPtr); }
			set { Internal_SetShadowFilteringQuality(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ShadowSettings(ShadowSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDirectionalShadowDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDirectionalShadowDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetNumCascades(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNumCascades(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetCascadeDistributionExponent(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCascadeDistributionExponent(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetShadowFilteringQuality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetShadowFilteringQuality(IntPtr thisPtr, int value);
	}

	/** @} */
}
