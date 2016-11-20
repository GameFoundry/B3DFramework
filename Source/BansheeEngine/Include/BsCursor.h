//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsPixelData.h"
#include "BsVector2I.h"

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
			CustomIcon() {}
			CustomIcon(const PixelData& pixelData, const Vector2I& hotSpot)
				:hotSpot(hotSpot), pixelData(pixelData)
			{ }

			Vector2I hotSpot;
			PixelData pixelData;
		};

	public:
		Cursor();

		/**	Moves the cursor to the specified screen position. */
		void setScreenPosition(const Vector2I& screenPos);

		/**	Retrieves the cursor position in screen coordinates. */
		Vector2I getScreenPosition();

		/**	Hides the cursor. */
		void hide();

		/**	Shows the cursor. */
		void show();

		/**	Limit cursor movement to the specified window. */
		void clipToWindow(const RenderWindow& window);

		/**	Limit cursor movement to specific area on the screen. */
		void clipToRect(const Rect2I& screenRect);

		/**	Disables cursor clipping that was set using any of the clipTo* methods. */
		void clipDisable();
		
		/**	Sets a cursor icon. Uses one of the built-in cursor types. */
		void setCursor(CursorType type);

		/**
		 * Sets a cursor icon. Uses one of the manually registered icons.
		 * 			
		 * @param[in]	name		The name to identify the cursor, one set previously by calling setCursorIcon().
		 */
		void setCursor(const String& name);

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
		void setCursorIcon(const String& name, const PixelData& pixelData, const Vector2I& hotSpot);

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
		void setCursorIcon(CursorType type, const PixelData& pixelData, const Vector2I& hotSpot);

		/**	Removes a custom cursor icon and releases any data associated with it. */
		void clearCursorIcon(const String& name);

		/**
		 * Removes a custom cursor icon and releases any data associated with it. Restores original icon associated with
		 * this cursor type.
		 */
		void clearCursorIcon(CursorType type);

	private:
		/**	Restores the default cursor icon for the specified cursor type. */
		void restoreCursorIcon(CursorType type);

		/**	Sends the cursor image to the OS, making it active. */
		void updateCursorImage();

		UnorderedMap<String, UINT32> mCustomIconNameToId;
		UnorderedMap<UINT32, CustomIcon> mCustomIcons;
		UINT32 mNextUniqueId;
		INT32 mActiveCursorId;
	};

	/** @} */
}