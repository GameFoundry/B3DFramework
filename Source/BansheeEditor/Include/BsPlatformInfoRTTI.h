//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsRTTIType.h"
#include "BsPlatformInfo.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class BS_ED_EXPORT PlatformInfoRTTI : public RTTIType <PlatformInfo, IReflectable, PlatformInfoRTTI>
	{
	private:
		WString& getDefines(PlatformInfo* obj) { return obj->defines; }
		void setDefines(PlatformInfo* obj, WString& val) { obj->defines = val; }

		PlatformType& getType(PlatformInfo* obj) { return obj->type; }
		void setType(PlatformInfo* obj, PlatformType& val) { obj->type = val; }

		WeakResourceHandle<Prefab>& getMainScene(PlatformInfo* obj) { return obj->mainScene; }
		void setMainScene(PlatformInfo* obj, WeakResourceHandle<Prefab>& val) { obj->mainScene = val; }

		bool& getFullscreen(PlatformInfo* obj) { return obj->fullscreen; }
		void setFullscreen(PlatformInfo* obj, bool& val) { obj->fullscreen = val; }

		UINT32& getWindowedWidth(PlatformInfo* obj) { return obj->windowedWidth; }
		void setWindowedWidth(PlatformInfo* obj, UINT32& val) { obj->windowedWidth = val; }

		UINT32& getWindowedHeight(PlatformInfo* obj) { return obj->windowedHeight; }
		void setWindowedHeight(PlatformInfo* obj, UINT32& val) { obj->windowedHeight = val; }

		bool& getDebug(PlatformInfo* obj) { return obj->debug; }
		void setDebug(PlatformInfo* obj, bool& val) { obj->debug = val; }

	public:
		PlatformInfoRTTI()
		{
			addPlainField("defines", 0, &PlatformInfoRTTI::getDefines, &PlatformInfoRTTI::setDefines);
			addPlainField("type", 1, &PlatformInfoRTTI::getType, &PlatformInfoRTTI::setType);
			addReflectableField("mainScene", 2, &PlatformInfoRTTI::getMainScene, &PlatformInfoRTTI::setMainScene);
			addPlainField("fullscreen", 3, &PlatformInfoRTTI::getFullscreen, &PlatformInfoRTTI::setFullscreen);
			addPlainField("windowedWidth", 4, &PlatformInfoRTTI::getWindowedWidth, &PlatformInfoRTTI::setWindowedWidth);
			addPlainField("windowedHeight", 5, &PlatformInfoRTTI::getWindowedHeight, &PlatformInfoRTTI::setWindowedHeight);
			addPlainField("debug", 6, &PlatformInfoRTTI::getDebug, &PlatformInfoRTTI::setDebug);
		}

		const String& getRTTIName() override
		{
			static String name = "PlatformInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PlatformInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<PlatformInfo>();
		}
	};

	class BS_ED_EXPORT WinPlatformInfoRTTI : public RTTIType <WinPlatformInfo, PlatformInfo, WinPlatformInfoRTTI>
	{
	private:
		WeakResourceHandle<Texture>& getIcon(WinPlatformInfo* obj) { return obj->icon; }
		void setIcon(WinPlatformInfo* obj, WeakResourceHandle<Texture>& val) { obj->icon = val; }

		WString& getTitlebarText(WinPlatformInfo* obj) { return obj->titlebarText; }
		void setTitlebarText(WinPlatformInfo* obj, WString& val) { obj->titlebarText = val; }

	public:
		WinPlatformInfoRTTI()
		{
			addReflectableField("icon", 9, &WinPlatformInfoRTTI::getIcon, &WinPlatformInfoRTTI::setIcon);
			addPlainField("titlebarText", 10, &WinPlatformInfoRTTI::getTitlebarText, &WinPlatformInfoRTTI::setTitlebarText);
		}

		const String& getRTTIName() override
		{
			static String name = "WinPlatformInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_WinPlatformInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<WinPlatformInfo>();
		}
	};

	/** @} */
	/** @endcond */
}