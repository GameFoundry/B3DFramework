//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
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
        /// Notifies the native script interop object that the managed instance was finalized.
        /// </summary>
        ~ScriptObject()
        {
            if (mCachedPtr == IntPtr.Zero)
            {
                Debug.LogError("Script object is being finalized but doesn't have a pointer to its interop object. Type: " + GetType());

#if DEBUG
                // This will cause a crash, so we ignore it in release mode hoping all it causes is a memory leak.
                Internal_ManagedInstanceDeleted(mCachedPtr);
#endif
            }
            else
                Internal_ManagedInstanceDeleted(mCachedPtr);
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
    }

    /** @endcond */
}
