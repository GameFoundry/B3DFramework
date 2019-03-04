Cursors					{#cursors}
===============
[TOC]

If developing an application that accepts mouse input, you can control the behaviour of the cursor through the @ref bs::Cursor "Cursor" class, accessible globally through @ref bs::gCursor "gCursor()". It allows you to manipulate cursor position, look and clipping behaviour.

# Position {#cursors_a}
You can retrieve the current position of the cursor by calling @ref bs::Cursor::getScreenPosition "Cursor::getScreenPosition()". Note that this same information is reported by the input system, and is generally preferred to use those values instead.

You can also change the cursor position directly by calling @ref bs::Cursor::setScreenPosition "Cursor::setScreenPosition()". Values for both methods will be in pixels relative to the user's screen (or screens).

~~~~~~~~~~~~~{.cpp}
// Move cursor to the top left corner of the screen
gCursor().setScreenPosition(Vector2I(0, 0));
~~~~~~~~~~~~~

# Visibility {#cursors_b}
Cursor can be hidden by calling @ref bs::Cursor::hide "Cursor::hide()", and shown again by calling @ref bs::Cursor::show "Cursor::show()".

~~~~~~~~~~~~~{.cpp}
// Hide the cursor if user is holding right mouse button (e.g. rotating the camera)
if(gInput().isButtonHeld(BC_MOUSE_RIGHT))
	gCursor().hide();
else
	gCursor().show();	
~~~~~~~~~~~~~

# Icon {#cursors_c}
You can change the cursor's icon by calling @ref bs::Cursor::setCursor(CursorType) "Cursor::setCursor()" and specifying one of the builtin cursor types, as @ref bs::CursorType "CursorType" enum.

~~~~~~~~~~~~~{.cpp}
// Change to wait cursor in case we're doing some processing
gCursor().setCursor(CursorType::Wait);
~~~~~~~~~~~~~

You can also define your own cursor icons by calling @ref bs::Cursor::setCursorIcon(const String&, const PixelData&, const Vector2I&) "Cursor::setCursorIcon()". You'll need to provide a unique name for your cursor, a **PixelData** object containing the image to use, and a cursor *hot-spot*. Hot spot determines at which part of the image will the user's clicks be registered (e.g. in case of an arrow icon, it would be at the top of the arrow).

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = ...; // Manually fill or read pixel data from a texture
Vector2I hotSpot(5, 5);

gCursor().setCursorIcon("MyCustomCursor", *pixelData, hotSpot);
~~~~~~~~~~~~~

Once you have registered the icon you can apply it by calling an overload of @ref bs::Cursor::setCursor(const String&) "Cursor::setCursor()" that accepts a cursor name.

~~~~~~~~~~~~~{.cpp}
gCursor().setCursor("MyCustomCursor");
~~~~~~~~~~~~~

You can also change icons of the built-in cursor types by calling @ref bs::Cursor::setCursorIcon(CursorType, const PixelData&, const Vector2I&) "Cursor::setCursorIcon()" overload that accepts a **CursorType** as its first parameter.

# Clipping {#cursors_d}
Sometimes it is useful to limit the cursor to a specific area of the screen (e.g. if playing in windowed mode its useful to limit the cursor to the window). For this purpose you can use either of these methods:
 - @ref bs::Cursor::clipToWindow "Cursor::clipToWindow()" - Accepts a **RenderWindow** as a parameter, and will limit cursor movement within that window.
 - @ref bs::Cursor::clipToRect "Cursor::clipToRect()" - Accepts an area relative to the user's screen, to which to limit the movement to.

~~~~~~~~~~~~~{.cpp}
// Limit cursor movement to the primary application window
SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
gCursor().clipToWindow(window);
~~~~~~~~~~~~~

When you wish to disable clipping, you can call @ref bs::Cursor::clipDisable "Cursor::clipDisable()".

~~~~~~~~~~~~~{.cpp}
// Remove any limits to cursor movement
gCursor().clipDisable();
~~~~~~~~~~~~~