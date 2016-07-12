//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	class ScriptAnimation;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Renderable. */
	class BS_SCR_BE_EXPORT ScriptRenderable : public ScriptObject < ScriptRenderable >
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeRenderable")

		/**	Returns the native wrapped renderable. */
		SPtr<Renderable> getInternal() const { return mRenderable; }

	private:
		ScriptRenderable(MonoObject* managedInstance, const HSceneObject& parentSO);
		~ScriptRenderable();

		/** Updates the internal transform of the renderable handled according to the scene object it is attached to. */
		void updateTransform(const HSceneObject& parent);

		/**	Destroys the internal renderable object. */
		void destroy();

		/** @copydoc ScriptObject::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted() override;

		SPtr<Renderable> mRenderable;
		UINT32 mLastUpdateHash;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* instance, ScriptSceneObject* parentSO);
		static void internal_SetAnimation(ScriptRenderable* thisPtr, ScriptAnimation* animation);
		static void internal_UpdateTransform(ScriptRenderable* thisPtr, ScriptSceneObject* parentSO);
		static void internal_SetMesh(ScriptRenderable* thisPtr, ScriptMesh* mesh);
		static void internal_GetBounds(ScriptRenderable* thisPtr, ScriptSceneObject* parentSO, AABox* box, Sphere* sphere);
		static UINT64 internal_GetLayers(ScriptRenderable* thisPtr);
		static void internal_SetLayers(ScriptRenderable* thisPtr, UINT64 layers);
		static void internal_SetMaterial(ScriptRenderable* thisPtr, ScriptMaterial* material, int index);
		static void internal_SetMaterials(ScriptRenderable* thisPtr, MonoArray* materials);
		static void internal_OnDestroy(ScriptRenderable* thisPtr);
	};

	/** @} */
}