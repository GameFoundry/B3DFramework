//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Raw text resource that serves as an include file for shaders. */
	class BS_CORE_EXPORT ShaderInclude : public Resource
	{
	public:
		/**	Text of the include file. */
		const String& getString() const { return mString; }

		/**	Creates a new include file resource with the specified include string. */
		static HShaderInclude create(const String& includeString);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates an include file resource with the specified include string.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<ShaderInclude> _createPtr(const String& includeString);

		/** @} */
	private:
		ShaderInclude(const String& includeString);

		String mString;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderIncludeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}