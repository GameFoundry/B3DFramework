//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIListBoxField.h"
#include "BsGUILayoutX.h"
#include "BsGUILabel.h"
#include "BsGUIListBox.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIListBoxField::DEFAULT_LABEL_WIDTH = 100;

	GUIListBoxField::GUIListBoxField(const PrivatelyConstruct& dummy, const Vector<HString>& elements, bool multiselect,
		const GUIContent& labelContent, UINT32 labelWidth, const String& style, const GUIDimensions& dimensions, bool withLabel)
		:GUIElementContainer(dimensions, style), mListBox(nullptr), mLayout(nullptr), mLabel(nullptr)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if (withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(getLabelStyleType()));
			mLayout->addElement(mLabel);
		}

		mListBox = GUIListBox::create(elements, multiselect, getSubStyleName(getListBoxStyleType()));
		mLayout->addElement(mListBox);

		mListBox->onSelectionToggled.connect(std::bind(&GUIListBoxField::selectionChanged, this, _1, _2));
	}

	GUIListBoxField::~GUIListBoxField()
	{

	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent, 
		UINT32 labelWidth, const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, labelContent, labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent, 
		const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const HString& labelText, 
		UINT32 labelWidth, const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const HString& labelText, 
		const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, GUIContent(labelText), 
			DEFAULT_LABEL_WIDTH, *curStyle, GUIDimensions::create(options), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const GUIOptions& options, 
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, GUIContent(), 0, *curStyle,
			GUIDimensions::create(options), false);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, labelContent, labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const HString& labelText, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const HString& labelText,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIListBoxField* GUIListBoxField::create(const Vector<HString>& elements, bool multiselect, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &GUIListBoxField::getGUITypeName();

		return bs_new<GUIListBoxField>(PrivatelyConstruct(), elements, multiselect, GUIContent(), 0, *curStyle,
			GUIDimensions::create(), false);
	}

	bool GUIListBoxField::isMultiselect() const
	{
		return mListBox->isMultiselect();
	}

	void GUIListBoxField::setElements(const Vector<HString>& elements)
	{
		mListBox->setElements(elements);
	}

	void GUIListBoxField::selectElement(UINT32 index)
	{
		mListBox->selectElement(index);
	}

	void GUIListBoxField::deselectElement(UINT32 idx)
	{
		mListBox->deselectElement(idx);
	}

	const Vector<bool>& GUIListBoxField::getElementStates() const
	{
		return mListBox->getElementStates();
	}

	void GUIListBoxField::setElementStates(const Vector<bool>& states)
	{
		mListBox->setElementStates(states);
	}

	void GUIListBoxField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mListBox->setTint(color);
	}

	void GUIListBoxField::_updateLayoutInternal(const GUILayoutData& data)
	{
		mLayout->_setLayoutData(data);
		mLayout->_updateLayoutInternal(data);
	}

	Vector2I GUIListBoxField::_getOptimalSize() const
	{
		return mLayout->_getOptimalSize();
	}

	void GUIListBoxField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mListBox->setStyle(getSubStyleName(getListBoxStyleType()));
	}

	void GUIListBoxField::selectionChanged(UINT32 newIndex, bool enabled)
	{
		onSelectionChanged(newIndex, enabled);
	}

	const String& GUIListBoxField::getGUITypeName()
	{
		static String typeName = "GUIListBoxField";
		return typeName;
	}

	const String& GUIListBoxField::getListBoxStyleType()
	{
		static String LISTBOX_STYLE_TYPE = "EditorFieldListBox";
		return LISTBOX_STYLE_TYPE;
	}

	const String& GUIListBoxField::getLabelStyleType()
	{
		static String LABEL_STYLE_TYPE = "EditorFieldLabel";
		return LABEL_STYLE_TYPE;
	}
}