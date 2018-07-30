//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
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
		GUISkinEntry() { }
		GUISkinEntry(const String& name, const GUIElementStyle& style);

		String name;
		GUIElementStyle style;

	public:
		friend class GUISkinEntryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	class BS_EXPORT GUISkinEntryRTTI : public RTTIType <GUISkinEntry, IReflectable, GUISkinEntryRTTI>
	{
	private:
		String& getName(GUISkinEntry* obj) { return obj->name; }
		void setName(GUISkinEntry* obj, String& val) { obj->name = val; }

		GUIElementStyle& getStyle(GUISkinEntry* obj) { return obj->style; }
		void setStyle(GUISkinEntry* obj, GUIElementStyle& val) { obj->style = val; }

	public:
		GUISkinEntryRTTI()
		{
			addPlainField("name", 0, &GUISkinEntryRTTI::getName, &GUISkinEntryRTTI::setName);
			addReflectableField("style", 1, &GUISkinEntryRTTI::getStyle, &GUISkinEntryRTTI::setStyle);
		}

		const String& getRTTIName() override
		{
			static String name = "GUISkinEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GUISkinEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<GUISkinEntry>();
		}
	};

	class BS_EXPORT GUISkinRTTI : public RTTIType <GUISkin, Resource, GUISkinRTTI>
	{
	private:
		GUISkinEntry& getStyle(GUISkin* obj, UINT32 idx) 
		{ 
			return mStyles[idx];
		}

		void setStyle(GUISkin* obj, UINT32 idx, GUISkinEntry& val) { obj->mStyles[val.name] = val.style; }
		UINT32 getStyleArraySize(GUISkin* obj) { return (UINT32)obj->mStyles.size(); }
		void setStyleArraySize(GUISkin* obj, UINT32 size) { /* Do nothing */ }

	public:
		GUISkinRTTI()
		{
			addReflectableArrayField("mStyles", 0, &GUISkinRTTI::getStyle, &GUISkinRTTI::getStyleArraySize,
				&GUISkinRTTI::setStyle, &GUISkinRTTI::setStyleArraySize);
		}

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			GUISkin* skin = static_cast<GUISkin*>(obj);

			for (auto& style : skin->mStyles)
				mStyles.push_back(GUISkinEntry(style.first, style.second));
		}

		const String& getRTTIName() override
		{
			static String name = "GUISkin";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GUISkin;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GUISkin::_createPtr();
		}

	private:
		Vector<GUISkinEntry> mStyles;
	};

	/** @} */
	/** @endcond */
}