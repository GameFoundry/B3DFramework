//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Script
	 *  @{
	 */

	class IScriptObjectWrapper;

	/**
	 * Interface to be implemented by types that are exported to scripting. Such types should also be decorated with B3D_SCRIPT_EXPORT() macro, along
	 * with any fields or methods that should be exported.
	 */
	class B3D_UTILITY_EXPORT IScriptExportable
	{
	public:
		IScriptExportable() = default;

		IScriptExportable(const IScriptExportable& other);
		IScriptExportable(IScriptExportable&& other);

		virtual ~IScriptExportable();

		IScriptExportable& operator=(const IScriptExportable& other);
		IScriptExportable& operator=(IScriptExportable&& other);

		/**
		 * Returns the script object wrapper associated with this object. This will be null if the object was never passed to script world, or if was passed
		 * but has gone out of scope.
		 */
		IScriptObjectWrapper* GetScriptObjectWrapper() const { return mScriptObjectWrapper; }

	private:
		friend class IScriptObjectWrapper;

		/** Notifies the object that a script object wrapper has been created for it, allowing a script object to access the native object through it. */
		void AssociateWithScriptObjectWrapper(IScriptObjectWrapper* wrapper);

		/** Clears the currently associated script object wrapper. Generally called before the script object is destroyed. */
		void ClearAssociatedScriptObjectWrapper() { mScriptObjectWrapper = nullptr; }

		IScriptObjectWrapper* mScriptObjectWrapper = nullptr;
	};

	/** @} */
} // namespace bs
