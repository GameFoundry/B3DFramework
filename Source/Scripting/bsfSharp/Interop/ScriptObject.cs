//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace bs
{
    /** @cond INTEROP */

    /// <summary>
    /// A base class for all script objects that interface with the native code.
    /// </summary>
    public class ScriptObject
    {
        /// <summary>
        /// A pointer to the native script interop object.
        /// </summary>
        internal IntPtr mCachedPtr;

        /// <summary>
        /// If >0, then new ScriptObjectWrapper interface is used for interop, otherwise old ScriptObjectBase interface is used. This field is temporary during the transition.
        /// </summary>
        internal int mIsUsingNewScriptObjectWrapper = 0;

        /// <summary>
        /// Notifies the native script interop object that the managed instance was finalized.
        /// </summary>
        ~ScriptObject()
        {
            if (mCachedPtr == IntPtr.Zero)
            {
                Debug.LogError("Script object is being finalized but doesn't have a pointer to its interop object. Type: " + GetType());

#if DEBUG
                // This will cause a crash, so we ignore it in release mode hoping all it causes is a memory leak.
                if(mIsUsingNewScriptObjectWrapper > 0)
                    Internal_ScriptObjectFinalizerCalled(mCachedPtr);
                else
                    Internal_ManagedInstanceDeleted(mCachedPtr);
#endif
            }
            else
            {
                if(mIsUsingNewScriptObjectWrapper > 0)
                    Internal_ScriptObjectFinalizerCalled(mCachedPtr);
                else
                    Internal_ManagedInstanceDeleted(mCachedPtr);
            }
        }

        /// <summary>
        /// Returns a pointer to the native script interop object.
        /// </summary>
        /// <returns>Pointer to the native script interop object.</returns>
        internal IntPtr GetCachedPtr()
        {
            return mCachedPtr;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ManagedInstanceDeleted(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ScriptObjectFinalizerCalled(IntPtr scriptObjectWrapperPointer);
    }

    /** @endcond */
}
