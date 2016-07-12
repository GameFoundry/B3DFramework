//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI-Engine
     *  @{
     */

    /// <summary>
    /// A GUI element that represents a horizontal slider with a draggable handle.
    /// </summary>
    public sealed class GUISliderH : GUIElement
    {
        public delegate void OnChangedDelegate(float percent);

        /// <summary>
        /// Triggered when the slider handle moves. Provided parameter represents
        /// the position of the handle, in percent ranging [0, 1].
        /// </summary>
        public event OnChangedDelegate OnChanged;

        /// <summary>
        /// Returns the position of the slider handle, in percent ranging [0, 1].
        /// </summary>
        public float Percent
        {
            get { return Internal_GetPercent(mCachedPtr); }
            set { Internal_SetPercent(mCachedPtr, value); }
        }

        /// <summary>
        /// Returns the position of the slider handle, in range determined by <see cref="SetRange"/>. If range is not defined
        /// set to [0, 1] this is equivalent of <see cref="Percent"/>.
        /// </summary>
        public float Value
        {
            get { return Internal_GetValue(mCachedPtr); }
            set { Internal_SetValue(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new horizontal slider.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUISliderH(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new vertical slider.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUISliderH(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        /// <summary>
        /// Sets a range that will input field values will be clamped to. Set to large negative/positive values if clamping
        /// is not required.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp values to.</param>
        /// <param name="max">Maximum boundary of the range to clamp values to.</param>
        public void SetRange(float min, float max)
        {
            Internal_SetRange(mCachedPtr, min, max);
        }

        /// <summary>
        /// The upper bound of the slider range
        /// </summary>
        /// <returns>The upper bound of the slider range</returns>
        public float GetRangeMaximum()
        {
            return Internal_GetRangeMaximum(mCachedPtr);
        }

        /// <summary>
        /// The lower bound of the slider range
        /// </summary>
        /// <returns>The lower bound of the slider range</returns>
        public float GetRangeMinimum()
        {
            return Internal_GetRangeMinimum(mCachedPtr);
        }

        /// <summary>
        /// A step value that determines the minimal increment the slider can be increased or decreased by.
        /// </summary>
        public float Step
        {
            get { return Internal_GetStep(mCachedPtr); }
            set { Internal_SetStep(mCachedPtr, value); }
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        /// <summary>
        /// Triggered by the native interop object when the slider handle is moved.
        /// </summary>
        /// <param name="percent">New position of the slider handle, in percent ranging [0, 1].</param>
        private void DoOnChanged(float percent)
        {
            if (OnChanged != null)
                OnChanged(percent);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUISliderH instance, string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetPercent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPercent(IntPtr nativeInstance, float percent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetValue(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetValue(IntPtr nativeInstance, float percent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRange(IntPtr nativeInstance, float min, float max);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeMaximum(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeMinimum(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStep(IntPtr nativeInstance, float step);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetStep(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /// <summary>
    /// A GUI element that represents a vertical slider with a draggable handle.
    /// </summary>
    public sealed class GUISliderV : GUIElement
    {
        public delegate void OnChangedDelegate(float percent);

        /// <summary>
        /// Triggered when the slider handle moves. Provided parameter represents
        /// the position of the handle, in percent ranging [0, 1].
        /// </summary>
        public event OnChangedDelegate OnChanged;

        /// <summary>
        /// Returns the position of the slider handle, in percent ranging [0, 1].
        /// </summary>
        public float Percent
        {
            get { return Internal_GetPercent(mCachedPtr); }
            set { Internal_SetPercent(mCachedPtr, value); }
        }

        /// <summary>
        /// Returns the position of the slider handle, in range determined by <see cref="SetRange"/>. If range is not defined
        /// set to [0, 1] this is equivalent of <see cref="Percent"/>.
        /// </summary>
        public float Value
        {
            get { return Internal_GetValue(mCachedPtr); }
            set { Internal_SetValue(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new vertical slider.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUISliderV(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new vertical slider.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUISliderV(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        /// <summary>
        /// Sets a range that will input field values will be clamped to. Set to large negative/positive values if clamping
        /// is not required.
        /// </summary>
        /// <param name="min">Minimum boundary of the range to clamp values to.</param>
        /// <param name="max">Maximum boundary of the range to clamp values to.</param>
        public void SetRange(float min, float max)
        {
            Internal_SetRange(mCachedPtr, min, max);
        }

        /// <summary>
        /// The upper bound of the slider range
        /// </summary>
        /// <returns>The upper bound of the slider range</returns>
        public float GetRangeMaximum()
        {
            return Internal_GetRangeMaximum(mCachedPtr);
        }

        /// <summary>
        /// The lower bound of the slider range
        /// </summary>
        /// <returns>The lower bound of the slider range</returns>
        public float GetRangeMinimum()
        {
            return Internal_GetRangeMinimum(mCachedPtr);
        }

        /// <summary>
        /// A step value that determines the minimal increment the slider can be increased or decreased by.
        /// </summary>
        public float Step
        {
            get { return Internal_GetStep(mCachedPtr); }
            set { Internal_SetStep(mCachedPtr, value); }
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        /// <summary>
        /// Triggered by the native interop object when the slider handle is moved.
        /// </summary>
        /// <param name="percent">New position of the slider handle, in percent ranging [0, 1].</param>
        private void DoOnChanged(float percent)
        {
            if (OnChanged != null)
                OnChanged(percent);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUISliderV instance, string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetPercent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPercent(IntPtr nativeInstance, float percent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetValue(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetValue(IntPtr nativeInstance, float percent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRange(IntPtr nativeInstance, float min, float max);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeMaximum(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeMinimum(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStep(IntPtr nativeInstance, float step);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetStep(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}
