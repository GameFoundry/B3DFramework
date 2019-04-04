//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			get { return Internal_getdirectionalShadowDistance(mCachedPtr); }
			set { Internal_setdirectionalShadowDistance(mCachedPtr, value); }
		}

		/// <summary>
		/// Number of cascades to use for directional shadows. Higher number of cascades increases shadow quality as each 
		/// individual cascade has less area to cover, but can significantly increase performance cost, as well as a minor 
		/// increase in memory cost. Valid range is roughly [1, 6].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int NumCascades
		{
			get { return Internal_getnumCascades(mCachedPtr); }
			set { Internal_setnumCascades(mCachedPtr, value); }
		}

		/// <summary>
		/// Allows you to control how are directional shadow cascades distributed. Value of 1 means the cascades will be linearly 
		/// split, each cascade taking up the same amount of space. Value of 2 means each subsequent split will be twice the size 
		/// of the previous one (meaning cascades closer to the viewer cover a smaller area, and therefore yield higher 
		/// resolution shadows). Higher values increase the size disparity between near and far cascades at an exponential rate. 
		/// Valid range is roughly [1, 4].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float CascadeDistributionExponent
		{
			get { return Internal_getcascadeDistributionExponent(mCachedPtr); }
			set { Internal_setcascadeDistributionExponent(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the number of samples used for percentage closer shadow map filtering. Higher values yield higher quality 
		/// shadows, at the cost of performance. Valid range is [1, 4].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public int ShadowFilteringQuality
		{
			get { return Internal_getshadowFilteringQuality(mCachedPtr); }
			set { Internal_setshadowFilteringQuality(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ShadowSettings(ShadowSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getdirectionalShadowDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setdirectionalShadowDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getnumCascades(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setnumCascades(IntPtr thisPtr, int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getcascadeDistributionExponent(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setcascadeDistributionExponent(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getshadowFilteringQuality(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setshadowFilteringQuality(IntPtr thisPtr, int value);
	}

	/** @} */
}
