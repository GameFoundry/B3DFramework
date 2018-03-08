Extending the GUI system						{#customGUI}
===============
[TOC]

Even though bs::f provides fully skinnable and very customizable GUI elements, sometimes the built-in ones are just not enough if you need some very specialized functionality or look. bs::f allows you to create brand new elements and fully customize the way how they are rendered and how the user interacts with such elements.

You are expected to have read the user-facing GUI manuals before proceeding, and as such familiarized yourself with the basics.

All GUI elements derive from the base @ref bs::GUIElementBase "GUIElementBase" type. The elements can be categorized into two major groups:
 - *Layouts* - They derive from @ref bs::GUILayout "GUILayout" and do not have any graphics, but instead they control the placement of all elements attached to them.
 - *Elements* - They derive from @ref bs::GUIElement "GUIElement", and they're standard GUI elements like buttons, input boxes, sliders and such.
 
It's unlikely you will have the need to create **GUILayout** types, so we will instead focus on creating custom GUI elements. 
 
# GUIElement implementation {#customGUI_a}
When creating a custom GUI element you will need to override the **GUIElement** class. In this section we'll go over the *minimum* set of functionality every GUI element must implement, split into relevant sections. In later sections we'll focus on optional functionality.

~~~~~~~~~~~~~{.cpp}
// GUI element that displays a single texture
class GUITexture : public GUIElement
{
	// ... implementation
};
~~~~~~~~~~~~~ 

## Construction {#customGUI_a_a}
In its constructor **GUIElement** expects two parameters:
 - `styleName` - Name of the style to use for rendering the element. This will be looked up in the currently active **GUISkin** and relevant @ref bs::GUIElementStyle "GUIElementStyle" objects will be used for determining object's textures, fonts, colors, borders, margins and similar. Each GUI element type should have a unique default style name, but you also might want to allow the user to override the default style name if he desires. 
 - `dimensions` - Initial set of dimensions determining GUI element's position and size, represented by @ref bs::GUIDimensions "GUIDimensions". You can create an empty set of dimensions by calling @ref bs::GUIDimensions::create "GUIDimensions::create()", in which case the system will use whichever dimensions are provided by the **GUIElementStyle**. Other overload of **GUIDimensions::create()** accepts a @ref bs::GUIOptions "GUIOptions" object, in which you can specify an arbitrary sized array of various dimension related options.

As a rule of thumb in order to aid construction GUI elements should provide a set of static `create()` methods (but aren't required to). You usually want to provide a few overloads of `create()` that allow the user to use custom style/dimensions or stick with the default one.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
public:
	static const String& getGUITypeName()
	{
		// Returns the name of this GUI element, used for looking up the default style
		static String name = "Texture";
		return name;
	}
	
	GUITexture(const String& styleName, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions)
	{ }

	// create() overload accepting custom dimensions in the form of GUIOptions
	static GUITexture* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK)
	{
		// If user didn't provide a style name, use the default
		String finalStyleName = styleName;
		if(styleName == StringUtil::BLANK)
			finalStyleName = getGUITypeName();

		// Pass style name, and construct dimensions from the provided list of options
		return bs_new<GUITexture>(finalStyleName, GUIDimensions::create(options));
	}
	
	// create() overload using default dimensions
	static GUITexture* create(const String& styleName = StringUtil::BLANK)
	{
		// Same as above
		String finalStyleName = styleName;
		if(styleName == StringUtil::BLANK)
			finalStyleName = getGUITypeName();

		// Pass style name, and construct default dimensions object
		return bs_new<GUITexture>(finalStyleName, GUIDimensions::create());
	}

	// ... remaining GUITexture implementation
};
~~~~~~~~~~~~~ 

## Graphics {#customGUI_a_b}
In order for your GUI element to be visible you need to define its graphics. This involves:
 - Creating a set of vertices and indices that represent your GUI element
 - Create the material to render the GUI element with, and bind required textures and other properties to it

This is done by implementing the following methods:

@ref bs::GUIElement::_getNumRenderElements() "GUIElement::_getNumRenderElements()"

Returns the number of separate elements that your GUI element consists of. Each element corresponds to a mesh and a material to render the mesh with. For most GUI elements there will be only one element, but you will need multiple elements in case your GUI element uses multiple textures or materials.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	UINT32 _getNumRenderElements() const override
	{
		// Our simple GUI element has just a single render element (a single quad to be exact)
		return 1;
	}
};
~~~~~~~~~~~~~

@ref bs::GUIElement::_getMeshInfo() "GUIElement::_getMeshInfo()"

Takes a render element index as input and returns the number of vertices and indices for the mesh at the specified index. This allows external systems to know how big of a buffer to allocate for the element's mesh. It also returns the type of the mesh which lets external systems know how to render the provided vertices.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const override
	{
		// Since we're rendering a single quad, we use 4 vertices, rendered as 2 triangles (6 indices)
		numVertices = 4;
		numIndices = 6;
		type = GUIMeshType::Triangle;
	}
};
~~~~~~~~~~~~~

@ref bs::GUIElement::_fillBuffer() "GUIElement::_fillBuffer()"

This is the primary method when it comes to **GUIElement** graphics. It allows you to provide the actual vertex and index data that will be used for rendering the GUI element. Note that number of vertices and indices provided must match the number returned by **GUIElement::_getMeshInfo()**.

The method receives pointers to the index and vertex memory that must be populated by the function, offsets at which memory location should the function output its data at, and external buffer sizes (used for overflow checking if needed).

The vertices are always in a specific format:
 - Each vertex contains a 2D position, followed by 2D texture coordinates
 - Vertex size is `sizeof(Vector2) * 2`

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		// Get location of this element in the provided buffers
		vertices += vertexStride * vertexOffset;
		UINT8* uvs = vertices + sizeof(Vector2);
		
		indices += indexStride * indexOffset;
		
		// Populate the vertices, uv and indices buffer
		// ... generate vertices, uvs and indices as you would for any mesh
	}
};
~~~~~~~~~~~~~

bs::f also provides a set of helper classes for generating required geometry in the form of @ref bs::ImageSprite "ImageSprite" and @ref bs::TextSprite "TextSprite" classes. **ImageSprite** can easily generate image geometry of specified size, whether a simple quad or a scale-9-grid image (scalable image with fixed borders). And **TextSprite** will take a text string, font and additional options as input, and output a set of quads required for text rendering.

@ref bs::GUIElement::_getMaterial() "GUIElement::_getMaterial()"

Takes a render element index as input and returns a material to render the element with, as well material parameters. 

GUI materials are represented by the @ref bs::SpriteMaterial "SpriteMaterial" type and you can use @ref bs::SpriteManager "SpriteManager" to retrieve the built-in materials. You can also create your own sprite materials by implementing the **SpriteMaterial** class and registering them with the **SpriteManager** by calling @ref bs::SpriteManager::registerMaterial "SpriteManager::registerMaterial()". 

Material parameters are represented by the @ref bs::SpriteMaterialInfo "SpriteMaterialInfo" structure, which contains the texture and color to render the element with. It also contains a `groupId` identifier which tells the external systems with which other GUI elements is this element allowed to be grouped with. Grouped elements are rendered together (batched) for better performance, but sometimes this is not wanted. Generally you want to provide the memory address of the parent **GUIWidget** for the `groupId` field, and don't need to worry about it further. You can retrieve the element's **GUIWidget** by calling @ref bs::GUIElement::_getParentWidget() "GUIElement::_getParentWidget()".

Textures used by the material parameter can be retrieved from the current **GUIElementStyle**, or provided directly to the GUI element by user-specified means. It is preferred to use the **GUIElementStyle** approach as this allows the user to customize the element look through **GUISkin**%s, or by providing a custom style name. To retrieve the currently active style call @ref bs::GUIElement::_getStyle() "GUIElement::_getStyle()". 

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		// Get material for rendering opaque images
		*material = SpriteManager::instance().getImageOpaqueMaterial();
		
		// Populate the material parameters (normally you want to populate this when properties change and just return it here)
		//// Use the normal texture as provided in GUIElementStyle
		mMatParams.texture = _getStyle().normal.texture;
		
		//// No tint
		mMatParams.tint = Color::White;
		
		//// Ensures only the GUI elements sharing the same GUIWidget can be grouped
		mMatParams.groupId = (UINT64)_getParentWidget();
		
		return mMatParams;
	}
	
	SpriteMaterialInfo mMatParams;
};
~~~~~~~~~~~~~

@ref bs::GUIElement::updateRenderElementsInternal() "GUIElement::updateRenderElementsInternal()"

Called whenever the element's size or style changes. This is the method where you should rebuild the GUI element's mesh. While you could do it directly in **GUIElement::_fillBuffer()** it is better performance-wise to do it here, and instead just return the cached mesh when **GUIElement::_fillBuffer()** is called. This is because **GUIElement::_fillBuffer()** will get called even when mesh geometry didn't change, if the external systems deem it required.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	void updateRenderElementsInternal()
	{		
		// Move the code for vertex/index generation here, cache the generate data and then just return the cached data when _fillBuffer() is called.
		
		GUIElement::updateRenderElementsInternal();
	}
};
~~~~~~~~~~~~~

## Layout {#customGUI_a_c}
In order for the GUI element to work well with the automatic positioning performed by GUI layouts, you must override the @ref bs::GUIElement::_getOptimalSize "GUIElement::_getOptimalSize()" method. As the name implies the method should return the optimal size of your GUI element. For example if your element was displaying a texture 64x64 in size, then the optimal size should probably return 64x64. If you element is displaying text you can use @ref bs::GUIHelper "GUIHelper" to help you calculate the bounds of the text. If displaying something else then it's up to you to determine what constitutes an optimal size.

~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	Vector2I _getOptimalSize() const
	{
		Vector2I optimalSize;
		
		// Retrieve the size of the main texture, if one is provided
		HSpriteTexture tex = _getStyle().normal.texture;
		if(tex != nullptr)
		{
			optimalSize.x = tex->getWidth();
			optimalSize.y = tex->getHeight();
		}
		
		return optimalSize;
	}
};
~~~~~~~~~~~~~

This concludes the minimal set of functionality required for implementing a **GUIElement**. At this point you will have a static GUI element that allows no user interaction. In next sections we expand on this by adding support for user input and dynamic graphics.

# Redrawing {#customGUI_b}
If your element allows the user to update certain contents of it after creation (i.e. it's not static), you need to notify the GUI system so it knows when to rebuild the element. For example if your GUI element displays a text string you might provide a `setText()` method that allows the user to change what is displayed. When text is changed the GUI system will need to update the batched GUI mesh, update the GUI element's mesh and/or update the layout position/size of the element.

There are three ways to notify the GUI system the element is dirty, each more expensive in performance than the previous:
 - @ref bs::GUIElement::_markMeshAsDirty() "GUIElement::_markMeshAsDirty()" - Causes the batched GUI mesh to be re-assembled. Will cause a call to **GUIElement::_fillBuffer()**. Call this if element's contents and size didn't change (e.g. when its position, or depth changes). Batched GUI mesh is a mesh managed by the GUI system that consists of multiple GUI elements sharing the same material properties (used to improve rendering performance).
 - @ref bs::GUIElement::_markContentAsDirty() "GUIElement::_markContentAsDirty()" - Has the same effect as **GUIElement::_markMeshDirty()**, but will also trigger a **GUIElement::updateRenderElementsInternal()** call so the GUI element mesh will be fully rebuilt. Call this when contents of the GUI element change, but not its size (e.g. changing a texture but it is the same dimensions as the old one.)
 - @ref bs::GUIElement::_markLayoutAsDirty() "GUIElement::_markLayoutAsDirty()" - Has the same effect as **GUIElement::_markContentDirty()**, but will also cause the layout system to recalculate the element's position and size. This can be expensive as this will generally trigger layout updates for all siblings and children, potentially even parent GUI elements. Call this when element changes and no other dirty calls are appropriate. Normally you have to call this whenever the element's size changes.
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	// Changes the look of the GUI element by fading it out when enabled
	void setFadeOut(bool fadeOut)
	{
		if(fadeOut)
			mMatParams.tint = Color(1.0f, 1.0f, 1.0f, 0.5f);
		else
			mMatParams.tint = Color::White;
		
		// Ensure the system redraws the mesh. Since neither the size or contents changed we can call the cheapest redraw method, '_markMeshAsDirty()'
		_markMeshAsDirty();
	}
	
	// Updated version of _getMaterial()
	const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		// Get material for rendering opaque images
		*material = SpriteManager::instance().getImageOpaqueMaterial();
		
		// Assuming remaining mMatParams contents are populated elsewhere
		return mMatParams;
	}
	
	SpriteMaterialInfo mMatParams;
};
~~~~~~~~~~~~~
 
# Input {#customGUI_c}
In order for the GUI element to accept input from the user you can implement any or all of the following methods:
 - @ref bs::GUIElement::_mouseEvent "GUIElement::_mouseEvent()" - Triggered when the mouse moves and/or is clicked. See @ref bs::GUIMouseEventType "GUIMouseEventType" for a list of events. Receives @ref bs::GUIMouseEvent "GUIMouseEvent" structure as input.
 - @ref bs::GUIElement::_textInputEvent "GUIElement::_textInputEvent()" - Triggered when the user inputs some text on the keyboard, while the GUI element is in focus. Receives @ref bs::GUITextInputEvent "GUITextInputEvent" structure as input.
 - @ref bs::GUIElement::_virtualButtonEvent "GUIElement::_virtualButtonEvent()" - Triggered when a certain virtual button is pressed, while the GUI element is in focus. Receives @ref bs::GUIVirtualButtonEvent "GUIVirtualButtonEvent" structure as input.
 - @ref bs::GUIElement::_commandEvent "GUIElement::_commandEvent()" - Triggers when some kind of a specialized event happens, like losing/gaining focus, deleting text, selecting text, forcing redraw and similar. See @ref bs::GUICommandEventType "GUICommandEventType" for an explanation of what each event type does. Receives @ref bs::GUICommandEvent "GUICommandEvent" structure as input.
 
Each of the methods should return false if other GUI elements are allowed to receive the same event. If true is returned the GUI element will consume the event and it wont be visible to others.
 
~~~~~~~~~~~~~{.cpp}
class GUITexture : public GUIElement
{
	// ... remaining GUITexture implementation

	bool _mouseEvent(const GUIMouseEvent& ev)
	{
		// Fade out the texture when user mouses over the element
		if(ev.getType() == GUIMouseEventType::MouseOver)
			setFadeOut(true);
		else if(ev.getType() == GUIMouseEventType::MouseOut)
			setFadeOut(false);

		return false;
	}
};
~~~~~~~~~~~~~