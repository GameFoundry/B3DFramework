//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2018 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "Wrappers/BsScriptHandleSlider.h"
#include "Handles/BsHandleSlider2D.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for HandleSlider2D. */
	class BS_SCR_BED_EXPORT ScriptHandleSlider2D : public ScriptObject <ScriptHandleSlider2D, ScriptHandleSliderBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "HandleSlider2D")

	protected:
		/** @copydoc ScriptHandleSliderBase::getSlider */
		HandleSlider* getSlider() const override { return mSlider; }

		/** @copydoc ScriptHandleSliderBase::getSlider */
		void destroyInternal() override;

	private:
		ScriptHandleSlider2D(MonoObject* instance, UINT32 width, UINT32 height, Slider2DConstraint constraint, UINT64 layer);
		~ScriptHandleSlider2D();

		HandleSlider2D* mSlider;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, UINT32 width, UINT32 height, 
			Slider2DConstraint constraint, UINT64 layer);
		static void internal_GetDelta(ScriptHandleSlider2D* nativeInstance, Vector2I* value);
	};

	/** @} */
}
