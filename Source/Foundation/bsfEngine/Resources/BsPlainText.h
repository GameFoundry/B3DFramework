//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Resources-Engine
	 *  @{
	 */

	/**	A resource containing plain text data. */
	class BS_EXPORT PlainText : public Resource
	{
	public:
		/**	Returns the text contained in the resource. */
		const WString& getString() const { return mString; }

		/**	Modifies the text contained in the resource. */
		void setString(const WString& data) { mString = data; }

		/**	Creates a new text file resource with the specified string. */
		static HPlainText create(const WString& data);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates an include file resource with the specified include string.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<PlainText> CreatePtrInternal(const WString& data);

		/** @} */
	private:
		PlainText(const WString& data);

		WString mString;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PlainTextRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
