//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGUIOptions.h"
#include "B3DGUIRenderable.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**
	 * Provides all needed permutations of static Create methods used for creating a GUI element.
	 *
	 * Note @p GUIElementType must have a constructor of signature(PrivatelyConstruct, const ContentType&, const String&, const GUISizeConstraints&).
	 */
	template <class GUIElementType, class ContentType>
	class TGUIConstructionMethods
	{
	public:
		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const ContentType& contents, const String& styleClass, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), contents, GUIRenderable::GetStyleClass<GUIElementType>(styleClass), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 */
		static GUIElementType* Create(const ContentType& contents, const String& styleClass)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), contents, GUIRenderable::GetStyleClass<GUIElementType>(styleClass), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const ContentType& contents, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), contents, GUIRenderable::GetStyleClass<GUIElementType>(StringUtility::kBlank), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	contents			Structure describing the contents of the GUI element to create.
		 */
		static GUIElementType* Create(const ContentType& contents)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), contents, GUIRenderable::GetStyleClass<GUIElementType>(StringUtility::kBlank), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const String& styleClass, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), ContentType(), GUIRenderable::GetStyleClass<GUIElementType>(styleClass), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 */
		static GUIElementType* Create(const String& styleClass)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), ContentType(), GUIRenderable::GetStyleClass<GUIElementType>(styleClass), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), ContentType(), GUIRenderable::GetStyleClass<GUIElementType>(StringUtility::kBlank), GUISizeConstraints::Create(options));
		}

		/** Creates a new GUI element. */
		static GUIElementType* Create()
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), ContentType(), GUIRenderable::GetStyleClass<GUIElementType>(StringUtility::kBlank), GUISizeConstraints::Create());
		}
	};

	/**
	 * Provides all needed permutations of static Create methods used for creating a GUI element.
	 *
	 * Note @p GUIElementType must have a constructor of signature(PrivatelyConstruct, const String&, const GUISizeConstraints&).
	 */
	template <class GUIElementType>
	class TGUIConstructionMethodsWithoutContent
	{
	public:
		/**
		 * Creates a new GUI element.
		 *
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(const String& styleClass, B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), GUIRenderable::GetStyleClass<GUIElementType>(styleClass), GUISizeConstraints::Create(options));
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	styleClass			Style class that will be used for determining GUI element visuals from the current style sheet. If no class is provided, default style is determined based on GUI element type.
		 */
		static GUIElementType* Create(const String& styleClass)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), GUIRenderable::GetStyleClass<GUIElementType>(styleClass), GUISizeConstraints::Create());
		}

		/**
		 * Creates a new GUI element.
		 *
		 * @param	options				Additional options that control GUI element size and position. This will override options set in the style sheet.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static GUIElementType* Create(B3D_PARAMS const TInlineArray<GUIOption, 4>& options)
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), GUIRenderable::GetStyleClass<GUIElementType>(StringUtility::kBlank), GUISizeConstraints::Create(options));
		}

		/** Creates a new GUI element. */
		static GUIElementType* Create()
		{
			return B3DNew<GUIElementType>(typename GUIElementType::PrivatelyConstruct(), GUIRenderable::GetStyleClass<GUIElementType>(StringUtility::kBlank), GUISizeConstraints::Create());
		}
	};

	/** @} */
} // namespace b3d
