//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptHandleSlider.h"
#include "BsHandleSliderDisc.h"
#include "BsVector3.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for HandleSliderDisc. */
	class BS_SCR_BED_EXPORT ScriptHandleSliderDisc : public ScriptObject <ScriptHandleSliderDisc, ScriptHandleSliderBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "HandleSliderDisc")

	protected:
		/** @copydoc ScriptHandleSliderBase::getSlider */
		virtual HandleSlider* getSlider() const override { return mSlider; }

		/** @copydoc ScriptHandleSliderBase::destroyInternal */
		virtual void destroyInternal() override;

	private:
		ScriptHandleSliderDisc(MonoObject* instance, const Vector3& normal, float radius, bool fixedScale, UINT64 layer);
		~ScriptHandleSliderDisc();

		HandleSliderDisc* mSlider;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, Vector3* normal, float radius, bool fixedScale, UINT64 layer);
		static void internal_GetDelta(ScriptHandleSliderDisc* nativeInstance, float* value);
		static void internal_GetStartAngle(ScriptHandleSliderDisc* nativeInstance, float* value);
		static void internal_SetCutoffPlane(ScriptHandleSliderDisc* nativeInstance, float value, bool enabled);
	};

	/** @} */
}