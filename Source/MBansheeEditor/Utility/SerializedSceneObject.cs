//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Utility-Editor
     *  @{
     */

    /// <summary>
    /// Records a state of a scene object, it's components and optionally its child hierarchy. Stored state can
    /// be applied to a scene object in order to restore the originally recorded state.
    /// </summary>
    public class SerializedSceneObject : ScriptObject
    {
        /// <summary>
        /// Records the current state of the provided scene object.
        /// </summary>
        /// <param name="so">Scene object to record the state for.</param>
        /// <param name="hierarchy">If true, state will be recorded for the scene object and all of its children. Otherwise
        ///                         the state will only be recorded for the provided scene object.</param>
        public SerializedSceneObject(SceneObject so, bool hierarchy)
        {
            IntPtr soPtr = IntPtr.Zero;
            if (so != null)
                soPtr = so.GetCachedPtr();

            Internal_CreateInstance(this, soPtr, hierarchy);
        }

        /// <summary>
        /// Restores the referenced scene object to its original state.
        /// </summary>
        public void Restore()
        {
            Internal_Restore(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(SerializedSceneObject instance, IntPtr so, bool hierarchy);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Restore(IntPtr thisPtr);
    }

    /** @} */
}
