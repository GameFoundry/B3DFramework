//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
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
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) StockIcons : public Module<StockIcons>
	{
	public:
		/**
		 * Retrieves a particular stock icon.
		 *
		 * @param	icon		Icon to retrieve.
		 * @param	size		Size of the icon in points.
		 */
		B3D_SCRIPT_EXPORT()
		HSpriteImage GetIcon(StockIcon icon, u32 size = 8) const;

		/** Returns the unicode character corresponding to an icon. */
		B3D_SCRIPT_EXPORT()
		u32 GetUnicode(StockIcon icon) const;

		/** Returns the font in which the provided icon is stored in. */
		B3D_SCRIPT_EXPORT()
		HFont GetFont(StockIcon icon) const;

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

			StockIconKey(StockIcon icon, u32 size)
				: Icon(icon), Size(size)
			{
			}

			StockIcon Icon = StockIcon::None;
			u32 Size = 8;
		};

		mutable TUnorderedMap<StockIconKey, WeakResourceHandle<SpriteImage>> mCache;
		UnorderedMap<String, StockIcon> mNameLookup;
	};

	/** @} */
} // namespace bs
