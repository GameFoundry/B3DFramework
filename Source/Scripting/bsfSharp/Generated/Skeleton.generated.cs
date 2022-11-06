//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Contains information about bones required for skeletal animation.</summary>
	[ShowInInspector]
	public partial class Skeleton : ScriptObject
	{
		private Skeleton(bool __dummy0) { }
		protected Skeleton() { }

		/// <summary>Returns the total number of bones in the skeleton.</summary>
		[NativeWrapper]
		public int NumBones
		{
			get { return Internal_GetNumBones(mCachedPtr); }
		}

		/// <summary>Returns information about a bone at the provided index.</summary>
		/// <param name="boneIdx">Index of the bone to retrieve information for.</param>
		/// <returns>Information about the bone at the specified index.</returns>
		public BoneInfo GetBoneInfo(int boneIdx)
		{
			BoneInfo temp;
			Internal_GetBoneInfo(mCachedPtr, boneIdx, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetNumBones(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetBoneInfo(IntPtr thisPtr, int boneIdx, out BoneInfo __output);
	}

	/** @} */
}
