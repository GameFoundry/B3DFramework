//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	class ScriptRRefBase;

	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Handles creation and lookup of script interop objects for resources. Since resources can be created in native code
	 * yet used by managed code this manager provides lookups to find managed equivalents of native resources.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResourceManager : public Module<ScriptResourceManager>
	{
	public:
		ScriptResourceManager();
		~ScriptResourceManager();

		/**
		 * Attempts to find an existing interop object for the specified resource reference, or creates a new object if one
		 * cannot be found.
		 *
		 * @param[in]	resource		Resource handle to create the reference wrapper object for.
		 * @param[in]	rrefClass		Class of the managed RRef object to create.
		 */
		ScriptRRefBase* GetScriptRRef(const HResource& resource, ::MonoClass* rrefClass);

		/**
		 * Same as getScriptRRef(const HResource&, MonoClass*) except it automatically deduced the resource class from
		 * the provided template parameter.
		 */
		template <class T>
		ScriptRRefBase* GetScriptRRef(const TResourceHandle<T>& resource)
		{
			::MonoClass* rrefClass = ScriptResourceWrapper::GetResourceReferenceScriptClass(T::GetRttiStatic()->GetRttiId());
			return GetScriptRRef(resource, rrefClass);
		}

		/** Notifies the system that script object holding the resource reference has been garbage collected. */
		void NotifyScriptRRefScriptObjectDestroyed(ScriptRRefBase* scriptRRef);

	private:
		/**	Triggered when the native resource has been unloaded and therefore destroyed. */
		void OnResourceDestroyed(const UUID& UUID);

		/**
		 * Clears all cached RRefs. Should be called before assembly refresh since the refs will no longer be valid
		 * after.
		 */
		void ClearRRefs();

		UnorderedMap<::MonoClass*, UnorderedMap<UUID, ScriptRRefBase*>> mScriptRRefsPerType;

		HEvent mResourceDestroyedConn;
		HEvent mOnWillUnloadAssembliesConnection;
	};

	/** @} */
} // namespace b3d
