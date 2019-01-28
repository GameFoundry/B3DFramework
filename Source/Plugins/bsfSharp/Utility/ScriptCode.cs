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
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// A resource containing compileable script code.
    /// </summary>
    public class ScriptCode : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private ScriptCode()
        { }

        /// <summary>
        /// Script code text.
        /// </summary>
        public string Text
        {
            get { return Internal_GetText(mCachedPtr); }
            set { Internal_SetText(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the script code be compiled with editor assemblies.
        /// </summary>
        public bool EditorScript
        {
            get { return Internal_IsEditorScript(mCachedPtr); }
            set { Internal_SetEditorScript(mCachedPtr, value); }
        }

        /// <summary>
        /// Returns all script types that have been created when this script code resource was compiled.
        /// </summary>
        public Type[] Types
        {
            get { return Internal_GetTypes(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetText(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetText(IntPtr thisPtr, string value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsEditorScript(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetEditorScript(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Type[] Internal_GetTypes(IntPtr thisPtr);
    }

    /** @} */
}
