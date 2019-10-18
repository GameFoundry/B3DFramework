//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/BsCoreApplication.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderWindow.h"
#include "BsScriptRENDER_WINDOW_DESC.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	struct __START_UP_DESCInterop
	{
		MonoString* renderAPI;
		MonoString* renderer;
		MonoString* physics;
		MonoString* audio;
		MonoString* input;
		bool physicsCooking;
		bool asyncAnimation;
		__RENDER_WINDOW_DESCInterop primaryWindowDesc;
		MonoArray* importers;
	};

	class BS_SCR_BE_EXPORT ScriptSTART_UP_DESC : public ScriptObject<ScriptSTART_UP_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "StartUpDesc")

		static MonoObject* box(const __START_UP_DESCInterop& value);
		static __START_UP_DESCInterop unbox(MonoObject* value);
		static START_UP_DESC fromInterop(const __START_UP_DESCInterop& value);
		static __START_UP_DESCInterop toInterop(const START_UP_DESC& value);

	private:
		ScriptSTART_UP_DESC(MonoObject* managedInstance);

	};
#endif
}
