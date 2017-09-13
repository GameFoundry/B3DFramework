//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIResourceField.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIDropButton.h"
#include "GUI/BsGUIButton.h"
#include "Resources/BsBuiltinResources.h"
#include "GUI/BsGUIResourceTreeView.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Resources/BsResources.h"
#include "Library/BsProjectLibrary.h"
#include "Library/BsProjectResourceMeta.h"
#include "BsManagedResourceMetaData.h"
#include "Utility/BsBuiltinEditorResources.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Scene/BsSelection.h"
#include "Serialization/BsScriptAssemblyManager.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIResourceField::DEFAULT_LABEL_WIDTH = 100;

	GUIResourceField::GUIResourceField(const PrivatelyConstruct& dummy, const String& typeNamespace, const String& type
		, const GUIContent& labelContent, UINT32 labelWidth, const String& style, const GUIDimensions& dimensions
		, bool withLabel)
		: GUIElementContainer(dimensions, style), mLabel(nullptr), mDropButton(nullptr), mClearButton(nullptr)
		, mNamespace(typeNamespace), mType(type)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if (withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), 
				getSubStyleName(BuiltinEditorResources::ObjectFieldLabelStyleName));
			mLayout->addElement(mLabel);
		}

		mDropButton = GUIDropButton::create((UINT32)DragAndDropType::Resources, GUIOptions(GUIOption::flexibleWidth()), 
			getSubStyleName(BuiltinEditorResources::ObjectFieldDropBtnStyleName));
		mClearButton = GUIButton::create(HString(L""), getSubStyleName(BuiltinEditorResources::ObjectFieldClearBtnStyleName));
		mClearButton->onClick.connect(std::bind(&GUIResourceField::onClearButtonClicked, this));

		mLayout->addElement(mDropButton);
		mLayout->addElement(mClearButton);

		mDropButton->onDataDropped.connect(std::bind(&GUIResourceField::dataDropped, this, _1));
		mDropButton->onClick.connect(std::bind(&GUIResourceField::onDropButtonClicked, this));
	}

	GUIResourceField::~GUIResourceField()
	{

	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText, UINT32 labelWidth, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText, const GUIOptions& options,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(options), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(), 0, *curStyle,
			GUIDimensions::create(options), false);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const GUIContent& labelContent,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, labelContent, DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText, UINT32 labelWidth,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), labelWidth, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const HString& labelText,
		const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(labelText), DEFAULT_LABEL_WIDTH, *curStyle,
			GUIDimensions::create(), true);
	}

	GUIResourceField* GUIResourceField::create(const String& typeNamespace, const String& type, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &BuiltinEditorResources::ObjectFieldStyleName;

		return bs_new<GUIResourceField>(PrivatelyConstruct(), typeNamespace, type, GUIContent(), 0, *curStyle,
			GUIDimensions::create(), false);
	}

	HResource GUIResourceField::getValue() const
	{
		if (!mUUID.empty())
			return gResources().loadFromUUID(mUUID, false, ResourceLoadFlag::Default | ResourceLoadFlag::KeepSourceData);

		return HResource();
	}

	void GUIResourceField::setValue(const HResource& value)
	{
		if (value)
		{
			Path resPath = gProjectLibrary().uuidToPath(value.getUUID());
			if (!resPath.isEmpty() || !value.isLoaded(false))
				setUUID(value.getUUID(), false);
			else // A non-project library resource
			{
				if (mUUID == value.getUUID())
					return;

				mUUID = value.getUUID();

				WString title = value->getName() + L" (" + toWString(mType) + L")";
				mDropButton->setContent(GUIContent(HEString(title)));
			}
		}
		else
			setUUID(UUID::EMPTY, false);
	}

	WeakResourceHandle<Resource> GUIResourceField::getValueWeak() const
	{
		if (!mUUID.empty())
			return Resources::instance()._getResourceHandle(mUUID).getWeak();

		return WeakResourceHandle<Resource>();
	}

	void GUIResourceField::setValueWeak(const WeakResourceHandle<Resource>& value)
	{
		if (value)
		{
			Path resPath = gProjectLibrary().uuidToPath(value.getUUID());
			if (!resPath.isEmpty() || !value.isLoaded(false))
				setUUID(value.getUUID(), false);
			else // A non-project library resource
			{
				if (mUUID == value.getUUID())
					return;

				mUUID = value.getUUID();

				WString title = value->getName() + L" (" + toWString(mType) + L")";
				mDropButton->setContent(GUIContent(HEString(title)));
			}
		}
		else
			setUUID(UUID::EMPTY, false);
	}

	void GUIResourceField::setUUID(const UUID& uuid, bool triggerEvent)
	{ 
		if (mUUID == uuid)
			return;

		mUUID = uuid;

		Path resPath = gProjectLibrary().uuidToPath(mUUID);
		if (!resPath.isEmpty())
		{
			WString title = resPath.getWFilename(false) + L" (" + toWString(mType) + L")";
			mDropButton->setContent(GUIContent(HEString(title)));
		}
		else
			mDropButton->setContent(GUIContent(HEString(L"None (" + toWString(mType) + L")")));

		if (triggerEvent)
		{
			HResource handle = gResources()._getResourceHandle(mUUID);
			onValueChanged(handle.getWeak());
		}
	}

	void GUIResourceField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mDropButton->setTint(color);
		mClearButton->setTint(color);
	}

	void GUIResourceField::_updateLayoutInternal(const GUILayoutData& data)
	{
		mLayout->_setLayoutData(data);
		mLayout->_updateLayoutInternal(data);
	}

	Vector2I GUIResourceField::_getOptimalSize() const
	{
		return mLayout->_getOptimalSize();
	}

	void GUIResourceField::onDropButtonClicked()
	{
		if (mUUID == UUID::EMPTY)
			return;

		Path resPath = gProjectLibrary().uuidToPath(mUUID);
		resPath = resPath.getRelative(gProjectLibrary().getResourcesFolder());

		Selection::instance().ping(resPath);
	}

	void GUIResourceField::dataDropped(void* data)
	{
		DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(data);
		UINT32 numResources = (UINT32)draggedResources->resourcePaths.size();

		if (numResources <= 0)
			return;

		MonoClass* acceptedClass = MonoManager::instance().findClass(mNamespace, mType);

		for (UINT32 i = 0; i < numResources; i++)
		{
			Path path = draggedResources->resourcePaths[i];

			SPtr<ProjectResourceMeta> meta = gProjectLibrary().findResourceMeta(path);
			if (meta == nullptr)
				continue;

			UINT32 typeId = meta->getTypeID();
			const UUID& uuid = meta->getUUID();

			bool found = false;

			BuiltinResourceInfo* resInfo = ScriptAssemblyManager::instance().getBuiltinResourceInfo(typeId);
			if (resInfo != nullptr)
			{
				if (resInfo->monoClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
			else if (typeId == TID_ManagedResource)
			{
				SPtr<ManagedResourceMetaData> managedResMetaData = std::static_pointer_cast<ManagedResourceMetaData>(meta->getResourceMetaData());
				MonoClass* providedClass = MonoManager::instance().findClass(managedResMetaData->typeNamespace, managedResMetaData->typeName);

				if (providedClass->isSubClassOf(acceptedClass))
				{
					setUUID(uuid);
					found = true;
				}
			}
			else
			{
				BS_EXCEPT(NotImplementedException, "Unsupported resource type added to resource field.");
			}

			if (found)
				break;
		}
	}

	void GUIResourceField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(BuiltinEditorResources::ObjectFieldLabelStyleName));

		mDropButton->setStyle(getSubStyleName(BuiltinEditorResources::ObjectFieldDropBtnStyleName));
		mClearButton->setStyle(getSubStyleName(BuiltinEditorResources::ObjectFieldClearBtnStyleName));
	}

	void GUIResourceField::onClearButtonClicked()
	{
		setUUID(UUID::EMPTY);
	}

	const String& GUIResourceField::getGUITypeName()
	{
		static String typeName = "GUIResourceField";
		return typeName;
	}
}