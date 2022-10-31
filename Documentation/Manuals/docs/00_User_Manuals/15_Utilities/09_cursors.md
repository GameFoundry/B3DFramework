---
title: Cursors
---

If developing an application that accepts mouse input, you can control the behaviour of the cursor through the @bs::Cursor class, accessible globally through @bs::GetCursor. It allows you to manipulate cursor position, look and clipping behaviour.

# Position
You can retrieve the current position of the cursor by calling @bs::Cursor::getScreenPosition. Note that this same information is reported by the input system, and is generally preferred to use those values instead.

You can also change the cursor position directly by calling @bs::Cursor::setScreenPosition. Values for both methods will be in pixels relative to the user's screen (or screens).

~~~~~~~~~~~~~{.cpp}
// Move cursor to the top left corner of the screen
GetCursor().setScreenPosition(Vector2I(0, 0));
~~~~~~~~~~~~~

# Visibility
Cursor can be hidden by calling @bs::Cursor::hide, and shown again by calling @bs::Cursor::show.

~~~~~~~~~~~~~{.cpp}
// Hide the cursor if user is holding right mouse button (e.g. rotating the camera)
if(GetInput().isButtonHeld(BC_MOUSE_RIGHT))
	GetCursor().hide();
else
	GetCursor().show();	
~~~~~~~~~~~~~

# Icon
You can change the cursor's icon by calling @bs::Cursor::setCursor(CursorType) and specifying one of the builtin cursor types, as @bs::CursorType enum.

~~~~~~~~~~~~~{.cpp}
// Change to wait cursor in case we're doing some processing
GetCursor().setCursor(CursorType::Wait);
~~~~~~~~~~~~~

You can also define your own cursor icons by calling @bs::Cursor::setCursorIcon(const String&, const PixelData&, const Vector2I&). You'll need to provide a unique name for your cursor, a **PixelData** object containing the image to use, and a cursor *hot-spot*. Hot spot determines at which part of the image will the user's clicks be registered (e.g. in case of an arrow icon, it would be at the top of the arrow).

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = ...; // Manually fill or read pixel data from a texture
Vector2I hotSpot(5, 5);

GetCursor().setCursorIcon("MyCustomCursor", *pixelData, hotSpot);
~~~~~~~~~~~~~

Once you have registered the icon you can apply it by calling an overload of @bs::Cursor::setCursor(const String&) that accepts a cursor name.

~~~~~~~~~~~~~{.cpp}
GetCursor().setCursor("MyCustomCursor");
~~~~~~~~~~~~~

You can also change icons of the built-in cursor types by calling @bs::Cursor::setCursorIcon(CursorType, const PixelData&, const Vector2I&) overload that accepts a **CursorType** as its first parameter.

# Clipping
Sometimes it is useful to limit the cursor to a specific area of the screen (e.g. if playing in windowed mode its useful to limit the cursor to the window). For this purpose you can use either of these methods:
 - @bs::Cursor::clipToWindow - Accepts a **RenderWindow** as a parameter, and will limit cursor movement within that window.
 - @bs::Cursor::clipToRect - Accepts an area relative to the user's screen, to which to limit the movement to.

~~~~~~~~~~~~~{.cpp}
// Limit cursor movement to the primary application window
SPtr<RenderWindow> window = GetApplication().getPrimaryWindow();
GetCursor().clipToWindow(window);
~~~~~~~~~~~~~

When you wish to disable clipping, you can call @bs::Cursor::clipDisable.

~~~~~~~~~~~~~{.cpp}
// Remove any limits to cursor movement
GetCursor().clipDisable();
~~~~~~~~~~~~~
