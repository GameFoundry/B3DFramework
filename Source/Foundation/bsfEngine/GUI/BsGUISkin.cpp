//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIElementStyle.h"
#include "Debug/BsDebug.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsGUISkinRTTI.h"

namespace bs
{
	GUIElementStyle GUISkin::DefaultStyle;

	GUISkin::GUISkin()
		: Resource(false)
	{}

	GUISkin::GUISkin(const GUISkin& skin)
		: Resource(false)
	{}

	bool GUISkin::HasStyle(const String& name) const
	{
		auto iterFind = mStyles.find(name);

		if(iterFind != mStyles.end())
			return true;

		return false;
	}

	const GUIElementStyle* GUISkin::GetStyle(const String& guiElemType) const
	{
		auto iterFind = mStyles.find(guiElemType);

		if(iterFind != mStyles.end())
			return &iterFind->second;

		BS_LOG(Warning, GUI, "Cannot find GUI style with name: {0}. Returning default style.", guiElemType);

		return &DefaultStyle;
	}

	void GUISkin::SetStyle(const String& guiElemType, const GUIElementStyle& style)
	{
		mStyles[guiElemType] = style;
	}

	void GUISkin::RemoveStyle(const String& guiElemType)
	{
		mStyles.erase(guiElemType);
	}

	Vector<String> GUISkin::GetStyleNames() const
	{
		Vector<String> output;
		for(auto& pair : mStyles)
			output.push_back(pair.first);

		return output;
	}

	HGUISkin GUISkin::Create()
	{
		SPtr<GUISkin> newSkin = CreatePtrInternal();

		return static_resource_cast<GUISkin>(gResources().CreateResourceHandleInternal(newSkin));
	}

	SPtr<GUISkin> GUISkin::CreatePtrInternal()
	{
		SPtr<GUISkin> newSkin = bs_core_ptr<GUISkin>(new(bs_alloc<GUISkin>()) GUISkin());
		newSkin->SetThisPtrInternal(newSkin);
		newSkin->Initialize();

		return newSkin;
	}

	RTTITypeBase* GUISkin::GetRttiStatic()
	{
		return GUISkinRTTI::Instance();
	}

	RTTITypeBase* GUISkin::GetRtti() const
	{
		return GUISkin::GetRttiStatic();
	}
} // namespace bs
