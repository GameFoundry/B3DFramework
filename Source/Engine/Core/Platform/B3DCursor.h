//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIUnits.h"
#include "Utility/B3DModule.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	/** @addtogroup Platform
	 *  @{
	 */

	/**
	 * Allows you to manipulate the platform cursor in various ways.
	 *
	 * @note	Thread safe.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Platform)) Cursor : public Module<Cursor>
	{
		/**	Internal container for data about a single cursor icon. */
		struct CustomIcon
		{
			CustomIcon() = default;

			CustomIcon(const PixelData& pixelData, const Vector2I& hotSpot)
				: HotSpot(hotSpot), PixelData(pixelData)
			{}

			Vector2I HotSpot;
			PixelData PixelData;
		};

	public:
		Cursor();

		/**	Moves the cursor to the specified screen position. */
		B3D_SCRIPT_EXPORT()
		void SetScreenPosition(const GUIPhysicalPoint& screenPos);

		/**	Retrieves the cursor position in screen coordinates. */
		B3D_SCRIPT_EXPORT()
		GUIPhysicalPoint GetScreenPosition() const;

		/**	Hides the cursor. */
		B3D_SCRIPT_EXPORT()
		void Hide();

		/**	Shows the cursor. */
		B3D_SCRIPT_EXPORT()
		void Show();

		/**	Limit cursor movement to the specified window. */
		void ClipToWindow(const RenderWindow& window);

		/**	Limit cursor movement to specific area on the screen. */
		B3D_SCRIPT_EXPORT()
		void ClipToRect(const Area2I& screenRect);

		/**	Disables cursor clipping that was set using any of the clipTo* methods. */
		B3D_SCRIPT_EXPORT()
		void ClipDisable();

		/**	Sets a cursor icon. Uses one of the built-in cursor types. */
		B3D_SCRIPT_EXPORT()
		void SetCursor(CursorType type);

		/**
		 * Sets a cursor icon. Uses one of the manually registered icons.
		 *
		 * @param	name	The name to identify the cursor, one set previously by calling setCursorIcon().
		 */
		B3D_SCRIPT_EXPORT()
		void SetCursor(const String& name);

		/**
		 * Registers a new custom cursor icon you can then set by calling "setCursor".
		 *
		 * @param	name		The name to identify the cursor.
		 * @param	pixelData	Cursor image data.
		 * @param	hotSpot		Offset on the cursor image to where the actual input happens (for example tip of the
		 *						Arrow cursor).
		 *
		 * @note
		 * Stores an internal copy of the pixel data. Clear it by calling removeCursorIcon(). If a custom icon with the
		 * same name already exists it will be replaced.
		 */
		B3D_SCRIPT_EXPORT()
		void SetCursorIcon(const String& name, const PixelData& pixelData, const Vector2I& hotSpot);

		/**
		 * Registers a new custom cursor icon you can then set by calling setCursor().
		 *
		 * @param	type		One of the built-in cursor types.
		 * @param	pixelData	Cursor image data.
		 * @param	hotSpot		Offset on the cursor image to where the actual input happens (for example tip of the
		 *						Arrow cursor).
		 *
		 * @note
		 * Stores an internal copy of the pixel data. Clear it by calling removeCursorIcon(). If a custom icon with the
		 * same type already exists it will be replaced.
		 */
		B3D_SCRIPT_EXPORT()
		void SetCursorIcon(CursorType type, const PixelData& pixelData, const Vector2I& hotSpot);

		/**	Removes a custom cursor icon and releases any data associated with it. */
		B3D_SCRIPT_EXPORT()
		void ClearCursorIcon(const String& name);

		/**
		 * Removes a custom cursor icon and releases any data associated with it. Restores original icon associated with
		 * this cursor type.
		 */
		B3D_SCRIPT_EXPORT()
		void ClearCursorIcon(CursorType type);

	private:
		/**	Restores the default cursor icon for the specified cursor type. */
		void RestoreCursorIcon(CursorType type);

		/**	Sends the cursor image to the OS, making it active. */
		void UpdateCursorImage();

		UnorderedMap<String, u32> mCustomIconNameToId;
		UnorderedMap<u32, CustomIcon> mCustomIcons;
		u32 mNextUniqueId = (u32)CursorType::Count;
		i32 mActiveCursorId = -1;
	};

	/** Easy way to access Cursor. */
	B3D_EXPORT Cursor& GetCursor();

	/** @} */
} // namespace b3d
