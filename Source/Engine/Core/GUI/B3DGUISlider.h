//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElementContainer.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	A slider with a draggable handle that can be vertical or horizontal. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUISlider : public GUIElementContainer
	{
	public:
		/**	Current position of the slider handle, in percent ranging [0.0f, 1.0f]. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(HandlePositionInPercent))
		void SetHandlePositionInPercent(float percent);

		/** @copydoc SetHandlePositionInPercent */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(HandlePositionInPercent))
		float GetHandlePositionInPercent() const;

		/**
		 * Current position of the slider handle, scaled within the current minimum and maximum range, rounded up to nearest step increment. If no range
		 * is provided, the range is [0, 1].
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(HandlePositionInRange))
		void SetHandlePositionInRange(float value);

		/** @copydoc SetHandlePositionInRange */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(HandlePositionInRange))
		float GetHandlePositionInRange() const;

		/** Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don't require clamping. */
		B3D_SCRIPT_EXPORT()
		void SetRange(float minimum, float maximum);

		/** Returns the minimum value of the slider */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RangeMinimum))
		float GetRangeMinimum() const;

		/** Returns the maximum value of the slider */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RangeMaximum))
		float GetRangeMaximum() const;

		/** Step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no step. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Step))
		void SetStep(float step);

		/** @copydoc SetStep. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Step))
		float GetStep() const;

		void SetTint(const Color& color) override;

		/** Triggered when the user changes the value of the slider. */
		B3D_SCRIPT_EXPORT()
		Event<void(float percent)> OnChanged;

		static constexpr const char* kHandleClassStyle = "SliderHandle";
		static constexpr const char* kBackgroundClassStyle = "SliderBackground";
		static constexpr const char* kFillClassStyle = "SliderFill";

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		const char* GetStyleSheetElement() const override { return "slider"; }
		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		GUISlider(bool horizontal, const String& styleName, const GUISizeConstraints& dimensions);
		virtual ~GUISlider();

		void UpdateLayoutForChildren() override;

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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIHorizontalSlider : public GUISlider, public TGUIConstructionMethodsWithoutContent<GUIHorizontalSlider>
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIVerticalSlider : public GUISlider, public TGUIConstructionMethodsWithoutContent<GUIVerticalSlider>
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
} // namespace b3d
