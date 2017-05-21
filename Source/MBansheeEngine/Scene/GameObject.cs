//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Scene
     *  @{
     */

    /// <summary>
    /// A base class for objects that can be part of the scene and referenced by other game objects.
    /// </summary>
    public class GameObject : ScriptObject
    {
        /// <summary>
        /// Returns a unique ID for the game object.
        /// </summary>
        public UInt64 InstanceId
        {
            get { return Internal_GetInstanceId(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Internal_GetInstanceId(IntPtr thisPtr);
    }

    /// <summary>
    /// Flags used for notifying child scene object and components when a transform has been changed.
    /// </summary>
    [Flags]
    public enum TransformChangedFlags // Note: Must match C++ enum TransformChangedFlags
    {
        /// <summary>
        /// Component will not be notified about any events relating to the transform.
        /// </summary>
        None = 0x00,
        /// <summary>
        /// Component will be notified when the its position, rotation or scale has changed.
        /// </summary>
        Transform = 0x01,
        /// <summary>
        /// Component will be notified when its parent changes.
        /// </summary>
        Parent = 0x02,
        /// <summary>
        /// Component will be notified when its scene object's mobility state changes.
        /// </summary>
        Mobility = 0x04
}

/** @} */
}
