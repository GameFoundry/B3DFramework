---
title: Cursors
---

If developing an application that accepts mouse input, you can control the behaviour of the cursor through the @b3d::Cursor class, accessible globally through @b3d::GetCursor. It allows you to manipulate cursor position, look and clipping behaviour.

# Position
You can retrieve the current position of the cursor by calling @b3d::Cursor::GetScreenPosition. Note that this same information is reported by the input system, and is generally preferred to use those values instead.

You can also change the cursor position directly by calling @b3d::Cursor::SetScreenPosition. Values for both methods will be in pixels relative to the user's screen (or screens).

~~~~~~~~~~~~~{.cpp}
// Move cursor to the top left corner of the screen
GetCursor().SetScreenPosition(Vector2I(0, 0));
~~~~~~~~~~~~~

# Visibility
Cursor can be hidden by calling @b3d::Cursor::Hide, and shown again by calling @b3d::Cursor::Show.

~~~~~~~~~~~~~{.cpp}
// Hide the cursor if user is holding right mouse button (e.g. rotating the camera)
if(GetInput().IsButtonHeld(BC_MOUSE_RIGHT))
	GetCursor().Hide();
else
	GetCursor().Show();
~~~~~~~~~~~~~

# Icon
You can change the cursor's icon by calling @b3d::Cursor::SetCursor(CursorType) and specifying one of the builtin cursor types, as @b3d::CursorType enum.

~~~~~~~~~~~~~{.cpp}
// Change to wait cursor in case we're doing some processing
GetCursor().SetCursor(CursorType::Wait);
~~~~~~~~~~~~~

You can also define your own cursor icons by calling @b3d::Cursor::SetCursorIcon(const String&, const PixelData&, const Vector2I&). You'll need to provide a unique name for your cursor, a **PixelData** object containing the image to use, and a cursor *hot-spot*. Hot spot determines at which part of the image will the user's clicks be registered (e.g. in case of an arrow icon, it would be at the top of the arrow).

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> cursorPixelData = ...; // Manually fill or read pixel data from a texture
Vector2I hotSpotPosition(5, 5);

GetCursor().SetCursorIcon("MyCustomCursor", *cursorPixelData, hotSpotPosition);
~~~~~~~~~~~~~

Once you have registered the icon you can apply it by calling an overload of @b3d::Cursor::SetCursor(const String&) that accepts a cursor name.

~~~~~~~~~~~~~{.cpp}
GetCursor().SetCursor("MyCustomCursor");
~~~~~~~~~~~~~

You can also change icons of the built-in cursor types by calling @b3d::Cursor::SetCursorIcon(CursorType, const PixelData&, const Vector2I&) overload that accepts a **CursorType** as its first parameter.

# Clipping
Sometimes it is useful to limit the cursor to a specific area of the screen (e.g. if playing in windowed mode its useful to limit the cursor to the window). For this purpose you can use either of these methods:
 - @b3d::Cursor::ClipToWindow - Accepts a **RenderWindow** as a parameter, and will limit cursor movement within that window.
 - @b3d::Cursor::ClipToRect - Accepts an area relative to the user's screen, to which to limit the movement to.

~~~~~~~~~~~~~{.cpp}
// Limit cursor movement to the primary application window
TShared<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
GetCursor().ClipToWindow(primaryWindow);
~~~~~~~~~~~~~

When you wish to disable clipping, you can call @b3d::Cursor::ClipDisable.

~~~~~~~~~~~~~{.cpp}
// Remove any limits to cursor movement
GetCursor().ClipDisable();
~~~~~~~~~~~~~
