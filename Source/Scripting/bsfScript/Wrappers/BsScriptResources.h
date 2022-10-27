//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Resources/BsResources.h"

namespace bs
{
#if BS_IS_BANSHEE3D
	class ScriptRRefBase;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Resources & GameResourceManager. */
	class BS_SCR_BE_EXPORT ScriptResources : public ScriptObject<ScriptResources>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Resources")

	private:
		ScriptResources(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalLoad(MonoString* path, ResourceLoadFlag flags);
		static MonoObject* InternalLoadFromUuid(UUID* uuid, ResourceLoadFlag flags);
		static MonoObject* InternalLoadAsync(MonoString* path, ResourceLoadFlag flags);
		static MonoObject* InternalLoadAsyncFromUuid(UUID* uuid, ResourceLoadFlag flags);
		static void InternalRelease(ScriptResourceBase* resource);
		static void InternalReleaseRef(ScriptRRefBase* resource);
		static void InternalUnloadUnused();
		static float InternalGetLoadProgress(ScriptRRefBase* resource, bool loadDependencies);
	};

	/** @} */
#endif
} // namespace bs
