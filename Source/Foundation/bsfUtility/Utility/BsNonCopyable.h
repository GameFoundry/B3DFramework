//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** Interface that prevents copies be made of any type that implements it. */
	class INonCopyable
	{
	protected:
		INonCopyable() = default;
		~INonCopyable() = default;

	private:
		INonCopyable(const INonCopyable&) = delete;
		INonCopyable& operator=(const INonCopyable&) = delete;
	};

	/** @} */
}
