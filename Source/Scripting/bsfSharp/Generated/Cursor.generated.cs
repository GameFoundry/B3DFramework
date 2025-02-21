//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Platform
	 *  @{
	 */

	/// <summary>Allows you to manipulate the platform cursor in various ways.</summary>
	[ShowInInspector]
	public partial class Cursor : ScriptObject
	{
		private Cursor(bool __dummy0) { }
		protected Cursor() { }

		/// <summary>Moves the cursor to the specified screen position.</summary>
		public static void SetScreenPosition(TVector2<TUnitValue<int,PhysicalPixel>> screenPos)
		{
			Internal_SetScreenPosition(ref screenPos);
		}

		/// <summary>Retrieves the cursor position in screen coordinates.</summary>
		public static TVector2<TUnitValue<int,PhysicalPixel>> GetScreenPosition()
		{
			TVector2<TUnitValue<int,PhysicalPixel>> temp;
			Internal_GetScreenPosition(out temp);
			return temp;
		}

		/// <summary>Hides the cursor.</summary>
		public static void Hide()
		{
			Internal_Hide();
		}

		/// <summary>Shows the cursor.</summary>
		public static void Show()
		{
			Internal_Show();
		}

		/// <summary>Limit cursor movement to specific area on the screen.</summary>
		public static void ClipToRect(Rect2I screenRect)
		{
			Internal_ClipToRect(ref screenRect);
		}

		/// <summary>Disables cursor clipping that was set using any of the clipTo* methods.</summary>
		public static void ClipDisable()
		{
			Internal_ClipDisable();
		}

		/// <summary>Sets a cursor icon. Uses one of the built-in cursor types.</summary>
		public static void SetCursor(CursorType type)
		{
			Internal_SetCursor(type);
		}

		/// <summary>Sets a cursor icon. Uses one of the manually registered icons.</summary>
		/// <param name="name">The name to identify the cursor, one set previously by calling setCursorIcon().</param>
		public static void SetCursor(string name)
		{
			Internal_SetCursor0(name);
		}

		/// <summary>Registers a new custom cursor icon you can then set by calling &quot;setCursor&quot;.</summary>
		/// <param name="name">The name to identify the cursor.</param>
		/// <param name="pixelData">Cursor image data.</param>
		/// <param name="hotSpot">
		/// Offset on the cursor image to where the actual input happens (for example tip of the Arrow cursor).
		/// </param>
		public static void SetCursorIcon(string name, PixelData pixelData, TVector2<int> hotSpot)
		{
			Internal_SetCursorIcon(name, pixelData, ref hotSpot);
		}

		/// <summary>Registers a new custom cursor icon you can then set by calling setCursor().</summary>
		/// <param name="type">One of the built-in cursor types.</param>
		/// <param name="pixelData">Cursor image data.</param>
		/// <param name="hotSpot">
		/// Offset on the cursor image to where the actual input happens (for example tip of the Arrow cursor).
		/// </param>
		public static void SetCursorIcon(CursorType type, PixelData pixelData, TVector2<int> hotSpot)
		{
			Internal_SetCursorIcon0(type, pixelData, ref hotSpot);
		}

		/// <summary>Removes a custom cursor icon and releases any data associated with it.</summary>
		public static void ClearCursorIcon(string name)
		{
			Internal_ClearCursorIcon(name);
		}

		/// <summary>
		/// Removes a custom cursor icon and releases any data associated with it. Restores original icon associated with this 
		/// cursor type.
		/// </summary>
		public static void ClearCursorIcon(CursorType type)
		{
			Internal_ClearCursorIcon0(type);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScreenPosition(ref TVector2<TUnitValue<int,PhysicalPixel>> screenPos);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetScreenPosition(out TVector2<TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Hide();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Show();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClipToRect(ref Rect2I screenRect);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClipDisable();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCursor(CursorType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCursor0(string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCursorIcon(string name, PixelData pixelData, ref TVector2<int> hotSpot);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCursorIcon0(CursorType type, PixelData pixelData, ref TVector2<int> hotSpot);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClearCursorIcon(string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClearCursorIcon0(CursorType type);
	}

	/** @} */
}
