//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"
#include "Image/BsPixelData.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup Platform-Engine
	 *  @{
	 */

	/**
	 * Allows you to manipulate the platform cursor in various ways.
	 *
	 * @note	Thread safe.
	 */
	class BS_EXPORT Cursor : public Module<Cursor>
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
		void SetScreenPosition(const Vector2I& screenPos);

		/**	Retrieves the cursor position in screen coordinates. */
		Vector2I GetScreenPosition();

		/**	Hides the cursor. */
		void Hide();

		/**	Shows the cursor. */
		void Show();

		/**	Limit cursor movement to the specified window. */
		void ClipToWindow(const RenderWindow& window);

		/**	Limit cursor movement to specific area on the screen. */
		void ClipToRect(const Rect2I& screenRect);

		/**	Disables cursor clipping that was set using any of the clipTo* methods. */
		void ClipDisable();

		/**	Sets a cursor icon. Uses one of the built-in cursor types. */
		void SetCursor(CursorType type);

		/**
		 * Sets a cursor icon. Uses one of the manually registered icons.
		 *
		 * @param[in]	name		The name to identify the cursor, one set previously by calling setCursorIcon().
		 */
		void SetCursor(const String& name);

		/**
		 * Registers a new custom cursor icon you can then set by calling "setCursor".
		 *
		 * @param[in]	name		The name to identify the cursor.
		 * @param[in]	pixelData	Cursor image data.
		 * @param[in]	hotSpot		Offset on the cursor image to where the actual input happens (for example tip of the
		 *							Arrow cursor).
		 *
		 * @note
		 * Stores an internal copy of the pixel data. Clear it by calling removeCursorIcon(). If a custom icon with the
		 * same name already exists it will be replaced.
		 */
		void SetCursorIcon(const String& name, const PixelData& pixelData, const Vector2I& hotSpot);

		/**
		 * Registers a new custom cursor icon you can then set by calling setCursor().
		 *
		 * @param[in]	type		One of the built-in cursor types.
		 * @param[in] 	pixelData	Cursor image data.
		 * @param[in]	hotSpot		Offset on the cursor image to where the actual input happens (for example tip of the
		 *							Arrow cursor).
		 *
		 * @note
		 * Stores an internal copy of the pixel data. Clear it by calling removeCursorIcon(). If a custom icon with the
		 * same type already exists it will be replaced.
		 */
		void SetCursorIcon(CursorType type, const PixelData& pixelData, const Vector2I& hotSpot);

		/**	Removes a custom cursor icon and releases any data associated with it. */
		void ClearCursorIcon(const String& name);

		/**
		 * Removes a custom cursor icon and releases any data associated with it. Restores original icon associated with
		 * this cursor type.
		 */
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
	BS_EXPORT Cursor& GetCursor();

	/** @} */
} // namespace bs
