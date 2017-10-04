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
    /// Determines how is grid drawn.
    /// </summary>
    internal enum GridMode // Note: Must match C++ enum GridMode
    {
        Perspective,
        OrthoX,
        OrthoY,
        OrthoZ,
        OrthoNegX,
        OrthoNegY,
        OrthoNegZ
    }

    /// <summary>
    /// Handles rendering of the scene grid for the specified camera. Grid properties are controlled through 
    /// <see cref="EditorSettings"/>.
    /// </summary>
    internal sealed class SceneGrid : ScriptObject
    {
        /// <summary>
        /// Creates a new scene grid renderer.
        /// </summary>
        /// <param name="sceneCamera">Camera into which the grid will be rendered.</param>
        internal SceneGrid(Camera sceneCamera)
        {
            Internal_Create(this, sceneCamera.GetCachedPtr());
        }

        /// <summary>
        /// Queues grid drawing for this frame.
        /// </summary>
        internal void Draw()
        {
            Internal_Draw(mCachedPtr);
        }

        /// <summary>
        /// Changes how is the grid drawn.
        /// </summary>
        /// <param name="mode">Determines orientation and position of the grid so it suits the camera in the provided mode.
        /// </param>
        internal void SetMode(GridMode mode)
        {
            Internal_SetMode(mCachedPtr, mode);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(SceneGrid managedInstance, IntPtr camera);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Draw(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMode(IntPtr thisPtr, GridMode mode);
    }

    /** @} */
}
