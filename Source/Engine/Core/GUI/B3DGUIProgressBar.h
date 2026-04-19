//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElementContainer.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * GUI element containing a background image and a fill image that is scaled depending on the percentage set by the
	 * caller.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIProgressBar : public GUIElementContainer, public TGUIConstructionMethodsWithoutContent<GUIProgressBar>
	{
	public:
		static constexpr const char* kProgressBarFillStyleClass = "ProgressBarFill"; /**< Style class for the progress bar fill. */
		static constexpr const char* kProgressBarBackgroundStyleClass = "ProgressBarBackground"; /**< Style class for the progress bar background. */

		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/** Determines how far is the progress bar filled, in range [0, 1]. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Percent))
		void SetPercent(float percent);

		/** @copydoc SetPercent */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Percent))
		float GetPercent() const { return mPercent; }

		void SetTint(const Color& color) override;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct {};
		GUIProgressBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		void UpdateLayoutForChildren() override;

	private:
		GUITexture* mBar;
		GUITexture* mBackground;

		float mPercent;
	};

	/** @} */
} // namespace b3d
