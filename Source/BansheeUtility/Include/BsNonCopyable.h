//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
