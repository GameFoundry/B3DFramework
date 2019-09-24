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

	/// <summary>Determines which parts of the scene will trigger motion blur.</summary>
	public enum MotionBlurDomain
	{
		/// <summary>Camera movement and rotation will result in full-screen motion blur.</summary>
		CameraOnly = 0,
		/// <summary>
		/// Object movement and rotation will result in blurring of the moving object. Can be significantly more expensive than 
		/// just using camera blur due to the requirement to use a velocity buffer (unless some other effect also requires it, in 
		/// which case it will be re-used).
		/// </summary>
		ObjectOnly = 1,
		/// <summary>Both the camera movement and object movement will result in motion blur.</summary>
		CameraAndObject = 2
	}

	/** @} */
}
