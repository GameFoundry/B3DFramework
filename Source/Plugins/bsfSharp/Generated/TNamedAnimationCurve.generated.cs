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

	/// <summary>An animation curve and its name.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct NamedFloatCurve
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static NamedFloatCurve Default()
		{
			NamedFloatCurve value = new NamedFloatCurve();
			value.name = "";
			value.flags = (AnimationCurveFlags)0;
			value.curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedFloatCurve(string name, AnimationCurve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedFloatCurve(string name, AnimationCurveFlags flags, AnimationCurve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Name of the curve.</summary>
		public string name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags flags;
		/// <summary>Actual curve containing animation data.</summary>
		public AnimationCurve curve;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>An animation curve and its name.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct NamedVector3Curve
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static NamedVector3Curve Default()
		{
			NamedVector3Curve value = new NamedVector3Curve();
			value.name = "";
			value.flags = (AnimationCurveFlags)0;
			value.curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector3Curve(string name, Vector3Curve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector3Curve(string name, AnimationCurveFlags flags, Vector3Curve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Name of the curve.</summary>
		public string name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags flags;
		/// <summary>Actual curve containing animation data.</summary>
		public Vector3Curve curve;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>An animation curve and its name.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct NamedVector2Curve
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static NamedVector2Curve Default()
		{
			NamedVector2Curve value = new NamedVector2Curve();
			value.name = "";
			value.flags = (AnimationCurveFlags)0;
			value.curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector2Curve(string name, Vector2Curve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector2Curve(string name, AnimationCurveFlags flags, Vector2Curve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Name of the curve.</summary>
		public string name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags flags;
		/// <summary>Actual curve containing animation data.</summary>
		public Vector2Curve curve;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>An animation curve and its name.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct NamedQuaternionCurve
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static NamedQuaternionCurve Default()
		{
			NamedQuaternionCurve value = new NamedQuaternionCurve();
			value.name = "";
			value.flags = (AnimationCurveFlags)0;
			value.curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedQuaternionCurve(string name, QuaternionCurve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedQuaternionCurve(string name, AnimationCurveFlags flags, QuaternionCurve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Name of the curve.</summary>
		public string name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags flags;
		/// <summary>Actual curve containing animation data.</summary>
		public QuaternionCurve curve;
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>An animation curve and its name.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct NamedIntegerCurve
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static NamedIntegerCurve Default()
		{
			NamedIntegerCurve value = new NamedIntegerCurve();
			value.name = "";
			value.flags = (AnimationCurveFlags)0;
			value.curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedIntegerCurve(string name, IntegerCurve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedIntegerCurve(string name, AnimationCurveFlags flags, IntegerCurve curve)
		{
			this.name = name;
			this.flags = (AnimationCurveFlags)0;
			this.curve = curve;
		}

		/// <summary>Name of the curve.</summary>
		public string name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags flags;
		/// <summary>Actual curve containing animation data.</summary>
		public IntegerCurve curve;
	}

	/** @} */
}
