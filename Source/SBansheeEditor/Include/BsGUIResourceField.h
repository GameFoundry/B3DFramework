//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsGUIElementContainer.h"

namespace bs
{
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**
	 * GUI object that displays a field in which a Resource can be dragged and dropped. The field accepts a Resource of a
	 * specific type and displays an optional label.
	 */
	class BS_SCR_BED_EXPORT GUIResourceField : public GUIElementContainer
	{
		struct PrivatelyConstruct {};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const GUIContent& labelContent, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelText		Text to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const HString& labelText, UINT32 labelWidth, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelText		Text to display in the editor field label.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const HString& labelText, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field without a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const GUIContent& labelContent,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelText		Text to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const HString& labelText, UINT32 labelWidth,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field with a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	labelText		Text to display in the editor field label.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const HString& labelText,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new resource GUI editor field without a label.
		 *
		 * @param[in]	typeNamespace	Namespace of the type this field accepts. 
		 * @param[in]	type			Type name of the type this field accepts. Must derive from Resource.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIResourceField* create(const String& typeNamespace, const String& type, const String& style = StringUtil::BLANK);

		GUIResourceField(const PrivatelyConstruct& dummy, const String& typeNamespace, const String& type, const GUIContent& labelContent,
			UINT32 labelWidth, const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the resource referenced by the field, if any. */
		HResource getValue() const;

		/**	Sets the resource referenced by the field. */
		void setValue(const HResource& value);

		/**	Returns a weak reference to the texture referenced by the field, if any. */
		WeakResourceHandle<Resource> getValueWeak() const;

		/**	Sets a weak reference to the texture referenced by the field. */
		void setValueWeak(const WeakResourceHandle<Resource>& value);

		/** Returns the resource referenced by the field. Returns empty string with no resource is referenced. */
		String getUUID() const { return mUUID; }

		/** @copydoc GUIElement::setTint */
		virtual void setTint(const Color& color) override;

		/**
		 * Triggered whenever the referenced resource changes. Provides	a weak handle of the resource, or empty handle if
		 * no resource is referenced.
		 */
		Event<void(const WeakResourceHandle<Resource>&)> onValueChanged;

		/** @name Internal 
		 *  @{
		 */

		/** @copydoc GUIElement::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */
	private:
		virtual ~GUIResourceField();

		/**
		 * Sets the resource referenced by the field by finding the resource with the provided UUID.
		 *			
		 * @param[in]	uuid			Unique resource identifier of the resource to show, or empty string if no resource.
		 * @param[in]	triggerEvent	Determines should the onValueChanged() event be triggered if the new UUID is
		 *								different from the previous one.
		 */
		void setUUID(const String& uuid, bool triggerEvent = true);

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when a drag and drop operation finishes over this element. */
		void dataDropped(void* data);

		/**	Triggered when the drop button that displays the game object label is clicked. */
		void onDropButtonClicked();

		/**	Triggered when the clear button is clicked. */
		void onClearButtonClicked();

	private:
		static const UINT32 DEFAULT_LABEL_WIDTH;

		GUILayout* mLayout;
		GUILabel* mLabel;
		GUIDropButton* mDropButton;
		GUIButton* mClearButton;
		String mNamespace;
		String mType;
		String mUUID;
	};

	/** @} */
}