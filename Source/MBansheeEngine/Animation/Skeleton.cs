//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Animation
     *  @{
     */

    /// <summary>
    /// Contains information about bones required for skeletal animation.
    /// </summary>
    public class Skeleton : ScriptObject
    {
        /// <summary>
        /// Constructor for internal runtime use only.
        /// </summary>
        private Skeleton()
        { }

        /// <summary>
        /// Returns the total number of bones in the skeleton.
        /// </summary>
        /// <returns>Number of bones.</returns>
        public int NumBones
        {
            get { return Internal_GetNumBones(mCachedPtr); }
        }

        /// <summary>
        /// Returns information about a bone at the provided index.
        /// </summary>
        /// <param name="boneIdx">Index of the bone to retrieve information for.</param>
        /// <returns>Information about the bone, or null if index was out of range.</returns>
        public BoneInfo GetBoneInfo(int boneIdx)
        {
            return Internal_GetBoneInfo(mCachedPtr, boneIdx);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetNumBones(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern BoneInfo Internal_GetBoneInfo(IntPtr thisPtr, int boneIdx);
    }

    /// <summary>
    /// Contains internal information about a single bone in a <see cref="Skeleton"/>.
    /// </summary>
    public class BoneInfo
    {
        /// <summary>
        /// Unique name of the bone.
        /// </summary>
        public string Name;

        /// <summary>
        /// Index of the parent bone (within the relevant <see cref="Skeleton"/> object). -1 if root bone.
        /// </summary>
        public int Parent;

        /// <summary>
        /// Inverse transform of the pose the skeleton was initially created in.
        /// </summary>
        public Matrix4 InvBindPose;
    }

    /** @} */
}