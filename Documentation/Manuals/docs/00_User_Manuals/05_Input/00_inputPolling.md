---
title: Input polling
---

Input polling refers to the process of querying the input system to check if a user interacted with an input device in some way. This may include checking if the user pressed a keyboard, mouse or a gamepad button or moved the mouse or an analog axis.

All the input is handled through the @b3d::Input class, which can be globally accessed through the @b3d::GetInput method.

# Button presses
Use the following methods to check if a button has been pressed, released or is currently being held down:
 - @b3d::Input::IsButtonDown - Checks if the button was pressed this frame. Only valid for one frame.
 - @b3d::Input::IsButtonHeld - Checks if the button is currently being held. This is valid for the first frame the button is pressed, and for any following frame until it is released.
 - @b3d::Input::IsButtonUp - Checks if the button was released this frame. Only valid for one frame.

These methods work on any kind of input device buttons, including keyboard, gamepad and mouse. Use @b3d::ButtonCode to choose which button to query for.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Move 5 units forward for every frame while W key is pressed
if (GetInput().IsButtonHeld(ButtonCode::W))
	position.z += 5.0f;
~~~~~~~~~~~~~

# Analog input
Moving the mouse, gamepad sticks or triggers results in an analog input. While buttons can only be toggled on or off, analog input is received in a specific range (for example, anywhere between -1 and 1). This allows more precise input for applications that require it.

Analog input is represented through the concept of *axes*. Use @b3d::Input::GetAxisValue to get a value for a specific axis. Check @b3d::InputAxis for a list of all supported axes.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Move forward or backwards depending on how much the user moves the left gamepad stick
position.z += GetInput().GetAxisValue(InputAxis::LeftStickY);
~~~~~~~~~~~~~

Most axes report their input in range [-1, 1], with the exception of mouse axes, which are unbound.

# Mouse input
Often it is useful to receive mouse position directly, rather than dealing with raw mouse axis data. Use @b3d::Input::GetPointerPosition to retrieve the current position of the mouse cursor, in coordinates relative to the screen.

Use @b3d::Input::GetPointerDelta to get the difference in coordinates between the position of the mouse on the previous and current frame.

~~~~~~~~~~~~~{.cpp}
Vector2I screenPosition = GetInput().GetPointerPosition();
~~~~~~~~~~~~~

You can also check if the left mouse button has been double-clicked by checking @b3d::Input::IsPointerDoubleClicked.

~~~~~~~~~~~~~{.cpp}
if (GetInput().IsPointerDoubleClicked())
	B3D_LOG(Info, LogGeneric, "Mouse double-clicked!");
~~~~~~~~~~~~~
