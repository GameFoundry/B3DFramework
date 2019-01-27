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
	/// Structure containing a reference to a keyframe tangent, as a keyframe reference and type of the tangent.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct TangentRef
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static TangentRef Default()
		{
			TangentRef value = new TangentRef();
			value.keyframeRef = new KeyframeRef();
			value.type = TangentType.In;

			return value;
		}

		public TangentRef(KeyframeRef keyframeRef, TangentType type)
		{
			this.keyframeRef = keyframeRef;
			this.type = type;
		}

		public KeyframeRef keyframeRef;
		public TangentType type;
	}

	/** @} */
}
