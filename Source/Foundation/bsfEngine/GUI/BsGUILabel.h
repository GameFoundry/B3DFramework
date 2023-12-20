//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIInteractable.h"
#include "2D/BsTextSprite.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUIContent.h"
#include "BsGUIConstructionMethods.h"

namespace bs
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

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct {};
		GUILabel(PrivatelyConstruct, const GUIContent& content, const String& styleClass, const GUISizeConstraints& dimensions);

		Vector2I CalculateUnconstrainedOptimalSize() const override;
		ElementType GetElementType() const override { return ElementType::Label; }
		const char* GetStyleSheetElement() const override { return "label"; }

		/** @} */
	protected:
		~GUILabel();

		u32 GetRenderElementDepthRange() const override;
		void UpdateRenderElements() override;

	private:

		GUIContent mContent;

		TextSprite* mTextSprite;
		ImageSprite* mBackgroundImageSprite;

		TextSpriteInformation mTextSpriteInformation;
		ImageSpriteInformation mImageSpriteInformation;
	};

	/** @} */
} // namespace bs
