---
title: Input events
---

Events represent another way of handling user input. They are an alternative to input polling, and it's up to the developer to choose which way of handling input they prefer. These approaches aren't identical though, and events can provide more information than polling. Same as polling, events are also handled by the **Input** class.

This approach uses the event system, on which you can read more on [here](../15_Utilities/03_events.md).

# Button presses
You can subscribe to the following events that report when the user interacted with a button:
 - @b3d::Input::OnButtonDown - Triggered whenever a button has been pressed.
 - @b3d::Input::OnButtonUp - Triggered whenever a button has been released.

Both of these events supply the @b3d::ButtonEvent structure, containing the code of the button that was pressed, along with some other information.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Callback method that triggers when any button is pressed
auto handleButtonDown = [&](const ButtonEvent& event)
{
	// If user presses space, "jump"
	if (event.ButtonCode == ButtonCode::Space)
		position.y += 5.0f;
};

// Connect the callback to the event
GetInput().OnButtonDown.Connect(handleButtonDown);
~~~~~~~~~~~~~

# Mouse/touch input
Use @b3d::Input::OnPointerMoved to track whenever the user moves the mouse or their finger on a touch device. The event supplies the @b3d::PointerEvent structure, containing information like screen position of the event, delta from the last frame, state of all the mouse buttons, scroll wheel movement and more.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Callback method that triggers whenever the pointer moves
auto handlePointerMove = [&](const PointerEvent& event)
{
	// Move the object in 2D plane together with the pointer, if left mouse button is pressed
	if (event.ButtonStates[(int)PointerEventButton::Left])
	{
		position.x = (float)event.ScreenPosition.x;
		position.y = (float)event.ScreenPosition.y;
	}

	// Change object depth depending on mouse scroll wheel
	position.z += event.MouseWheelScrollAmount;
};

// Connect the callback to the event
GetInput().OnPointerMoved.Connect(handlePointerMove);
~~~~~~~~~~~~~

Pointers may also receive specialized button down/up events, similar to **Input::OnButtonDown** and **Input::OnButtonUp**. They trigger at the same time, but provide **PointerEvent** structure instead of **ButtonEvent** - which may be more useful in certain situations. These methods are:
 - @b3d::Input::OnPointerPressed - Triggered whenever a pointer button has been pressed or screen touch began.
 - @b3d::Input::OnPointerReleased - Triggered whenever a pointer button has been released or screen touch ended.
 - @b3d::Input::OnPointerDoubleClick - Triggered when the user quickly clicks the pointer buttons or taps the screen in succession.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Callback method that triggers on double click
auto handleDoubleClick = [&](const PointerEvent& event)
{
	// Jump on double click
	position.y += 5.0f;
};

// Connect the callback to the event
GetInput().OnPointerDoubleClick.Connect(handleDoubleClick);
~~~~~~~~~~~~~

# Text input
If a user is typing text (using a physical or a touch keyboard) you may subscribe to @b3d::Input::OnCharInput to receive individual characters as the user inputs them.

~~~~~~~~~~~~~{.cpp}
StringStream inputString;

// Callback method that appends a character to the inputString stream
auto handleCharInput = [&](const TextInputEvent& event)
{
	inputString << (char)event.TextChar;
};

// Connect the callback to the event
GetInput().OnCharInput.Connect(handleCharInput);
~~~~~~~~~~~~~

Note that the system will register keyboard buttons as both text input and as normal button presses - it's up to the caller to decide which to process when. If keyboard is used for gameplay then button presses should be used, but if a user is actually typing text, then character input is better suited. This is because button events report button codes as physical keyboard keys, yet character input will actually translate those physical key presses into character codes depending on the user's keyboard settings, which ensures non-english keyboard layouts work as intended.
