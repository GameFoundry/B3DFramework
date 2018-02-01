//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Handles
     *  @{
     */

    /// <summary>
    /// Base class for all handle sliders. A handle slider is geometry that the user can interact with by selecting or 
    /// dragging (sliding) it while in scene view. Sliders generally output a one- or multi-dimensional delta value 
    /// that signals the drag amount (and/or direction). Handle sliders are one of the building blocks for 
    /// <see cref="Handle"/> implementations.
    /// </summary>
    public class HandleSlider : ScriptObject
    {
        /// <summary>
        /// Possible states the slider can be in.
        /// </summary>
        public enum StateType
        {
            /// <summary>Slider is not being interacted with.</summary>
            Inactive,
            /// <summary>Slider is clicked on and being dragged.</summary>
            Active,
            /// <summary>Slider is being hovered over but isn't clicked on.</summary>
            Hover
        };

        /// <summary>
        /// Creates a new handle slider.
        /// </summary>
        /// <param name="parentHandle">Handle that the slider belongs to.</param>
        public HandleSlider(Handle parentHandle)
        {
            parentHandle.RegisterSlider(this);
        }

        /// <summary>
        /// World position of the slider.
        /// </summary>
        public Vector3 Position
        {
            get
            {
                Vector3 value;
                Internal_GetPosition(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetPosition(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// World rotation of the slider.
        /// </summary>
        public Quaternion Rotation
        {
            get
            {
                Quaternion value;
                Internal_GetRotation(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetRotation(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// Scale of the slider.
        /// </summary>
        public Vector3 Scale
        {
            get
            {
                Vector3 value;
                Internal_GetScale(mCachedPtr, out value);
                return value;
            }

            set
            {
                Internal_SetScale(mCachedPtr, ref value);
            }
        }

        /// <summary>
        /// State the slider is currently in.
        /// </summary>
        public StateType State
        {
            get
            {
                StateType value;
                Internal_GetState(mCachedPtr, out value);
                return value;
            }
        }

        /// <summary>
        /// Determines can the slider be interacted with.
        /// </summary>
        public bool Enabled
        {
            get { return Internal_GetEnabled(mCachedPtr); }
            set { Internal_SetEnabled(mCachedPtr, value); }
        }

        /// <summary>
        /// Destroys the slider, removing it from the scene.
        /// </summary>
        internal void Destroy()
        {
            Internal_Destroy(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetPosition(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetRotation(IntPtr nativeInstance, out Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRotation(IntPtr nativeInstance, ref Quaternion value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetScale(IntPtr nativeInstance, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetScale(IntPtr nativeInstance, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetEnabled(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetEnabled(IntPtr nativeInstance, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetState(IntPtr nativeInstance, out StateType value);
    }

    /** @} */
}
