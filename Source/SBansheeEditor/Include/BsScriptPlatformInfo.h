//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsPlatformInfo.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/** Base class for all C++/CLR interop objects dealing with specific PlatformInfo implementations. */
	class BS_SCR_BED_EXPORT ScriptPlatformInfoBase : public ScriptObjectBase
	{
	public:
		/**	Returns the internal native platform info object. */
		SPtr<PlatformInfo> getPlatformInfo() const { return mPlatformInfo; }

	protected:
		ScriptPlatformInfoBase(MonoObject* instance);
		virtual ~ScriptPlatformInfoBase() {}

		SPtr<PlatformInfo> mPlatformInfo;
	};

	/**	Interop class between C++ & CLR for PlatformInfo. */
	class BS_SCR_BED_EXPORT ScriptPlatformInfo : public ScriptObject <ScriptPlatformInfo, ScriptPlatformInfoBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "PlatformInfo")

		/** Creates a new managed platform info object that wraps the provided native platform info. */
		static MonoObject* create(const SPtr<PlatformInfo>& platformInfo);
	private:
		ScriptPlatformInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static PlatformType internal_GetType(ScriptPlatformInfoBase* thisPtr);
		static MonoString* internal_GetDefines(ScriptPlatformInfoBase* thisPtr);
		static void internal_SetDefines(ScriptPlatformInfoBase* thisPtr, MonoString* value);
		static MonoObject* internal_GetMainScene(ScriptPlatformInfoBase* thisPtr);
		static void internal_SetMainScene(ScriptPlatformInfoBase* thisPtr, ScriptResourceRef* prefabRef);
		static bool internal_GetFullscreen(ScriptPlatformInfoBase* thisPtr);
		static void internal_SetFullscreen(ScriptPlatformInfoBase* thisPtr, bool fullscreen);
		static void internal_GetResolution(ScriptPlatformInfoBase* thisPtr, UINT32* width, UINT32* height);
		static void internal_SetResolution(ScriptPlatformInfoBase* thisPtr, UINT32 width, UINT32 height);
		static bool internal_GetDebug(ScriptPlatformInfoBase* thisPtr);
		static void internal_SetDebug(ScriptPlatformInfoBase* thisPtr, bool debug);
	};

	/**	Interop class between C++ & CLR for WinPlatformInfo. */
	class BS_SCR_BED_EXPORT ScriptWinPlatformInfo : public ScriptObject <ScriptWinPlatformInfo, ScriptPlatformInfoBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "WinPlatformInfo")

		/**	Creates a new managed platform info object that wraps the provided native platform info. */
		static MonoObject* create(const SPtr<WinPlatformInfo>& platformInfo);

	private:
		ScriptWinPlatformInfo(MonoObject* instance);

		/**	Returns the internal native Windows platform info object. */
		SPtr<WinPlatformInfo> getWinPlatformInfo() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_GetIcon(ScriptWinPlatformInfo* thisPtr);
		static void internal_SetIcon(ScriptWinPlatformInfo* thisPtr, ScriptResourceRef* textureRef);
		static MonoString* internal_GetTitleText(ScriptWinPlatformInfo* thisPtr);
		static void internal_SetTitleText(ScriptWinPlatformInfo* thisPtr, MonoString* text);
	};

	/** @} */
}