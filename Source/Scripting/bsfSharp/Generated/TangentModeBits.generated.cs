//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Flags that are used for describing how are tangents calculated for a specific keyframe in an animation curve.  Modes 
	/// for &quot;in&quot; and &quot;out&quot; tangents can be combined.
	/// </summary>
	public enum TangentMode
	{
		/// <summary>
		/// Tangent is infinite, ensuring there is a instantaneus jump between current and next keyframe value.
		/// </summary>
		OutStep = 514,
		/// <summary>Both tangents are calculated automatically based on the two surrounding keyframes.</summary>
		Auto = 0,
		/// <summary>Left tangent is calculated automatically based on the two surrounding keyframes.</summary>
		InAuto = 5,
		/// <summary>Tangent is calculated automatically based on the next keyframe.</summary>
		OutLinear = 258,
		/// <summary>Left tangent is manually adjusted by the user.</summary>
		InFree = 9,
		/// <summary>
		/// Tangent is infinite, ensuring there is a instantaneus jump between previous and current keyframe value.
		/// </summary>
		InStep = 33,
		/// <summary>Tangent is calculated automatically based on the previous keyframe.</summary>
		InLinear = 17,
		/// <summary>Right tangent is manually adjusted by the user.</summary>
		OutFree = 130,
		/// <summary>Right tangents are calculated automatically based on the two surrounding keyframes.</summary>
		OutAuto = 66,
		/// <summary>Both tangents are manually adjusted by the user.</summary>
		Free = 1024
	}

	/** @} */
}
