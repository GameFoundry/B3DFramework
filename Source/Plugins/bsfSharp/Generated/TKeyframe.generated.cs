//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Animation keyframe, represented as an endpoint of a cubic hermite spline.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct KeyFrameInt
	{
		/// <summary>Value of the key.</summary>
		public int value;
		/// <summary>Position of the key along the animation spline.</summary>
		public float time;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Animation keyframe, represented as an endpoint of a cubic hermite spline.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct KeyFrame
	{
		/// <summary>Value of the key.</summary>
		public float value;
		/// <summary>Input tangent (going from the previous key to this one) of the key.</summary>
		public float inTangent;
		/// <summary>Output tangent (going from this key to next one) of the key.</summary>
		public float outTangent;
		/// <summary>Position of the key along the animation spline.</summary>
		public float time;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Animation keyframe, represented as an endpoint of a cubic hermite spline.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct KeyFrameVec3
	{
		/// <summary>Value of the key.</summary>
		public Vector3 value;
		/// <summary>Input tangent (going from the previous key to this one) of the key.</summary>
		public Vector3 inTangent;
		/// <summary>Output tangent (going from this key to next one) of the key.</summary>
		public Vector3 outTangent;
		/// <summary>Position of the key along the animation spline.</summary>
		public float time;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Animation keyframe, represented as an endpoint of a cubic hermite spline.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct KeyFrameVec2
	{
		/// <summary>Value of the key.</summary>
		public Vector2 value;
		/// <summary>Input tangent (going from the previous key to this one) of the key.</summary>
		public Vector2 inTangent;
		/// <summary>Output tangent (going from this key to next one) of the key.</summary>
		public Vector2 outTangent;
		/// <summary>Position of the key along the animation spline.</summary>
		public float time;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Animation keyframe, represented as an endpoint of a cubic hermite spline.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct KeyFrameQuat
	{
		/// <summary>Value of the key.</summary>
		public Quaternion value;
		/// <summary>Input tangent (going from the previous key to this one) of the key.</summary>
		public Quaternion inTangent;
		/// <summary>Output tangent (going from this key to next one) of the key.</summary>
		public Quaternion outTangent;
		/// <summary>Position of the key along the animation spline.</summary>
		public float time;
	}

	/** @} */
}
