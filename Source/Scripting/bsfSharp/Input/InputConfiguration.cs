//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Input
     *  @{
     */

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
