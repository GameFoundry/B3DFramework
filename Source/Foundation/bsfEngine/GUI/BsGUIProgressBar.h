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
	class BS_EXPORT GUIProgressBar : public GUIElementContainer
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**	Name of the style for the fill image used by the progress bar. */
		static const String& GetBarStyleType();

		/**	Name of the style for the background image used by the progress bar. */
		static const String& GetBackgroundStyleType();

		/**
		 * Creates a new progress bar.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIProgressBar* Create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new progress bar.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIProgressBar* Create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Fills up the progress bar up to the specified percentage.
		 *
		 * @param[in]	pct	How far to extend the fill image, in percent ranging [0.0f, 1.0f]
		 */
		void SetPercent(float pct);

		/**	Gets the percentage of how full is the progress bar currently. */
		float GetPercent() const { return mPercent; }

		/** @copydoc GUIElement::setTint */
		virtual void SetTint(const Color& color) override;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementContainer::GetOptimalSizeInternal */
		virtual Vector2I GetOptimalSizeInternal() const override;

		/** @} */
	protected:
		GUIProgressBar(const String& styleName, const GUIDimensions& dimensions);

		/** @copydoc GUIElementContainer::_updateLayoutInternal */
		virtual void UpdateLayoutInternalInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElementContainer::styleUpdated */
		void StyleUpdated() override;

	private:
		GUITexture* mBar;
		GUITexture* mBackground;

		float mPercent;
	};

	/** @} */
}
