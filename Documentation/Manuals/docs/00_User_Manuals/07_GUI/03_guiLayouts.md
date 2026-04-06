---
title: GUI layouts
---

In the previous chapter we learned that **GUIPanel** is a special type of GUI element called a "layout". Layouts serve as containers for GUI elements (including other layouts), and they may also control how elements within them are positioned and sized.

There are three types of layouts:
 - @b3d::GUIPanel - Does no automatic positioning and sizing of child GUI elements. Instead, positions and sizes are set manually. Each panel can be placed at a different depth, allowing GUI elements to overlay each other.
 - @b3d::GUILayoutX - Automatically positions and sizes child GUI elements horizontally, one next to each other, left to right. You can request minimum/maximum allowed size, but cannot manually position elements in the layout.
 - @b3d::GUILayoutY - Same as **GUILayoutX**, but positions elements vertically, top to bottom.

Vertical and horizontal layouts are particularly valuable when designing GUI that needs to scale across various screen sizes and resolutions. By adding GUI elements to such layouts instead of manually positioning them, the GUI system can keep them at optimal position and size regardless of the available screen area.

# Adding and removing elements

All layout types share a common interface for element addition and removal:
 - @b3d::GUILayout::AddNewElement<T> - Creates a new element of type *T* and adds it to the end of the layout's element list.
 - @b3d::GUILayout::InsertNewElement<T> - Creates a new element of type *T* and inserts it at a specific position in the layout's element list.
 - @b3d::GUILayout::RemoveElement - Removes a GUI element from the layout's element list.

Here's an example of retrieving the GUI widget's primary panel and adding elements to it:

~~~~~~~~~~~~~{.cpp}
// Assuming guiWidget is a GUIWidget component
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a horizontal layout that will automatically position child elements
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

// Add a text label and a button to the right of it
layout->AddNewElement<GUILabel>(HString("This is a button: "));
layout->AddNewElement<GUIButton>(HString("Click me"));
~~~~~~~~~~~~~

GUI elements can also be created separately and then added to layouts later using these methods:
 - @b3d::GUILayout::AddElement - Adds an existing GUI element at the end of the layout's element list.
 - @b3d::GUILayout::InsertElement - Inserts an existing GUI element at a specific position in the layout's element list.

Same result as above, written differently:

~~~~~~~~~~~~~{.cpp}
// Assuming guiWidget is a GUIWidget component
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

// Create elements separately
GUILabel* label = GUILabel::Create(HString("This is a button: "));
GUIButton* button = GUIButton::Create(HString("Click me"));

// Add to layout
layout->AddElement(label);
layout->AddElement(button);
~~~~~~~~~~~~~

The choice of style is a matter of personal preference.

# Horizontal layout

Horizontal layouts are represented by the **GUILayoutX** class. Objects added to such a layout are positioned automatically, left to right next to other elements. When a GUI element is in such a layout, you cannot manually set the element's position.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

for (int i = 0; i < 5; i++)
{
	GUIButton* button = layout->AddNewElement<GUIButton>(HString("Click me #" + ToString(i)));
	button->SetWidth(90);
}
~~~~~~~~~~~~~

![Horizontal layout](../../Images/layoutHorizontal.png)

# Vertical layout

Vertical layouts are represented by the **GUILayoutY** class. Similarly to horizontal layouts, vertical layouts perform automatic positioning but top to bottom instead of left to right.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out vertically
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a vertical layout
GUILayoutY* layout = mainPanel->AddNewElement<GUILayoutY>();

for (int i = 0; i < 5; i++)
{
	GUIButton* button = layout->AddNewElement<GUIButton>(HString("Click me #" + ToString(i)));
	button->SetWidth(90);
}
~~~~~~~~~~~~~

![Vertical layout](../../Images/layoutVertical.png)

# Customizing automatic layouts

Even though vertical and horizontal layouts are automatic, the framework provides various mechanisms to customize the position and size of GUI elements in such layouts.

## Flexible size

Each GUI element can have a flexible size that determines its minimum and maximum allowed width/height. This is in contrast to the fixed size we set in previous examples.

When a flexible size is set, a GUI layout is allowed to resize the element to best fit the layout area, within the provided size range. Set flexible size by calling @b3d::GUIElement::SetFlexibleWidth and @b3d::GUIElement::SetFlexibleHeight.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally with flexible sizes
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

for (int i = 0; i < 5; i++)
{
	GUIButton* button = layout->AddNewElement<GUIButton>(HString("Click me #" + ToString(i)));

	// Button will be between 50 and 200 pixels, depending on available space
	button->SetFlexibleWidth(50, 200);
}
~~~~~~~~~~~~~

The layout will attempt to choose optimal size to fill the available space. If elements don't fit, they will be clipped. If you set the maximum size value to zero, this implies the element is free to stretch over the entire size of the layout.

By default, different GUI element types might have either a fixed or a flexible size. This is controlled by their style.

![Layout with flexibly sized elements](../../Images/layoutFlexible.png)

## Expanding size

In addition to flexible sizing, elements can be set to **expanding** mode. An expanding element will attempt to fill all available space in the layout, rather than using its optimal size. This is useful when you want an element to take up as much room as possible.

Set expanding size by using @b3d::GUIOption::ExpandingWidth and @b3d::GUIOption::ExpandingHeight when creating the element:

~~~~~~~~~~~~~{.cpp}
GUIPanel* mainPanel = guiWidget->GetPanel();
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

// This button will expand to fill all available horizontal space
GUIButton* expandingButton = layout->AddNewElement<GUIButton>(
	GUIContent(HString("I expand!")),
	GUIOptions(GUIOption::ExpandingWidth())
);

// This button stays at its optimal size
GUIButton* normalButton = layout->AddNewElement<GUIButton>(
	GUIContent(HString("Normal"))
);
~~~~~~~~~~~~~

You can also provide optional minimum and maximum constraints:

~~~~~~~~~~~~~{.cpp}
// Expand to fill available space, but not less than 100 or more than 500 pixels
GUIButton* constrainedButton = layout->AddNewElement<GUIButton>(
	GUIContent(HString("Constrained expand")),
	GUIOptions(GUIOption::ExpandingWidth(100, 500))
);
~~~~~~~~~~~~~

The difference between flexible and expanding is that a flexible element will use its optimal size when there is enough space, while an expanding element will always try to grow to the maximum available space.

## Spaces

When a GUI element is part of a vertical or horizontal layout, you can no longer control its position manually. Calling **GUIElement::SetPosition()** will have no effect.

You can control positioning to some extent by inserting spaces between layout elements. There are two types of spaces: fixed and flexible.

@b3d::GUIFixedSpace inserts a space of a specific number of pixels at the position in the layout it is added to. It acts like an invisible GUI element of a certain width or height.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally with 10 pixel spacing
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

for (int i = 0; i < 5; i++)
{
	GUIButton* button = layout->AddNewElement<GUIButton>(HString("Click me #" + ToString(i)));
	button->SetWidth(90);

	layout->AddNewElement<GUIFixedSpace>(10);
}
~~~~~~~~~~~~~

![Layout with fixed spacing](../../Images/layoutFixedSpace.png)

@b3d::GUIFlexibleSpace inserts a space that resizes itself to fill all available area in the layout. If normal GUI elements already fill the layout area, the flexible space will be of size 0. If there are multiple flexible spaces in a layout, the available size is spread equally between them.

Flexible spaces can be used for centering and justifying GUI elements.

~~~~~~~~~~~~~{.cpp}
// Add five GUI buttons laid out horizontally, justified to align with left/right borders
GUIPanel* mainPanel = guiWidget->GetPanel();

// Add a horizontal layout
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();

for (int i = 0; i < 5; i++)
{
	GUIButton* button = layout->AddNewElement<GUIButton>(HString("Click me #" + ToString(i)));
	button->SetWidth(90);

	if (i != 4)
		layout->AddNewElement<GUIFlexibleSpace>();
}
~~~~~~~~~~~~~

![Layout with flexible spacing](../../Images/layoutFlexibleSpace.png)

## Position and size

Layouts can be positioned and resized like any other GUI element. All elements inside a layout will be relative to the layout's position, and elements that don't fit within the size of the layout will be clipped. The size of the layout also controls how flexible sizes are determined (either element or space sizes).

~~~~~~~~~~~~~{.cpp}
// Add a horizontal GUI layout, position and size it
GUIPanel* mainPanel = guiWidget->GetPanel();

// Offset the layout to (50, 50) position, and set its size to 150 pixels wide
// and 25 pixels high
GUILayoutX* layout = mainPanel->AddNewElement<GUILayoutX>();
layout->SetPosition(50, 50);
layout->SetSize(GUILogicalSize(150, 25));

// Add five buttons to the layout
for (int i = 0; i < 5; i++)
{
	GUIButton* button = layout->AddNewElement<GUIButton>(HString("Click me #" + ToString(i)));
	button->SetWidth(90);

	if (i != 4)
		layout->AddNewElement<GUIFlexibleSpace>();
}
~~~~~~~~~~~~~

![Layout with a custom position and size](../../Images/layoutPosition.png)

# Compound layouts

Layouts themselves are treated as GUI elements. This means you can add layouts as children to other layouts, allowing you to create complex graphical interfaces. For example, let's create a table of four rows and five columns of buttons:

~~~~~~~~~~~~~{.cpp}
GUIPanel* mainPanel = guiWidget->GetPanel();

GUILayoutX* columns = mainPanel->AddNewElement<GUILayoutX>();
for (int i = 0; i < 5; i++)
{
	GUILayoutY* rows = columns->AddNewElement<GUILayoutY>();

	for (int j = 0; j < 4; j++)
	{
		GUIButton* button = rows->AddNewElement<GUIButton>(HString("Click me"));
		button->SetWidth(70);
	}

	if (i != 4)
		columns->AddNewElement<GUIFlexibleSpace>();
}
~~~~~~~~~~~~~

![Compound layout](../../Images/layoutTable.png)

# Panels

**GUIPanel** is a special type of layout that doesn't perform any automatic positioning. Instead, it allows you to position and size elements directly when you need exact precision over their placement. Elements like spaces or flexible sizes have no effect on GUI elements that are part of a GUI panel.

Aside from allowing exact placement control, panels can also be used for overlaying GUI elements on top of each other. When created, each GUI panel accepts a depth parameter which controls where it is positioned relative to other panels. Higher depth means the panel will be rendered below those with lower depth.

Let's show an example of using a GUI panel to manually position and size a GUI element, as well as using the depth parameter to show two overlapping GUI elements.

~~~~~~~~~~~~~{.cpp}
// Load texture for the underlay
HTexture texture = GetImporter().Import<Texture>("logo.png");
HSpriteTexture spriteTexture = SpriteTexture::Create(texture);

// Render this panel below the main panel
i16 depth = 1;
GUIPanel* background = mainPanel->AddNewElement<GUIPanel>(depth);

// Add a background image to the background panel
GUITexture* guiTexture = background->AddNewElement<GUITexture>(spriteTexture);
guiTexture->SetPosition(GUILogicalPoint(250, 90));
guiTexture->SetSize(GUILogicalSize(150, 150));

// Add a button to the main panel, which renders at depth 0 (default), in front of the background panel
GUIButton* button = mainPanel->AddNewElement<GUIButton>(HString("Click me"));
button->SetPosition(GUILogicalPoint(280, 190));
button->SetSize(GUILogicalSize(90, 25));
~~~~~~~~~~~~~

![GUI panels with overlapping elements](../../Images/panelOverlay.png)
