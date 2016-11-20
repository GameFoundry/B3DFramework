//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsMeshData.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for BoneWeight. */
	class BS_SCR_BE_EXPORT ScriptBoneWeight : public ScriptObject <ScriptBoneWeight>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "BoneWeight")

		/**	Unboxes a boxed managed BoneWeight struct and returns the native version of the structure. */
		static BoneWeight unbox(MonoObject* obj);

		/** Boxes a native BoneWeight struct and returns a managed object containing it. */
		static MonoObject* box(const BoneWeight& value);

	private:
		ScriptBoneWeight(MonoObject* instance);
	};

	/** @} */
}