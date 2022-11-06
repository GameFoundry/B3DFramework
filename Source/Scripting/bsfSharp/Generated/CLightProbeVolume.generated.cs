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

	[ShowInInspector]
	public partial class LightProbeVolume : Component
	{
		private LightProbeVolume(bool __dummy0) { }
		protected LightProbeVolume() { }

		[NativeWrapper]
		public AABox GridVolume
		{
			get
			{
				AABox temp;
				Internal_GetGridVolume(mCachedPtr, out temp);
				return temp;
			}
		}

		[NativeWrapper]
		public Vector3I CellCount
		{
			get
			{
				Vector3I temp;
				Internal_GetCellCount(mCachedPtr, out temp);
				return temp;
			}
		}

		public int AddProbe(Vector3 position)
		{
			return Internal_AddProbe(mCachedPtr, ref position);
		}

		public void SetProbePosition(int handle, Vector3 position)
		{
			Internal_SetProbePosition(mCachedPtr, handle, ref position);
		}

		public Vector3 GetProbePosition(int handle)
		{
			Vector3 temp;
			Internal_GetProbePosition(mCachedPtr, handle, out temp);
			return temp;
		}

		public void RemoveProbe(int handle)
		{
			Internal_RemoveProbe(mCachedPtr, handle);
		}

		public LightProbeInfo[] GetProbes()
		{
			return Internal_GetProbes(mCachedPtr);
		}

		public void RenderProbe(int handle)
		{
			Internal_RenderProbe(mCachedPtr, handle);
		}

		public void RenderProbes()
		{
			Internal_RenderProbes(mCachedPtr);
		}

		public void Resize(AABox volume, Vector3I cellCount)
		{
			Internal_Resize(mCachedPtr, ref volume, ref cellCount);
		}

		public void Clip()
		{
			Internal_Clip(mCachedPtr);
		}

		public void Reset()
		{
			Internal_Reset(mCachedPtr);
		}

		public void Resize(AABox volume)
		{
			Vector3I cellCount = new Vector3I(1, 1, 1);
			Internal_Resize(mCachedPtr, ref volume, ref cellCount);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_AddProbe(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetProbePosition(IntPtr thisPtr, int handle, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetProbePosition(IntPtr thisPtr, int handle, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveProbe(IntPtr thisPtr, int handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern LightProbeInfo[] Internal_GetProbes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RenderProbe(IntPtr thisPtr, int handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RenderProbes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Resize(IntPtr thisPtr, ref AABox volume, ref Vector3I cellCount);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Clip(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Reset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetGridVolume(IntPtr thisPtr, out AABox __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCellCount(IntPtr thisPtr, out Vector3I __output);
	}

	/** @} */
}
