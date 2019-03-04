GUI layouts									{#guiLayouts}
===============
[TOC]

In previous chapter we talked about how **GUIPanel** is a special type of a GUI element called a "layout". Layouts serve as containers for GUI elements (including other layouts), and they may also control how are elements within them positioned and sized.

There are three types of layouts:
 - @ref bs::GUIPanel "GUIPanel" - Does no automatic positioning and sizing of child GUI elements, instead user can set positions and sizes manually, as we have already seen. Each panel can be placed at a different depth, allowing GUI elements to overlay each other.
 - @ref bs::GUILayoutX "GUILayoutX" - Automatically positions and sizes child GUI elements horizontally one next to each other, left to right. User is able to request minimum/maximum allowed size, but is unable to manually position elements in the layout.
 - @ref bs::GUILayoutY "GUILayoutY" - Same as **GUILayoutX** only elements are positioned vertically, top to bottom.

You will find that vertical and horizontal layouts come in handy when you need to design GUI that needs to scale across various screen sizes/resolutions. By adding your GUI elements to such layouts instead of manually positioning them, ensures the GUI system can always keep them at optimal position and size, regardless of the available screen area.

# Adding/removing elements {#guiLayouts_a}
In the previous chapter we have already seen how to add a GUI element to a layout (a panel, more specifically). Here is the entire interface for dealing with element addition/removal, shared by all layout types:
 - @ref bs::GUILayout::addNewElement<Type, Args...> "GUILayout::addNewElement<T>()" - Creates a new element of type *T* and adds it to the end of the layout's element list.
 - @ref bs::GUILayout::insertNewElement<Type, Args...> "GUILayout::insertNewElement<T>()" - Creates a new element of type *T* and inserts it at a specific position in the layout's element list.
 - @ref bs::GUILayout::removeElement "GUILayout::removeElement()" - Removes a GUI element from the layout's element list.

Here's an example of retrieving the GUI widget's primary panel and adding some elements to it: 
 
~~~~~~~~~~~~~{.cpp}
// Assuming "gui" is a GUI widget component
GUIPanel* mainPanel = gui->getPanel();

// Add a horizontal layout that will automatically position child elements
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();

// Add a text label, and a button to the right of it
layout->addNewElement<GUILabel>(HString("This is a button: "));
layout->addNewElement<GUIButton>(HString("Click me"));
~~~~~~~~~~~~~ 

GUI elements can also be created separately and then added to layouts later using these methods:
 - @ref bs::GUILayout::addElement "GUILayout::addElement()" - Adds an existing GUI element at the end of the layout's element list.
 - @ref bs::GUILayout::insertElement "GUILayout::insertElement()" - Inserts an existing GUI elements at a specific position in the layout's element list.

Same result as above, only written differently:
 
~~~~~~~~~~~~~{.cpp}
// Assuming "gui" is a GUI widget component
GUIPanel* mainPanel = gui->getPanel();

// Add a horizontal layout that will automatically position child elements
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();

// Add a text label, and a button to the right of it
GUILabel* label = GUILabel::create(HString("This is a button: "));
GUIButton* button = GUIButton::create(HString("Click me"));

layout->addElement(label);
layout->addElement(button);
~~~~~~~~~~~~~  

The matter which style you use is personal preference.

# Horizontal layout {#guiLayouts_b}
Horizontal layouts are represented with the **GUILayoutX** class. Any objects added to such a layout will be positioned automatically, left to right next to other elements in the layout. When a GUI element is in such a layout, you will not be able to manually set the element's position.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally
GUIPanel* mainPanel = gui->getPanel();

// Add a horizontal layout that will automatically position child elements
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();

for(int i = 0; i < 5; i++)
{
	GUIButton* button = layout->addNewElement<GUIButton>(HString("Click me #" + toString(i)));
	button->setWidth(90);
}
~~~~~~~~~~~~~  

![Horizontal layout](layoutHorizontal.png) 

# Vertical layout {#guiLayouts_c}
Vertical layouts are represented with the **GUILayoutY** class. Similarly to horizontal layouts, vertical layouts also perform automatic positioning but top to bottom instead of left to right.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out vertically
GUIPanel* mainPanel = gui->getPanel();

// Add a vertical layout that will automatically position child elements
GUILayoutY* layout = mainPanel->addNewElement<GUILayoutY>();

for(int i = 0; i < 5; i++)
{
	GUIButton* button = layout->addNewElement<GUIButton>(HString("Click me #" + toString(i)));
	button->setWidth(90);
}
~~~~~~~~~~~~~  

![Vertical layout](layoutVertical.png) 

# Customizing automatic layouts {#guiLayouts_d}
Even though vertical & horizontal layouts are automatic, bs::f provides a variety of mechanisms that allow you to customize the position and size of GUI elements in such layouts.

## Flexible size {#guiLayouts_d_a}
Each GUI element can have a flexible size that determines its minimum & maximum allowed width/height. This is in contrast to the fixed size we were setting in the previous examples.

When a flexible size is set a GUI layout is allowed to resize the element to best fit the layout area, within the provided size range. Flexible size can be set by calling @ref bs::GUIElement::setFlexibleWidth "GUIElement::setFlexibleWidth()" and @ref bs::GUIElement::setFlexibleHeight "GUIElement::setFlexibleHeight()".

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally, with flexible sizes
GUIPanel* mainPanel = gui->getPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();

for(int i = 0; i < 5; i++)
{
	GUIButton* button = layout->addNewElement<GUIButton>(HString("Click me #" + toString(i)));
	
	// Button will be anywhere between 50 and 200 pixels, depending on how much space is there in the layout
	button->setFlexibleWidth(50, 200);
}
~~~~~~~~~~~~~

The layout will attempt to choose optimal size to fill out the available space. If elements don't fit, they will be clipped instead. If you set the maximum size value to zero, this will imply that the element is free to stretch over the entire size of the layout.

By default different GUI element types might have either a fixed or a flexible size. This is controlled by their *style*, which we'll talk about later.

![Layout with flexibly sized elements](layoutFlexible.png) 

## Spaces {#guiLayouts_d_b}
When a GUI element is a part of a vertical or a horizontal layout, you can no longer control its position manually. This means that calling **GUIElement::setPosition()** will have no effect.

You can however control positioning by some extent by inserting spaces between layout elements. There are two types of spaces: fixed and flexible.

@ref bs::GUIFixedSpace "GUIFixedSpace" - Inserts a space of X pixels at the position in the layout it is added to. As a parameter it takes the number of pixels it will take up. Essentially it acts like an invisible GUI element of a certain width or height.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally, with 10 pixels spacing between them
GUIPanel* mainPanel = gui->getPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();

for(int i = 0; i < 5; i++)
{
	GUIButton* button = layout->addNewElement<GUIButton>(HString("Click me #" + toString(i)));
	button->setWidth(90);
	
	layout->addNewElement<GUIFixedSpace>(10);
}
~~~~~~~~~~~~~

![Layout with fixed spacing](layoutFixedSpace.png) 

@ref bs::GUIFlexibleSpace "GUIFlexibleSpace" inserts a space that will resize itself to fill all available area in the layout. If normal GUI elements already fill the layout area then the flexible space will be of size 0. If there are multiple flexible spaces in a layout, the available size will be spread out equally between them.

Flexible spaces can be used for centering and justifying GUI elements.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally, justified so they neatly align with left/right borders of the layout
GUIPanel* mainPanel = gui->getPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();

for(int i = 0; i < 5; i++)
{
	GUIButton* button = layout->addNewElement<GUIButton>(HString("Click me #" + toString(i)));
	button->setWidth(90);
	
	if(i != 4)
		layout->addNewElement<GUIFlexibleSpace>();
}
~~~~~~~~~~~~~

![Layout with flexible spacing](layoutFlexibleSpace.png) 

## Position and size {#guiLayouts_d_c}
Layouts can be positioned and resized like any other GUI element. All elements inside a layout will be relative to the layout's position, and elements that don't fit within the size of the layout will be clipped. Size of the layout also controls how are flexible sizes determined (either element's or space's).

~~~~~~~~~~~~~{.cpp}
// Add a horizontal GUI layout, position and size it
GUIPanel* mainPanel = gui->getPanel();

// Offset the layout to (50, 50) position, and set its size to 150 pixels wide
// and 25 pixels high
GUILayoutX* layout = mainPanel->addNewElement<GUILayoutX>();
layout->setPosition(50, 50);
layout->setSize(150, 25);

// Add five buttons to the layout
for(int i = 0; i < 5; i++)
{
	GUIButton* button = layout->addNewElement<GUIButton>(HString("Click me #" + toString(i)));
	button->setWidth(90);
	
	if(i != 4)
		layout->addNewElement<GUIFlexibleSpace>();
}
~~~~~~~~~~~~~

![Layout with a custom position and size](layoutPosition.png) 

# Compound layouts {#guiLayouts_e}
Layouts themselves are treated as GUI elements. This means you can add layouts as children to other layouts, allowing you to create complex looking graphical interfaces. For example lets create a table of four rows and five columns of buttons:

~~~~~~~~~~~~~{.cpp}
GUIPanel* mainPanel = gui->getPanel();

GUILayoutX* columns = mainPanel->addNewElement<GUILayoutX>();
for (int i = 0; i < 5; i++)
{
	GUILayoutY* rows = columns->addNewElement<GUILayoutY>();

	for(int j = 0; j < 4; j++)
	{
		GUIButton* button = rows->addNewElement<GUIButton>(HString("Click me"));
		button->setWidth(70);
	}

	if(i != 4)
		columns->addNewElement<GUIFlexibleSpace>();
}
~~~~~~~~~~~~~

![Compound layout](layoutTable.png) 

# Panels {#guiLayouts_f}
**GUIPanel** is a special type of layout that doesn't perform any automatic positioning. Instead, as we've already seen, it allows you to position and size the elements directly, when you need exact precision over their placement. Elements like spaces or flexible sizes have no effect on GUI elements part of a GUI panel.

Aside from allowing you to control exact placement of elements, they can also be used for overlaying GUI elements on top of each other. When created each GUI panel accepts a depth parameter which controls where is it positioned relative to other panels. Higher depth means the panel will be rendered below those with lower depth.

Lets show an example of using a GUI panel to manually position and size a GUI element, as well as using the depth parameter to show two overlapping GUI elements.

~~~~~~~~~~~~~{.cpp}
// Load texture to use for the underlay
HTexture tex = gImporter().import<Texture>("BansheLogoRoundSmall.png");
HSpriteTexture spriteTexture = SpriteTexture::create(tex);

// Render this panel below the main panel
INT16 depth = 1;
GUIPanel* background = mainPanel->addNewElement<GUIPanel>(depth);

// Add a background image to the background panel
GUITexture* guiTexture = background->addNewElement<GUITexture>(spriteTexture);
guiTexture->setPosition(250, 90);
guiTexture->setSize(150, 150);
guiTexture->setTint(Color(1.0f, 1.0f, 1.0f, 0.5f));

// Add a button to the main panel, which renders a depth 0 (default), in front of the background panel
GUIButton* button = mainPanel->addNewElement<GUIButton>(HString("Click me"));
button->setPosition(280, 190);
button->setSize(90, 25);
~~~~~~~~~~~~~

![GUI panels with overlapping elements](panelOverlay.png) 