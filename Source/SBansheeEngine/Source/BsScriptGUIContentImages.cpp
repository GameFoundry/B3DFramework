//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIContentImages.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptResourceManager.h"

namespace bs
{
	MonoField* ScriptGUIContentImages::mNormalField;
	MonoField* ScriptGUIContentImages::mHoverField;
	MonoField* ScriptGUIContentImages::mActiveField;
	MonoField* ScriptGUIContentImages::mFocusedField;
	MonoField* ScriptGUIContentImages::mNormalOnField;
	MonoField* ScriptGUIContentImages::mHoverOnField;
	MonoField* ScriptGUIContentImages::mActiveOnField;
	MonoField* ScriptGUIContentImages::mFocusedOnField;

	ScriptGUIContentImages::ScriptGUIContentImages(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptGUIContentImages::initRuntimeData()
	{
		mNormalField = metaData.scriptClass->getField("normal");
		mHoverField = metaData.scriptClass->getField("hover");
		mActiveField = metaData.scriptClass->getField("active");
		mFocusedField = metaData.scriptClass->getField("focused");
		mNormalOnField = metaData.scriptClass->getField("normalOn");
		mHoverOnField = metaData.scriptClass->getField("hoverOn");
		mActiveOnField = metaData.scriptClass->getField("activeOn");
		mFocusedOnField = metaData.scriptClass->getField("focusedOn");
	}

	GUIContentImages ScriptGUIContentImages::getNative(MonoObject* instance)
	{
		GUIContentImages output;

		struct Entry
		{
			Entry(MonoField* field, HSpriteTexture& outputImage)
				:field(field), outputImage(outputImage)
			{ }

			MonoField* field;
			HSpriteTexture& outputImage;
		};

		Entry entries[8] = 
		{ 
			{ mNormalField, output.normal },
			{ mHoverField, output.hover },
			{ mActiveField, output.active },
			{ mFocusedField, output.focused },
			{ mNormalOnField, output.normalOn },
			{ mHoverOnField, output.hoverOn },
			{ mActiveOnField, output.activeOn },
			{ mFocusedOnField, output.focusedOn },
		};

		for (auto& entry : entries)
		{
			MonoObject* managedImage;
			entry.field->getValue(instance, &managedImage);

			if (managedImage != nullptr)
				entry.outputImage = ScriptSpriteTexture::toNative(managedImage)->getHandle();
		}

		return output;
	}

	MonoObject* ScriptGUIContentImages::getManaged(const GUIContentImages& images)
	{
		struct Entry
		{
			Entry(MonoField* field, const HSpriteTexture& outputImage)
				:field(field), image(outputImage)
			{ }

			MonoField* field;
			const HSpriteTexture& image;
		};

		Entry entries[8] =
		{
			{ mNormalField, images.normal },
			{ mHoverField, images.hover },
			{ mActiveField, images.active },
			{ mFocusedField, images.focused },
			{ mNormalOnField, images.normalOn },
			{ mHoverOnField, images.hoverOn },
			{ mActiveOnField, images.activeOn },
			{ mFocusedOnField, images.focusedOn },
		};

		MonoObject* output = metaData.scriptClass->createInstance();
		for (auto& entry : entries)
		{
			MonoObject* managedImage = nullptr;
			if (entry.image != nullptr)
			{
				ScriptSpriteTexture* scriptImage;
				ScriptResourceManager::instance().getScriptResource(entry.image, &scriptImage, true);

				managedImage = scriptImage->getManagedInstance();
			}

			entry.field->setValue(output, managedImage);
		}

		return output;
	}
}