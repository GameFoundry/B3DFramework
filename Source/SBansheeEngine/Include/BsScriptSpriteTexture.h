//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for SpriteTexture. */
	class BS_SCR_BE_EXPORT ScriptSpriteTexture : public TScriptResource<ScriptSpriteTexture, SpriteTexture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SpriteTexture")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

		/**	Returns the native internal sprite texture resource. */
		static MonoObject* toManaged(const HSpriteTexture& texture);

	private:
		friend class ScriptResourceManager;

		ScriptSpriteTexture(MonoObject* instance, const HSpriteTexture& texture);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* texture, Vector2* offset, Vector2* scale);
		static MonoObject* internal_GetTexture(ScriptSpriteTexture* thisPtr);
		static void internal_SetTexture(ScriptSpriteTexture* thisPtr, ScriptTexture* value);
		static void internal_GetOffset(ScriptSpriteTexture* thisPtr, Vector2* value);
		static void internal_SetOffset(ScriptSpriteTexture* thisPtr, Vector2* value);
		static void internal_GetScale(ScriptSpriteTexture* thisPtr, Vector2* value);
		static void internal_SetScale(ScriptSpriteTexture* thisPtr, Vector2* value);
		static UINT32 internal_GetWidth(ScriptSpriteTexture* thisPtr);
		static UINT32 internal_GetHeight(ScriptSpriteTexture* thisPtr);
	};

	/** @} */
}