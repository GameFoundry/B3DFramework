//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Scene-Editor
     *  @{
     */

    /// <summary>
    /// Handles rendering and interaction with scene handles for the specified camera. 
    /// </summary>
    internal sealed class SceneHandles : ScriptObject
    {
        /// <summary>
        /// Creates a new scene handle manager.
        /// </summary>
        /// <param name="parent">Editor window in which the scene handles are displayed.</param>
        /// <param name="sceneCamera">Camera through which the scene handles are displayed.</param>
        internal SceneHandles(EditorWindow parent, Camera sceneCamera)
        {
            Internal_Create(this, parent.GetCachedPtr(), sceneCamera.GetCachedPtr());
        }

        /// <summary>
        /// Triggers handle pre-input callbacks. Must be called before all <see cref="UpdateInput"/> calls and followed by 
        /// <see cref="EndInput"/>. This should be called only once per frame.
        /// </summary>
        internal static void BeginInput()
        {
            Internal_BeginInput();
        }

        /// <summary>
        /// Triggers handle post-input callbacks. Must be called after all <see cref="UpdateInput"/> calls and after 
        /// <see cref="BeginInput"/>. This should be called only once per frame.
        /// </summary>
        internal static void EndInput()
        {
            Internal_EndInput();
        }

        /// <summary>
        /// Updates active handles by moving them as a result of any input. Make sure to call <see cref="BeginInput"/> before
        /// this method, followed by <see cref="EndInput"/> when done.
        /// </summary>
        /// <param name="pointerPos">Position of the pointer relative to the scene camera viewport.</param>
        /// <param name="inputDelta">Movement of the pointer since last frame.</param>
        internal void UpdateInput(Vector2I pointerPos, Vector2I inputDelta)
        {
            Internal_UpdateInput(mCachedPtr, ref pointerPos, ref inputDelta);
        }

        /// <summary>
        /// Draws the handles onto the target camera.
        /// </summary>
        internal void Draw()
        {
            Internal_Draw(mCachedPtr);
        }

        /// <summary>
        /// Selects a handle under the pointer position.
        /// </summary>
        /// <param name="pointerPos">Position of the pointer relative to the target camera's viewport.</param>
        internal void TrySelect(Vector2I pointerPos)
        {
            Internal_TrySelect(mCachedPtr, ref pointerPos);
        }

        /// <summary>
        /// Checks is any handle currently active.
        /// </summary>
        /// <returns>True if a handle is active.</returns>
        internal bool IsActive()
        {
            return Internal_IsActive(mCachedPtr);
        }

        /// <summary>
        /// Deselects any currently active handles.
        /// </summary>
        internal void ClearSelection()
        {
            Internal_ClearSelection(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(SceneHandles managedInstance, IntPtr parentWindow, IntPtr camera);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_BeginInput();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_EndInput();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UpdateInput(IntPtr thisPtr, ref Vector2I pointerPos, ref Vector2I inputDelta);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Draw(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_TrySelect(IntPtr thisPtr, ref Vector2I pointerPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsActive(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClearSelection(IntPtr thisPtr);
    }

    /** @} */
}
