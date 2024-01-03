//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUIConstructionMethods.h"
#include "BsPrerequisites.h"
#include "GUI/BsGUIElementContainer.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	A slider with a draggable handle that can be vertical or horizontal. */
	class B3D_EXPORT GUISlider : public GUIElementContainer
	{
	public:
		/**
		 * Moves the slider handle the the specified position in the handle area.
		 *
		 * @param[in]	pct	Position to move the handle to, in percent ranging [0.0f, 1.0f]
		 */
		void SetPercent(float pct);

		/**	Gets the current position of the slider handle, in percent ranging [0.0f, 1.0f]. */
		float GetPercent() const;

		/**
		 * Gets the current value of the slider. This is the slider handle position percentage scaled within the current
		 * minimum and maximum range, rounded up to nearest step increment.
		 */
		float GetValue() const;

		/**	Sets a new value of the slider. This value should be within minimum and maximum range values. */
		void SetValue(float value);

		/**
		 * Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don't
		 * require clamping.
		 */
		void SetRange(float min, float max);

		/** Returns the minimum value of the slider */
		float GetRangeMinimum() const;

		/** Returns the maximum value of the slider */
		float GetRangeMaximum() const;

		/**
		 * Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no
		 * step.
		 */
		void SetStep(float step);

		/** Gets the minimum percentual variation of the handle position */
		float GetStep() const;

		void SetTint(const Color& color) override;

		/** Triggered when the user changes the value of the slider. */
		Event<void(float percent)> OnChanged;

		static constexpr const char* kHandleClassStyle = "SliderHandle";
		static constexpr const char* kBackgroundClassStyle = "SliderBackground";
		static constexpr const char* kFillClassStyle = "SliderFill";

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		const char* GetStyleSheetElement() const override { return "slider"; }
		Vector2I CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		GUISlider(bool horizontal, const String& styleName, const GUISizeConstraints& dimensions);
		virtual ~GUISlider();

		void UpdateLayoutRecursive(const GUILayoutData& data) override;

		/**	Triggered when the slider handles moves. */
		void OnHandleMoved(float newPosition, float newSize);

		bool DoOnCommandEvent(const GUICommandEvent& ev) override;

	private:
		GUISliderHandle* mSliderHandle;
		GUITexture* mBackground;
		GUITexture* mFillBackground;
		bool mHorizontal;
		float mMinRange = 0.0f;
		float mMaxRange = 1.0f;
		bool mHasFocus = false;

		HEvent mHandleMovedConn;
	};

	/** @} */

	/** @addtogroup GUI
	 *  @{
	 */

	/**	A horizontal slider with a draggable handle. */
	class B3D_EXPORT GUIHorizontalSlider : public GUISlider, public TGUIConstructionMethodsWithoutContent<GUIHorizontalSlider>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * @name Internal
		 * @{
		 */

		struct PrivatelyConstruct {};
		GUIHorizontalSlider(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& dimensions);

		/** @} */
	};

	/**	A vertical slider with a draggable handle. */
	class B3D_EXPORT GUIVerticalSlider : public GUISlider, public TGUIConstructionMethodsWithoutContent<GUIVerticalSlider>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * @name Internal
		 * @{
		 */

		struct PrivatelyConstruct {};
		GUIVerticalSlider (PrivatelyConstruct, const String& styleName, const GUISizeConstraints& dimensions);

		/** @} */
	};

	/** @} */
} // namespace bs
