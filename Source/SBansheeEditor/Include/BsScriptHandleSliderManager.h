//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**	Tracks all managed handle sliders. */
	class BS_SCR_BED_EXPORT ScriptHandleSliderManager : public Module<ScriptHandleSliderManager>
	{
	public:
		~ScriptHandleSliderManager();

		/**	Registers a new active managed handle slider. */
		void registerSlider(ScriptHandleSliderBase* slider);

		/**	Unregisters a managed handle slider when it is destroyed. */
		void unregisterSlider(ScriptHandleSliderBase* slider);

	private:
		Set<ScriptHandleSliderBase*> mSliders;
	};

	/** @} */
}