//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Input
     *  @{
     */

    /// <summary>
    /// Contains virtual to physical key mappings.
    /// </summary>
    public sealed class InputConfiguration : ScriptObject
    {
        /// <summary>
        /// Creates a new empty key configuration.
        /// </summary>
        public InputConfiguration()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Repeat interval for held virtual buttons. Buttons will be continously triggered in interval increments as long 
        /// as they button is being held.
        /// </summary>
        public UInt64 RepeatInternal
        {
            get
            {
                return Internal_GetRepeatInterval(mCachedPtr);
            }

            set
            {
                Internal_SetRepeatInterval(mCachedPtr, value);
            }
        }

        /// <summary>
        /// Registers a new virtual button.
        /// </summary>
        /// <param name="name">Unique name used to access the virtual button.</param>
        /// <param name="buttonCode">Physical button the virtual button is triggered by.</param>
        /// <param name="modifiers">Modifiers required to be pressed with the physical button to trigger the virtual button.
        ///                         </param>
        /// <param name="repeatable">If true, the virtual button events will be sent continually while the physical button 
        ///                          is being held.</param>
        public void RegisterButton(String name, ButtonCode buttonCode, 
            ButtonModifier modifiers = ButtonModifier.None, bool repeatable = false)
        {
            Internal_RegisterButton(mCachedPtr, name, buttonCode, modifiers, repeatable);
        }

        /// <summary>
        /// Unregisters a virtual button with the specified name. Events will no longer be generated for that button.
        /// </summary>
        /// <param name="name">Unique name used to access the virtual button.</param>
        public void UnregisterButton(String name)
        {
            Internal_UnregisterButton(mCachedPtr, name);
        }

        /// <summary>
        /// Registers a new virtual axis.
        /// </summary>
        /// <param name="name">Unique name used to access the virtual axis.</param>
        /// <param name="type">Type of physical axis to map to.</param>
        /// <param name="deadZone">Value below which to ignore axis value and consider it 0.</param>
        /// <param name="sensitivity">Higher sensitivity means the axis will more easily reach its maximum values.</param>
        /// <param name="invert">Should axis values be inverted.</param>
        public void RegisterAxis(String name, InputAxis type, float deadZone = 0.0001f, 
            float sensitivity = 1.0f, bool invert = false)
        {
            Internal_RegisterAxis(mCachedPtr, name, type, deadZone, sensitivity, invert);
        }

        /// <summary>
        /// Unregisters a virtual axis with the specified name. You will no longer be able to retrieve valid values for 
        /// that axis.
        /// </summary>
        /// <param name="name">Unique name used to access the virtual axis.</param>
        public void UnregisterAxis(String name)
        {
            Internal_UnregisterAxis(mCachedPtr, name);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(InputConfiguration inputConfig);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_RegisterButton(IntPtr thisPtr, String name, ButtonCode buttonCode,
            ButtonModifier modifiers, bool repeatable);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UnregisterButton(IntPtr thisPtr, String name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_RegisterAxis(IntPtr thisPtr, String name, InputAxis type, float deadZone,
            float sensitivity, bool invert);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UnregisterAxis(IntPtr thisPtr, String name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRepeatInterval(IntPtr thisPtr, UInt64 milliseconds);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Internal_GetRepeatInterval(IntPtr thisPtr);
    }

    /// <summary>
    /// Button modifiers used with along with keyboard buttons.
    /// </summary>
    public enum ButtonModifier // Note: Must match C++ enum ButtonModifier
    {
        None = 0x00,
        Shift = 0x01,
        Ctrl = 0x02,
        Alt = 0x04,
        ShiftCtrl = 0x03,
        CtrlAlt = 0x06,
        ShiftAlt = 0x05,
        ShiftCtrlAlt = 0x07
    };

    /// <summary>
    /// Handle for a virtual button. Virtual buttons allow you to map custom buttons to physical buttons and deal with them
    /// without knowing the underlying physical buttons, allowing easy input remapping.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VirtualButton // Note: Must match C++ class VirtualButton
    {
        private readonly int buttonId;

        /// <summary>
        /// Creates a new virtual button handle.
        /// </summary>
        /// <param name="name">Unique name of the virtual button.</param>
        public VirtualButton(string name)
        {
            buttonId = Internal_InitVirtualButton(name);
        }

        public static bool operator ==(VirtualButton lhs, VirtualButton rhs)
        {
            return lhs.buttonId == rhs.buttonId;
        }

        public static bool operator !=(VirtualButton lhs, VirtualButton rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return buttonId.GetHashCode();
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is VirtualButton))
                return false;

            VirtualButton otherBtn = (VirtualButton)other;
            if (buttonId.Equals(otherBtn.buttonId))
                return true;

            return false;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_InitVirtualButton(String name);
    }

    /// <summary>
    /// Handle for a virtual axis. Virtual axes allow you to map custom axes without needing to know the actual physical 
    /// device handling those axes.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct VirtualAxis // Note: Must match C++ class VirtualAxis
    {
        private readonly int axisId;

        /// <summary>
        /// Creates a new virual axis handle.
        /// </summary>
        /// <param name="name">Unique name of the virtual axis.</param>
        public VirtualAxis(string name)
        {
            axisId = Internal_InitVirtualAxis(name);
        }

        public static bool operator ==(VirtualAxis lhs, VirtualAxis rhs)
        {
            return lhs.axisId == rhs.axisId;
        }

        public static bool operator !=(VirtualAxis lhs, VirtualAxis rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return axisId.GetHashCode();
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is VirtualAxis))
                return false;

            VirtualAxis otherAxis = (VirtualAxis)other;
            if (axisId.Equals(otherAxis.axisId))
                return true;

            return false;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_InitVirtualAxis(String name);
    }

    /** @} */
}
