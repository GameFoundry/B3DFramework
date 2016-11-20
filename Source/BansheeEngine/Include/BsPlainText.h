//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsResource.h"

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
		static SPtr<PlainText> _createPtr(const WString& data);

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