//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "Wrappers/BsScriptHandleSlider.h"
#include "Handles/BsHandleSliderLine.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for HandleSliderLine. */
	class BS_SCR_BED_EXPORT ScriptHandleSliderLine : public ScriptObject <ScriptHandleSliderLine, ScriptHandleSliderBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "HandleSliderLine")

	protected:
		/** @copydoc ScriptHandleSliderBase::getSlider */
		HandleSlider* getSlider() const override { return mSlider; }

		/** @copydoc ScriptHandleSliderBase::getSlider */
		void destroyInternal() override;

	private:
		ScriptHandleSliderLine(MonoObject* instance, const Vector3& direction, float length, bool fixedScale, UINT64 layer);
		~ScriptHandleSliderLine();

		HandleSliderLine* mSlider;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, Vector3* direction, float length, bool fixedScale, UINT64 layer);
		static void internal_GetDelta(ScriptHandleSliderLine* nativeInstance, float* value);
	};

	/** @} */
}