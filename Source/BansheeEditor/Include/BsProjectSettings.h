//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsSettings.h"

namespace bs
{
	/** @addtogroup Settings
	 *  @{
	 */

	/**	Contains various globally accessible project-specific preferences. */
	class BS_ED_EXPORT ProjectSettings : public Settings
	{
	public:
		ProjectSettings();

		/**	Retrieves the UUID of the last scene open in the editor. */
		String getLastOpenScene() const { return mLastOpenScene; }

		/**	Sets the UUID of the last scene open in the editor. */
		void setLastOpenScene(const String& value) { mLastOpenScene = value; markAsDirty(); }

	private:
		String mLastOpenScene;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ProjectSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}