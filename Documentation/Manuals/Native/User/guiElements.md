GUI elements									{#guiElements}
===============

A GUI element is a basic primitive GUI is constructed out of. They can be text, buttons, input boxes, images, scroll areas and more. We'll explain what the individual GUI element types are later, but initially we'll focus on functionality common to all GUI elements.

# Displaying a GUI element
In order to display a GUI element we must first create it. All GUI elements are created using a static *create* method.

~~~~~~~~~~~~~{.cpp}
// GUILabel is a type of GUI element that displays the provided text on screen
GUILabel* label = GUILabel::create(HString(L"Hello!"));
~~~~~~~~~~~~~

But just creating the element is not enough. We must also register it with our **GUIWidget**. To do that we must first retrieve the primary @ref bs::GUIPanel "GUIPanel" from the widget. **GUIPanel** serves as an element container, and by default every widget has one. Use @ref bs::CGUIWidget::getPanel() "CGUIWidget::getPanel()" to retrieve the panel.

~~~~~~~~~~~~~{.cpp}
GUIPanel* mainPanel = gui->getPanel();
~~~~~~~~~~~~~

**GUIPanel** is a special type of a GUI element called a "layout". We'll discuss layouts in more detail in the next chapter, but for now all you need to know is that they are element containers you can add and remove other GUI elements to/from.

To add an element to the panel use @ref bs::GUIPanel::addElement "GUIPanel::addElement".

~~~~~~~~~~~~~{.cpp}
mainPanel->addElement(label);
~~~~~~~~~~~~~

At this point our GUI element will be displayed.

![Simple GUI](guiBasic.png) 

# Customizing GUI elements
All GUI elements share a common set of methods you can use to customize their position, size, color and other properties.

## Changing position
You can change the position of a GUI element by calling @ref bs::GUIElement::setPosition "GUIElement::setPosition". The position is in pixels, relative to the top left corner of the render target.

~~~~~~~~~~~~~{.cpp}
// Moves the displayed text to coordinates (50, 50)
label->setPosition(50, 50);
~~~~~~~~~~~~~

## Changing size
Element size can be changed by calling @ref bs::GUIElement::setSize "GUIElement::setSize".

~~~~~~~~~~~~~{.cpp}
// Make the label 30 pixels high, and 100 pixels wide
label->setSize(100, 30);
~~~~~~~~~~~~~

You can also set both position and size at the same time by calling @ref bs::GUIElement::setBounds "GUIElement::setBounds".

~~~~~~~~~~~~~{.cpp}
// Make the label 30 pixels high, and 100 pixels wide, and position it at (50, 50)
label->setBounds(Rect2I(50, 50, 100, 30));
~~~~~~~~~~~~~

## Changing color
You can change the tint of the GUI element with @ref bs::GUIElement::setTint "GUIElement::setTint". By default an all-white tint is used for all elements. 

~~~~~~~~~~~~~{.cpp}
// Make the label text green
label->setTint(Color::Green);
~~~~~~~~~~~~~

## Hiding
You can temporarily hide an element with @ref bs::GUIElement::setVisible "GUIElement::setVisible". As the name implies hidden element will not be displayed, and cannot be interacted with.

~~~~~~~~~~~~~{.cpp}
// Hide the label
label->setVisible(false);

// Show the label
label->setVisible(true);
~~~~~~~~~~~~~

# GUI element types
Banshee provides a large library of existing GUI element types. We'll focus on explaining the most important ones, but you can find an exhaustive list in @ref GUI.

## Label
A label is the most basic of GUI elements, that allows no user interaction and just displays a textual string. It is created with @ref bs::GUILabel::create "GUILabel::create", which accepts a string as input.

~~~~~~~~~~~~~{.cpp}
GUILabel* label = GUILabel::create(HString(L"Hello!"));
mainPanel->addElement(label);
~~~~~~~~~~~~~

Once created you can optionally change the displayed text with @ref bs::GUILabel::setContent "GUILabel::setContent".

~~~~~~~~~~~~~{.cpp}
label->setContent(HString(L"New text!"));
~~~~~~~~~~~~~

> You can use *setContent* function on most GUI elements, so we won't mention it further for each individual element.

![Label](guiLabel.png) 

## Texture
A texture is another basic GUI element that allows no interaction. All it does is display a **SpriteTexture** on the screen.

To create a GUI texture element, call @ref bs::GUITexture::create "GUITexture::create" which accepts three parameters: 
 - **SpriteTexture** - Determines which texture to draw.
 - @ref bs::TextureScaleMode "TextureScaleMode" - Determines how to scale the texture in the available area.
 - Transparency flag - Should transparency be enabled, allowing elements behind the texture to render.

~~~~~~~~~~~~~{.cpp}
// Create a sprite texture for use
HTexture tex = gImporter().import<Texture>("BansheLogoRoundSmall.png");
HSpriteTexture spriteTexture = SpriteTexture::create(tex);

// Create the texture GUI element with our sprite texture and default scaling/transparency
GUITexture* guiTexture = GUITexture::create(spriteTexture);

// Position the texture
guiTexture->setPosition(250, 90);
guiTexture->setSize(150, 150);

mainPanel->addElement(guiTexture);
~~~~~~~~~~~~~ 
 
![Texture](guiTexture.png) 
 
## Button
A button GUI element displays a textural string or an image and reports events about user interaction with the button.

GUI elements that can have either text or image contents (or both) accept a @ref bs::GUIContent "GUIContent" structure in their *create* and *setContent* functions. It is just a container and constructed simply:

~~~~~~~~~~~~~{.cpp}
// Contents containing only text
GUIContent textContents(HString(L"Click me!"));

// Contents containing only an image
HTexture tex = gImporter().import<Texture>("BansheLogoRoundSmall.png");
HSpriteTexture spriteTexture = SpriteTexture::create(tex);
		
GUIContent imageContents(spriteTexture);
~~~~~~~~~~~~~

To create a button, call @ref bs::GUIButton::create "GUIButton::create".
~~~~~~~~~~~~~{.cpp}
GUIButton* textButton = GUIButton::create(textContents);
GUIButton* imageButton = GUIButton::create(imageContents);

mainPanel->addElement(textButton);
mainPanel->addElement(imageButton);
~~~~~~~~~~~~~

Once created, user can interact with the button by mousing over it or clicking on it. **GUIButton** provides a set of callbacks that notify the developer when user interacts with the button:
 - @ref bs::GUIButton::onClick "GUIButton::onClick" - Triggers when the user clicks the button.
 - @ref bs::GUIButton::onHover "GUIButton::onHover" - Triggers when the mouse cursor hovers over a button.
 - @ref bs::GUIButton::onOut "GUIButton::onOut" - Triggers when the mouse cursor leaves the button area.
 - @ref bs::GUIButton::onDoubleClick "GUIButton::onDoubleClick" - Triggers when the user clicks the button twice in a quick succession.
 
~~~~~~~~~~~~~{.cpp}
auto buttonClicked = []()
{
	gDebug().logDebug("Button clicked!");
}

// Print a message "Button clicked!" whenever user clicks the button
imageButton->onClick.connect(buttonClicked);
~~~~~~~~~~~~~

![GUI buttons](guiButton.png) 

## Toggle
Toggle buttons are very similar to normal buttons, with the main difference being that they remain in a toggled state after they have been pressed. Multiple toggle buttons can also be grouped so that only one of them can be toggled at a time. Other than that they share the same interface as **GUIButton**, so we'll focus only on the additional functionality.

To create an individual toggle button call @ref bs::GUIToggle::create "GUIToggle::create".
~~~~~~~~~~~~~{.cpp}
GUIToggle* toggle = GUIButton::create(HString());

mainPanel->addElement(toggle);
~~~~~~~~~~~~~

To create a set of toggle buttons call **GUIToggle::create** overload with the @ref bs::GUIToggleGroup "GUIToggleGroup" parameter. All toggles sharing the same toggle group will allow only one of the buttons to be active at a time. This allows you to create a "pick one out of many" element.

To create a toggle group call @ref bs::GUIToggle::createToggleGroup "GUIToggle::createToggleGroup". After that just create the toggle elements as normal, and provide the group as a parameter.

~~~~~~~~~~~~~{.cpp}
SPtr<GUIToggleGroup> group = GUIToggle::createToggleGroup();

GUIToggle* radio0 = GUIButton::create(HString(), group);
GUIToggle* radio1 = GUIButton::create(HString(), group);
GUIToggle* radio2 = GUIButton::create(HString(), group);

mainPanel->addElement(radio0);
mainPanel->addElement(radio1);
mainPanel->addElement(radio2);
~~~~~~~~~~~~~

> Note that we aren't giving any textual labels to the toggle buttons. This is because their default style is a small box, in which we cannot fit any text. If you wish to add labels you need to either use a different style (discussed later), or use a separate **GUILabel** element next to the **GUIToggle** element.

Once created you can subscribe to the @ref bs::GUIToggle::onToggled "GUIToggle::onToggled" event, as well as all previously mentioned **GUIButton** events. **GUIToggle::onToggled** triggers whenever the toggle state of the element changes.

~~~~~~~~~~~~~{.cpp}
auto elementToggled = [](bool toggled)
{
	if(toggled)
		gDebug().logDebug("Toggled!");
	else
		gDebug().logDebug("Untoggled!");
}

toggle->onClick.connect(elementToggled);
~~~~~~~~~~~~~

![GUI toggle](guiToggle.png) 

## Input box
// TODO

## List box
// TODO

## Slider
// TODO

## Scroll area
// TODO