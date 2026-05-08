---
title: Virtual input
---

Virtual input is a high-level input system, built on top of existing **Input** functionality shown earlier. The main difference between the two systems is that virtual input abstracts the concept of buttons and axes into virtual objects, instead of referencing them directly.

This allows the application to use virtual buttons and axes without needing to know actual hardware buttons/axes the user is using. This means same virtual buttons can be used for multiple input devices (e.g. keyboard & gamepad), as well as allow the user to manually re-map keys with no additional gameplay logic.

All virtual input is handled through the @b3d::VirtualInput class, accessible through @b3d::GetVirtualInput. You'll notice it shares a very similar interface with **Input**, with the only difference being how we represent buttons and axes.

Before we explain individual aspects, let's see a quick working example to give you a rough idea:
~~~~~~~~~~~~~{.cpp}
// Set up input configuration that maps virtual keys to actual hardware keys
TShared<InputConfiguration> inputConfiguration = GetVirtualInput().GetConfiguration();

// Virtual button named "Forward" maps to W and Up arrow keys
inputConfiguration->RegisterButton("Forward", ButtonCode::W);
inputConfiguration->RegisterButton("Forward", ButtonCode::ArrowUp);

// (Somewhere else in the app) Use the virtual key
VirtualButton forwardKey = VirtualInput::GetOrCreateVirtualButton("Forward");

if (GetVirtualInput().IsButtonDown(forwardKey))
	B3D_LOG(Info, LogGeneric, "Moving forward...");
~~~~~~~~~~~~~

# Input configuration
Before we can use the virtual input system, we must first create a set of virtual buttons and axes, name them, and map them to actual hardware keys. To do this we require an @b3d::InputConfiguration object, which can be retrieved from **VirtualInput** by calling @b3d::VirtualInput::GetConfiguration.

~~~~~~~~~~~~~{.cpp}
TShared<InputConfiguration> inputConfiguration = GetVirtualInput().GetConfiguration();
~~~~~~~~~~~~~

# Virtual buttons
## Registration
Virtual buttons can be registered by giving them a unique name, and a hardware button code they map to. Any time a particular hardware button is pressed, the virtual button will be reported as pressed as well. New buttons are registered with @b3d::InputConfiguration::RegisterButton.

~~~~~~~~~~~~~{.cpp}
// Register a virtual button named "Forward" that maps to W and Up arrow keys
inputConfiguration->RegisterButton("Forward", ButtonCode::W);
inputConfiguration->RegisterButton("Forward", ButtonCode::ArrowUp);
~~~~~~~~~~~~~

You can also unregister an existing button by calling @b3d::InputConfiguration::UnregisterButton.

~~~~~~~~~~~~~{.cpp}
inputConfiguration->UnregisterButton("Forward");
~~~~~~~~~~~~~

These mappings can be registered/unregistered during runtime, meaning you should use this functionality to provide input remapping for your users.

## Usage
Once your virtual button has been registered you can use it by creating a @b3d::VirtualButton object using the @b3d::VirtualInput::GetOrCreateVirtualButton method with the button name you provided when registering the button.

~~~~~~~~~~~~~{.cpp}
VirtualButton forwardKey = VirtualInput::GetOrCreateVirtualButton("Forward");
~~~~~~~~~~~~~

> It is preferable you create virtual buttons during start-up and save them for later use, instead of creating them every time you use them.

Created button can be used in **VirtualInput** with following events, similar to **Input** events:
 - @b3d::VirtualInput::OnButtonDown - Triggered whenever a button has been pressed.
 - @b3d::VirtualInput::OnButtonUp - Triggered whenever a button has been released.
 - @b3d::VirtualInput::OnButtonHeld - Triggered every frame while a button is being held.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Callback method that triggers when any virtual button is being held down
auto handleButtonHeld = [&](const VirtualButton& button, u32 deviceIndex)
{
	// If user holds down W or Up arrow, move forward
	if (button == forwardKey)
		position.z += 5.0f;
};

// Connect the callback to the event
GetVirtualInput().OnButtonHeld.Connect(handleButtonHeld);
~~~~~~~~~~~~~

And you can also use the following polling methods. Again, similar to **Input**:
 - @b3d::VirtualInput::IsButtonDown - Checks if the button was pressed this frame. Only valid for one frame.
 - @b3d::VirtualInput::IsButtonHeld - Checks if the button is currently being held. This is valid for the first frame the button is pressed, and for any following frame until it is released.
 - @b3d::VirtualInput::IsButtonUp - Checks if the button was released this frame. Only valid for one frame.

~~~~~~~~~~~~~{.cpp}
Vector3 position(Vector3::kZero);

// Move 5 units forward for every frame while W or Up arrow is pressed
if (GetVirtualInput().IsButtonHeld(forwardKey))
	position.z += 5.0f;
~~~~~~~~~~~~~

# Virtual axes
## Registration
Virtual axes allow you to map hardware axes (e.g. gamepad analog stick or mouse movement) to virtual axes. They are registered similarly to buttons, through **InputConfiguration** by calling @b3d::InputConfiguration::RegisterAxis.

You are required to give it a unique name, and fill out @b3d::VirtualAxisCreateInformation structure that describes the axis. The structure allows you to choose which hardware axes to reference, as well as set other properties like sensitivity, inversion or dead zones.

~~~~~~~~~~~~~{.cpp}
// Map gamepad right stick X axis and mouse X axis to a virtual axis for looking left/right
VirtualAxisCreateInformation axisDescription;
axisDescription.Type = (int)InputAxis::RightStickX | (int)InputAxis::MouseX;

inputConfiguration->RegisterAxis("LookLeftRight", axisDescription);
~~~~~~~~~~~~~

> Note that unlike with buttons you shouldn't call **InputConfiguration::RegisterAxis** multiple times for the same virtual axis. Instead provide all hardware axes in the **VirtualAxisCreateInformation::Type** by ORing them together.

Existing virtual axes can be unmapped by calling @b3d::InputConfiguration::UnregisterAxis.

~~~~~~~~~~~~~{.cpp}
inputConfiguration->UnregisterAxis("LookLeftRight");
~~~~~~~~~~~~~

## Usage
Once you wish to use the virtual axis you construct a @b3d::VirtualAxis object using the @b3d::VirtualInput::GetOrCreateVirtualAxis method by providing it with the name of the axis.
~~~~~~~~~~~~~{.cpp}
VirtualAxis lookLeftRightAxis = VirtualInput::GetOrCreateVirtualAxis("LookLeftRight");
~~~~~~~~~~~~~

Then you can use @b3d::VirtualInput::GetAxisValue to retrieve the current value of the axis.

~~~~~~~~~~~~~{.cpp}
// Rotate the camera left/right depending on the axis
Degree lookAngle(0.0f);

//...

lookAngle += (Degree)GetVirtualInput().GetAxisValue(lookLeftRightAxis);
~~~~~~~~~~~~~
