//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Raw text resource that serves as an include file for shaders. */
	class B3D_EXPORT ShaderInclude : public Resource
	{
	public:
		/**	Text of the include file. */
		const String& GetString() const { return mString; }

		/**	Creates a new include file resource with the specified include string. */
		static HShaderInclude Create(const String& includeString);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates an include file resource with the specified include string.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<ShaderInclude> CreatePtrInternal(const String& includeString);

		/** @} */
	private:
		ShaderInclude(const String& includeString);

		String mString;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderIncludeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
