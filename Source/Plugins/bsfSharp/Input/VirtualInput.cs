//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Input
     *  @{
     */

    /// <summary>
    /// Handles virtual input that allows you to receive virtual input events that hide the actual physical input, allowing 
    /// you to easily change the input keys while being transparent to the external code.
    /// </summary>
    public static class VirtualInput
    {
        public delegate void OnButtonEventDelegate(VirtualButton btn, int deviceIdx);

        /// <summary>
        /// Triggered when a physical button combination corresponding to a virtual button is pressed.
        /// </summary>
        public static event OnButtonEventDelegate OnButtonDown;

        /// <summary>
        /// Triggered when a physical button combination corresponding to a virtual button is released.
        /// </summary>
        public static event OnButtonEventDelegate OnButtonUp;

        /// <summary>
        /// Triggered every frame while a physical button combination corresponding to a virtual button is being held down.
        /// </summary>
        public static event OnButtonEventDelegate OnButtonHeld;

        /// <summary>
        /// Input configuration that describes how physical keys map to virtual keys.
        /// </summary>
        public static InputConfiguration KeyConfig
        {
            get { return Internal_GetKeyConfig(); }
            set { Internal_SetKeyConfig(value); }
        }

        /// <summary>
        /// Checks if the physical button combination corresponding to the specified virtual button is being pressed this
        /// frame.
        /// </summary>
        /// <param name="button">Virtual button to check.</param>
        /// <param name="deviceIdx">Optional device index in case multiple input devices are available.</param>
        public static bool IsButtonDown(VirtualButton button, int deviceIdx = 0)
        {
            return Internal_IsButtonDown(ref button, deviceIdx);
        }

        /// <summary>
        /// Checks if the physical button combination corresponding to the specified virtual button is being released this
        /// frame.
        /// </summary>
        /// <param name="button">Virtual button to check.</param>
        /// <param name="deviceIdx">Optional device index in case multiple input devices are available.</param>
        public static bool IsButtonUp(VirtualButton button, int deviceIdx = 0)
        {
            return Internal_IsButtonUp(ref button, deviceIdx);
        }

        /// <summary>
        /// Checks if the physical button combination corresponding to the specified virtual button is being held down.
        /// </summary>
        /// <param name="button">Virtual button to check.</param>
        /// <param name="deviceIdx">Index of the device to check.</param>
        public static bool IsButtonHeld(VirtualButton button, int deviceIdx = 0)
        {
            return Internal_IsButtonHeld(ref button, deviceIdx);
        }

        /// <summary>
        /// Returns normalized value for the specified input axis. 
        /// </summary>
        /// <param name="axis">Virtual axis identifier.</param>
        /// <param name="deviceIdx">Optional device index in case multiple input devices are available.</param>
        /// <returns>Axis value, normally in [-1.0, 1.0] range, but can be outside the range for devices with unbound axes 
        ///          (for example mouse).</returns>
        public static float GetAxisValue(VirtualAxis axis, int deviceIdx = 0)
        {
            return Internal_GetAxisValue(ref axis, deviceIdx);
        }

        /// <summary>
        /// Triggered by the runtime when the virtual button is pressed.
        /// </summary>
        /// <param name="button">Virtual button that was pressed.</param>
        /// <param name="deviceIdx">Index of the device the button was pressed on.</param>
        private static void Internal_TriggerButtonDown(VirtualButton button, int deviceIdx)
        {
            if (OnButtonDown != null)
                OnButtonDown(button, deviceIdx);
        }

        /// <summary>
        /// Triggered by the runtime when the virtual button is released.
        /// </summary>
        /// <param name="button">Virtual button that was released.</param>
        /// <param name="deviceIdx">Index of the device the button was released on.</param>
        private static void Internal_TriggerButtonUp(VirtualButton button, int deviceIdx)
        {
            if (OnButtonUp != null)
                OnButtonUp(button, deviceIdx);
        }

        /// <summary>
        /// Triggered by the runtime every frame while a virtual button is being held down.
        /// </summary>
        /// <param name="button">Virtual button that is being held down.</param>
        /// <param name="deviceIdx">Index of the device the button is being held down on.</param>
        private static void Internal_TriggerButtonHeld(VirtualButton button, int deviceIdx)
        {
            if (OnButtonHeld != null)
                OnButtonHeld(button, deviceIdx);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern InputConfiguration Internal_GetKeyConfig();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetKeyConfig(InputConfiguration inputConfig);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsButtonDown(ref VirtualButton button, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsButtonUp(ref VirtualButton button, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsButtonHeld(ref VirtualButton button, int deviceIdx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetAxisValue(ref VirtualAxis button, int deviceIdx);
    };

    /** @} */
}
