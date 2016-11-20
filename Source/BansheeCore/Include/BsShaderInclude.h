//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"

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
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}