//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementContainer.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * GUI element containing a background image and a fill image that is scaled depending on the percentage set by the
	 * caller.
	 */
	class B3D_EXPORT GUIProgressBar : public GUIElementContainer
	{
	public:
		static constexpr const char* kProgressBarFillStyleClass = "ProgressBarFill"; /**< Style class for the progress bar fill. */
		static constexpr const char* kProgressBarBackgroundStyleClass = "ProgressBarBackground"; /**< Style class for the progress bar background. */

		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new progress bar.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIProgressBar* Create(const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new progress bar.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIProgressBar* Create(const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Fills up the progress bar up to the specified percentage.
		 *
		 * @param[in]	pct	How far to extend the fill image, in percent ranging [0.0f, 1.0f]
		 */
		void SetPercent(float pct);

		/**	Gets the percentage of how full is the progress bar currently. */
		float GetPercent() const { return mPercent; }

		void SetTint(const Color& color) override;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		Vector2I CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		GUIProgressBar(const String& styleName, const GUISizeConstraints& dimensions);

		void UpdateLayoutRecursive(const GUILayoutData& data) override;

	private:
		GUITexture* mBar;
		GUITexture* mBackground;

		float mPercent;
	};

	/** @} */
} // namespace bs
