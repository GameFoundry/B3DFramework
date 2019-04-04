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

	/// <summary>
	/// Allows you to define a volume of light probes that will be used for indirect lighting. Lighting information in the 
	/// scene will be interpolated from nearby probes to calculate the amount of indirect lighting at that position. It is up 
	/// to the caller to place the light probes in areas where the lighting changes in order to yield the best results.
	///
	/// The volume can never have less than 4 probes.
	/// </summary>
	[ShowInInspector]
	public partial class LightProbeVolume : Component
	{
		private LightProbeVolume(bool __dummy0) { }
		protected LightProbeVolume() { }

		/// <summary>Returns the volume that&apos;s used for adding probes in a uniform grid pattern.</summary>
		[NativeWrapper]
		public AABox GridVolume
		{
			get
			{
				AABox temp;
				Internal_getGridVolume(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns the cell count that&apos;s used for determining the density of probes within a grid volume.</summary>
		[NativeWrapper]
		public Vector3I CellCount
		{
			get
			{
				Vector3I temp;
				Internal_getCellCount(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Adds a new probe at the specified position and returns a handle to the probe. The position is relative to the volume 
		/// origin.
		/// </summary>
		public int AddProbe(Vector3 position)
		{
			return Internal_addProbe(mCachedPtr, ref position);
		}

		/// <summary>Updates the position of the probe with the specified handle.</summary>
		public void SetProbePosition(int handle, Vector3 position)
		{
			Internal_setProbePosition(mCachedPtr, handle, ref position);
		}

		/// <summary>Retrieves the position of the probe with the specified handle.</summary>
		public Vector3 GetProbePosition(int handle)
		{
			Vector3 temp;
			Internal_getProbePosition(mCachedPtr, handle, out temp);
			return temp;
		}

		/// <summary>
		/// Removes the probe with the specified handle. Note that if this is one of the last four remaining probes in the volume 
		/// it cannot be removed.
		/// </summary>
		public void RemoveProbe(int handle)
		{
			Internal_removeProbe(mCachedPtr, handle);
		}

		/// <summary>Returns a list of positions of all light probes in the volume.</summary>
		public LightProbeInfo[] GetProbes()
		{
			return Internal_getProbes(mCachedPtr);
		}

		/// <summary>
		/// Causes the information for this specific light probe to be updated. You generally want to call this when the probe is 
		/// moved or the scene around the probe changes.
		/// </summary>
		public void RenderProbe(int handle)
		{
			Internal_renderProbe(mCachedPtr, handle);
		}

		/// <summary>
		/// Causes the information for all lights probes to be updated. You generally want to call this if you move the entire 
		/// light volume or the scene around the volume changes.
		/// </summary>
		public void RenderProbes()
		{
			Internal_renderProbes(mCachedPtr);
		}

		/// <summary>
		/// Resizes the light probe grid and inserts new light probes, if the new size is larger than previous size. New probes 
		/// are inserted in a grid pattern matching the new size and density parameters.
		///
		/// Note that shrinking the volume will not remove light probes. In order to remove probes outside of the new volume call 
		/// clip().
		///
		/// Resize will not change the positions of current light probes. If you wish to reset all probes to the currently set 
		/// grid position, call reset().
		/// </summary>
		/// <param name="volume">Axis aligned volume to be covered by the light probes.</param>
		/// <param name="cellCount">
		/// Number of grid cells to split the volume into. Minimum number of 1, in which case each corner of the volume is 
		/// represented by a single probe. Higher values subdivide the volume in an uniform way.
		/// </param>
		public void Resize(AABox volume, Vector3I cellCount)
		{
			Internal_resize(mCachedPtr, ref volume, ref cellCount);
		}

		/// <summary>Removes any probes outside of the current grid volume.</summary>
		public void Clip()
		{
			Internal_clip(mCachedPtr);
		}

		/// <summary>
		/// Resets all probes to match the original grid pattern. This will reset probe positions, as well as add/remove probes 
		/// as necessary, essentially losing any custom changes to the probes.
		/// </summary>
		public void Reset()
		{
			Internal_reset(mCachedPtr);
		}

		/// <summary>
		/// Resizes the light probe grid and inserts new light probes, if the new size is larger than previous size. New probes 
		/// are inserted in a grid pattern matching the new size and density parameters.
		///
		/// Note that shrinking the volume will not remove light probes. In order to remove probes outside of the new volume call 
		/// clip().
		///
		/// Resize will not change the positions of current light probes. If you wish to reset all probes to the currently set 
		/// grid position, call reset().
		/// </summary>
		/// <param name="volume">Axis aligned volume to be covered by the light probes.</param>
		public void Resize(AABox volume)
		{
			Vector3I cellCount = new Vector3I(1, 1, 1);
			Internal_resize(mCachedPtr, ref volume, ref cellCount);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_addProbe(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setProbePosition(IntPtr thisPtr, int handle, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getProbePosition(IntPtr thisPtr, int handle, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeProbe(IntPtr thisPtr, int handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern LightProbeInfo[] Internal_getProbes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_renderProbe(IntPtr thisPtr, int handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_renderProbes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_resize(IntPtr thisPtr, ref AABox volume, ref Vector3I cellCount);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_clip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_reset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getGridVolume(IntPtr thisPtr, out AABox __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getCellCount(IntPtr thisPtr, out Vector3I __output);
	}

	/** @} */
}
