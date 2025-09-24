//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "2D/B3DTextSprite.h"
#include "2D/B3DImageSprite.h"
#include "GUI/B3DGUIContent.h"
#include "B3DGUIConstructionMethods.h"
#include "B3DGUISpriteHelper.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	GUI element that displays text and optionally a content image. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUILabel : public GUIInteractable, public TGUIConstructionMethods<GUILabel, GUIContent>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/** Changes the active content of the label. */
		B3D_SCRIPT_EXPORT()
		void SetContent(const GUIContent& content);

		static constexpr const char* kStyleSheetElementType = "label";
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct {};
		GUILabel(PrivatelyConstruct, const GUIContent& content, const String& styleClass, const GUISizeConstraints& dimensions);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;
		const char* GetStyleSheetElement() const override { return kStyleSheetElementType; }

		/** @} */
	protected:
		~GUILabel();

		u32 GetRenderElementDepthRange() const override;
		void UpdateRenderElements() override;

	private:

		GUIContent mContent;

		TextSprite* mTextSprite;
		GUIBackgroundSprite mBackgroundSprite;

		TextSpriteInformation mTextSpriteInformation;
	};

	/** @} */
} // namespace b3d
