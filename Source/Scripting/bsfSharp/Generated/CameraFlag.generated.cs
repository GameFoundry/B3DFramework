//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Flags for controlling Camera options.</summary>
	public enum CameraFlag
	{
		/// <summary>
		/// If set the camera will only render when requested by the user through Camera::notifyNeedsRedraw(). Otherwise the 
		/// camera will render every frame (unless disabled).
		/// </summary>
		OnDemand = 1
	}

	/** @} */
}
