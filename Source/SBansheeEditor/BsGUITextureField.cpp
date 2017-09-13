//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUITextureField.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIDropButton.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIPanel.h"
#include "Resources/BsResources.h"
#include "Library/BsProjectLibrary.h"
#include "Utility/BsBuiltinEditorResources.h"
#include "GUI/BsGUIResourceTreeView.h"
#include "GUI/BsGUISpace.h"
#include "Library/BsProjectResourceMeta.h"
#include "2D/BsSpriteTexture.h"
#include "Scene/BsSelection.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUITextureField::DEFAULT_LABEL_WIDTH = 100;

	GUITextureField::GUITextureField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		:GUIElementContainer(dimensions, style), mLabel(nullptr), mDropButton(nullptr), mClearButton(nullptr)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if (withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(BuiltinEditorResources::TextureFieldLabelStyleName));
			mLayout->addElement(mLabel);
		}

		mDropButton = GUIDropButton::create((UINT32)DragAndDropType::Resources, getSubStyleName(BuiltinEditorResources::TextureFieldDropStyleName));
		mClearButton = GUIButton::create(HString(L""), getSubStyleName(BuiltinEditorResources::TextureFieldClearBtnStyleName));
		mClearButton->onClick.connect(std::bind(&GUITextureField::onClearButtonClicked, this));

		GUIPanel* dropTargetPanel = mLayout->addNewElement<GUIPanel>();
		dropTargetPanel->addElement(mDropButton);

		GUIPanel* closeBtnPanel = dropTargetPanel->addNewElement<GUIPanel>(-1);
		GUILayoutY* closeBtnLayoutY = closeBtnPanel->addNewElement<GUILayoutY>();
		closeBtnLayoutY->addNewElement<GUIFixedSpace>(5);
		GUILayoutX* closeBtnLayoutX = closeBtnLayoutY->addNewElement<GUILayoutX>();
		closeBtnLayoutY->addNewElement<GUIFlexibleSpace>();
		closeBtnLayoutX->addNewElement<GUIFlexibleSpace>();
		closeBtnLayoutX->addElement(mClearButton);
		closeBtnLayoutX->addNewElement<GUIFixedSpace>(5);

		if (withLabel)
			mLayout->addNewElement<GUIFlexibleSpace>();

		mDropButton->onDataDropped.connect(std::bind(&GUITextureField::dataDropped, this, _1));
		mDropButton->onClick.connect(std::bind(&GUITextureField::onDropButtonClicked, this));
	}

	GUITextureField::~GUITextureField()
	{

	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create(const HString& labelText, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create( const HString& labelText, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUITextureField* GUITextureField::create(const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(), 0, *curStyle,
			GUIDimensions::create(options), false);
	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const GUIContent& labelContent, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const HString& labelText, UINT32 labelWidth, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const HString& labelText, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUITextureField* GUITextureField::create(const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::TextureFieldStyleName;

		return bs_new<GUITextureField>(PrivatelyConstruct(), GUIContent(), 0, *curStyle, GUIDimensions::create(), false);
	}

	HTexture GUITextureField::getValue() const
	{
		ResourceLoadFlags loadFlags = ResourceLoadFlag::Default | ResourceLoadFlag::KeepSourceData;
		return static_resource_cast<Texture>(Resources::instance().loadFromUUID(mUUID, false, loadFlags));
	}

	void GUITextureField::setValue(const HTexture& value)
	{
		if (value)
			setUUID(value.getUUID(), false);
		else
			setUUID(UUID::EMPTY, false);
	}

	WeakResourceHandle<Texture> GUITextureField::getValueWeak() const
	{
		HResource handle = Resources::instance()._getResourceHandle(mUUID);
		return static_resource_cast<Texture>(handle.getWeak());
	}

	void GUITextureField::setValueWeak(const WeakResourceHandle<Texture>& value)
	{
		if (value)
			setUUID(value.getUUID(), false);
		else
			setUUID(UUID::EMPTY, false);
	}

	void GUITextureField::setUUID(const UUID& uuid, bool triggerEvent)
	{
		if (mUUID == uuid)
			return;

		mUUID = uuid;

		HTexture texture;

		Path filePath;
		if (Resources::instance().getFilePathFromUUID(mUUID, filePath))
			texture = gResources().load<Texture>(filePath);
		
		if (texture != nullptr)
		{
			HSpriteTexture sprite = SpriteTexture::create(texture);
			mDropButton->setContent(GUIContent(sprite));
			mClearButton->setVisible(true);
		}
		else
		{
			mDropButton->setContent(GUIContent(HString(L"(None)")));
			mClearButton->setVisible(false);
		}

		if (triggerEvent)
			onValueChanged(texture.getWeak());
	}

	void GUITextureField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mDropButton->setTint(color);
		mClearButton->setTint(color);
	}

	void GUITextureField::_updateLayoutInternal(const GUILayoutData& data)
	{
		mLayout->_setLayoutData(data);
		mLayout->_updateLayoutInternal(data);
	}

	Vector2I GUITextureField::_getOptimalSize() const
	{
		return mLayout->_getOptimalSize();
	}

	void GUITextureField::onDropButtonClicked()
	{
		if (mUUID.empty())
			return;

		Path resPath = gProjectLibrary().uuidToPath(mUUID);
		resPath = resPath.getRelative(gProjectLibrary().getResourcesFolder());

		Selection::instance().ping(resPath);
	}

	void GUITextureField::dataDropped(void* data)
	{
		DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(data);
		UINT32 numResources = (UINT32)draggedResources->resourcePaths.size();

		if (numResources <= 0)
			return;

		for (UINT32 i = 0; i < numResources; i++)
		{
			Path path = draggedResources->resourcePaths[i];

			SPtr<ProjectResourceMeta> meta = gProjectLibrary().findResourceMeta(draggedResources->resourcePaths[i]);
			if (meta == nullptr || meta->getTypeID() != TID_Texture)
				continue;

			setUUID(meta->getUUID());
			break;
		}
	}

	void GUITextureField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(BuiltinEditorResources::TextureFieldLabelStyleName));

		mDropButton->setStyle(getSubStyleName(BuiltinEditorResources::TextureFieldDropStyleName));
		mClearButton->setStyle(getSubStyleName(BuiltinEditorResources::TextureFieldClearBtnStyleName));
	}

	void GUITextureField::onClearButtonClicked()
	{
		setUUID(UUID::EMPTY);
	}

	const String& GUITextureField::getGUITypeName()
	{
		return BuiltinEditorResources::TextureFieldStyleName;
	}
}