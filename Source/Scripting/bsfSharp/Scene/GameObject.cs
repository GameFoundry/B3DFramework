//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace bs
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
        /// Returns a universally unique identifier that persists scene save/load.
        /// </summary>
        public UUID UUID
        {
            get
            {
                UUID uuid;
                Internal_GetUUID(mCachedPtr, out uuid);
                return uuid;
            }
        }

        /// <summary>
        /// Checks if the game object has been destroyed.
        /// </summary>
        public bool IsDestroyed => Internal_GetIsDestroyed(mCachedPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetUUID(IntPtr nativeInstance, out UUID uuid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIsDestroyed(IntPtr nativeInstance);
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
