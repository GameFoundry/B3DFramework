//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	A resource containing plain text data. */
	class B3D_EXPORT PlainText : public Resource
	{
	public:
		/**	Returns the text contained in the resource. */
		const WString& GetString() const { return mString; }

		/**	Modifies the text contained in the resource. */
		void SetString(const WString& data) { mString = data; }

		/**	Creates a new text file resource with the specified string. */
		static HPlainText Create(const WString& data);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates an include file resource with the specified include string.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static TShared<PlainText> CreateShared(const WString& data);

		/** @} */
	private:
		PlainText(const WString& data);

		WString mString;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PlainTextRTTI;
		static RTTIType* GetRttiStatic();
		virtual RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
