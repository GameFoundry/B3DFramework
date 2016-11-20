//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsImportOptions.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how is a shader imported. */
	class BS_CORE_EXPORT ShaderImportOptions : public ImportOptions
	{
	public:
		/** Returns a modifiable list of defines that will control shader compilation. */
		UnorderedMap<String, String>& getDefines() { return mDefines; }

		/** Returns a modifiable list of defines that will control shader compilation. */
		const UnorderedMap<String, String>& getDefines() const { return mDefines; }

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;

	private:
		UnorderedMap<String, String> mDefines;
	};

	/** @} */
}