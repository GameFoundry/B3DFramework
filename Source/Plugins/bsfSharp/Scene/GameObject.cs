//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
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
