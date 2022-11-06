//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/BsCoreApplication.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderWindow.h"
#include "BsScriptRENDER_WINDOW_DESC.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	struct __START_UP_DESCInterop
	{
		MonoString* RenderApi;
		MonoString* Renderer;
		MonoString* Physics;
		MonoString* Audio;
		MonoString* Input;
		bool PhysicsCooking;
		bool AsyncAnimation;
		__RENDER_WINDOW_DESCInterop PrimaryWindowDesc;
		MonoArray* Importers;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSTART_UP_DESC : public ScriptObject<ScriptSTART_UP_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "StartUpDesc")

		static MonoObject* Box(const __START_UP_DESCInterop& value);
		static __START_UP_DESCInterop Unbox(MonoObject* value);
		static START_UP_DESC FromInterop(const __START_UP_DESCInterop& value);
		static __START_UP_DESCInterop ToInterop(const START_UP_DESC& value);

	private:
		ScriptSTART_UP_DESC(MonoObject* managedInstance);

	};
#endif
}
