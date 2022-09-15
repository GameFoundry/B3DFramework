//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "GUI/BsGUISkin.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT GUISkinEntry : public IReflectable
	{
	public:
		GUISkinEntry() = default;
		GUISkinEntry(const String& name, const GUIElementStyle& style);

		String name;
		GUIElementStyle style;

	public:
		friend class GUISkinEntryRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const ;
	};

	class BS_EXPORT GUISkinEntryRTTI : public RTTIType <GUISkinEntry, IReflectable, GUISkinEntryRTTI>
	{
	private:
		String& GetName(GUISkinEntry* obj) { return obj->name; }
		void SetName(GUISkinEntry* obj, String& val) { obj->name = val; }

		GUIElementStyle& GetStyle(GUISkinEntry* obj) { return obj->style; }
		void SetStyle(GUISkinEntry* obj, GUIElementStyle& val) { obj->style = val; }

	public:
		GUISkinEntryRTTI()
		{
			AddPlainField("name", 0, &GUISkinEntryRTTI::GetName, &GUISkinEntryRTTI::SetName);
			AddReflectableField("style", 1, &GUISkinEntryRTTI::GetStyle, &GUISkinEntryRTTI::SetStyle);
		}

		const String& GetRttiName() 
		{
			static String name = "GUISkinEntry";
			return name;
		}

		UINT32 GetRttiId() 
		{
			return TID_GUISkinEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<GUISkinEntry>();
		}
	};

	class BS_EXPORT GUISkinRTTI : public RTTIType <GUISkin, Resource, GUISkinRTTI>
	{
	private:
		GUISkinEntry& GetStyle(GUISkin* obj, UINT32 idx)
		{
			return mStyles[idx];
		}

		void SetStyle(GUISkin* obj, UINT32 idx, GUISkinEntry& val) { obj->mStyles[val.name] = val.style; }
		UINT32 GetStyleArraySize(GUISkin* obj) { return (UINT32)obj->mStyles.size(); }
		void SetStyleArraySize(GUISkin* obj, UINT32 size) { /* Do nothing */ }

	public:
		GUISkinRTTI()
		{
			AddReflectableArrayField("mStyles", 0, &GUISkinRTTI::GetStyle, &GUISkinRTTI::GetStyleArraySize,
				&GUISkinRTTI::SetStyle, &GUISkinRTTI::SetStyleArraySize);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			GUISkin* skin = static_cast<GUISkin*>(obj);

			for (auto& style : skin->mStyles)
				mStyles.push_back(GUISkinEntry(style.first, style.second));
		}

		const String& GetRttiName() override
		{
			static String name = "GUISkin";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_GUISkin;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return GUISkin::CreatePtrInternal();
		}

	private:
		Vector<GUISkinEntry> mStyles;
	};

	/** @} */
	/** @endcond */
}
