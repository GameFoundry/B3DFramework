//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Text
	 *  @{
	 */

	/** Contains a list of all icons that can be retrieved through the StockIcons module. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) StockIcon
	{
		None,
#define B3D_STOCK_ICON(Id, Identifier, Unicode, Label, Font) Identifier,
#include "Text/B3DStockIcons.generated.inc"
#undef B3D_STOCK_ICON
	};

	/** Provides easy access to variety of icons for use in the engine. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) StockIcons : public Module<StockIcons>
	{
	public:
		/**
		 * Retrieves a particular stock icon.
		 *
		 * @param	icon		Icon to retrieve.
		 * @param	size		Size of the icon in points.
		 */
		B3D_SCRIPT_EXPORT()
		B3D_NO_RREF HSpriteImage GetIcon(StockIcon icon, float size = 8.0f) const;

		/** Returns the unicode character corresponding to an icon. */
		B3D_SCRIPT_EXPORT()
		u32 GetUnicode(StockIcon icon) const;

		/** Returns the font in which the provided icon is stored in. */
		B3D_SCRIPT_EXPORT()
		B3D_NO_RREF HFont GetFont(StockIcon icon) const;

		/** Parses an icon name and returns the corresponding enum entry if found. */
		B3D_SCRIPT_EXPORT()
		StockIcon ParseIconName(const String& name);

	protected:
		void OnStartUp() override;
		void OnShutDown() override;
		
	private:
		/** Key for cache lookup. */
		struct StockIconKey
		{
			u64 GenerateHash() const
			{
				size_t hash = 0;
				B3DCombineHash(hash, Icon);
				B3DCombineHash(hash, Size);

				return hash;
			}

			bool operator==(const StockIconKey& other) const
			{
				return Icon == other.Icon && Size == other.Size;
			}

			StockIconKey(StockIcon icon, float size)
				: Icon(icon), Size(size)
			{
			}

			StockIcon Icon = StockIcon::None;
			float Size = 8.0f;
		};

		mutable TUnorderedMap<StockIconKey, TWeakResourceHandle<SpriteImage>> mCache;
		UnorderedMap<String, StockIcon> mNameLookup;
	};

	/** @} */
} // namespace b3d
