//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
    /// Allows manipulation of the platform cursor.
    /// </summary>
    public static class Cursor
    {
        /// <summary>
        /// Position of the cursor in screen coordinates.
        /// </summary>
        public static Vector2I ScreenPosition
        {
            get
            {
                Vector2I value;
                Internal_GetScreenPosition(out value);
                return value;
            }

            set
            {
                Internal_SetScreenPosition(ref value);
            }
        }

        /// <summary>
        /// Hides the cursor.
        /// </summary>
        public static void Hide()
        {
            Internal_Hide();
        }

        /// <summary>
        /// Shows the cursor.
        /// </summary>
        public static void Show()
        {
            Internal_Show();
        }

        /// <summary>
        /// Clips the cursor to the specified area. Enabled until <see cref="ClipDisable"/> is called.
        /// </summary>
        /// <param name="area">Area in screen space to clip the cursor to.</param>
        public static void ClipToRect(Rect2I area)
        {
            Internal_ClipToRect(ref area);
        }

        /// <summary>
        /// Disables cursor clipping previously enabled with <see cref="ClipToRect"/>.
        /// </summary>
        public static void ClipDisable()
        {
            Internal_ClipDisable();
        }

        /// <summary>
        /// Changes the active cursor icon.
        /// </summary>
        /// <param name="name">Name of the cursor icon, previously registered with 
        ///                    <see cref="SetCursorIcon(string,PixelData,Vector2I)"/></param>
        public static void SetCursor(string name)
        {
            Internal_SetCursorStr(name);
        }

        /// <summary>
        /// Changes the active cursor icon.
        /// </summary>
        /// <param name="type">One of the built-in cursor types.</param>
        public static void SetCursor(CursorType type)
        {
            Internal_SetCursor(type);
        }

        /// <summary>
        /// Updates the look of a specific cursor icon.
        /// </summary>
        /// <param name="name">Name of the cursor.</param>
        /// <param name="iconData">Pixel data specifying the new look.</param>
        /// <param name="hotspot">Offset into the icon image that determines where the cursor point is.</param>
        public static void SetCursorIcon(string name, PixelData iconData, Vector2I hotspot)
        {
            Internal_SetCursorIconStr(name, iconData, ref hotspot);
        }

        /// <summary>
        /// Updates the look of a specific cursor icon.
        /// </summary>
        /// <param name="type">One of the built-in cursor types.</param>
        /// <param name="iconData">Pixel data specifying the new look.</param>
        /// <param name="hotspot">Offset into the icon image that determines where the cursor point is.</param>
        public static void SetCursorIcon(CursorType type, PixelData iconData, Vector2I hotspot)
        {
            Internal_SetCursorIcon(type, iconData, ref hotspot);
        }

        /// <summary>
        /// Removes a cursor icon.
        /// </summary>
        /// <param name="name">Name of the cursor.</param>
        public static void ClearCursorIcon(string name)
        {
            Internal_ClearCursorIconStr(name);
        }

        /// <summary>
        /// Removes a cursor icon.
        /// </summary>
        /// <param name="type">One of the built-in cursor types.</param>
        public static void ClearCursorIcon(CursorType type)
        {
            Internal_ClearCursorIcon(type);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetScreenPosition(out Vector2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetScreenPosition(ref Vector2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Hide();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Show();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClipToRect(ref Rect2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClipDisable();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCursorStr(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCursor(CursorType cursor);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCursorIconStr(string name, PixelData iconData, ref Vector2I hotspot);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCursorIcon(CursorType cursor, PixelData iconData, ref Vector2I hotspot);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClearCursorIconStr(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClearCursorIcon(CursorType cursor);
    }

    /// <summary>
    /// Built-in cursor types.
    /// </summary>
    public enum CursorType //Note: Must match C++ enum CursorType
	{
		Arrow,
		ArrowDrag,
		ArrowLeftRight,
		Wait,
		IBeam,
		SizeNESW,
		SizeNS,
		SizeNWSE,
		SizeWE,
		Deny,

		// Keep at the end
		Count
	};

    /** @} */
}
