//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
			value.Name = "";
			value.Flags = (AnimationCurveFlags)0;
			value.Curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedFloatCurve(string name, AnimationCurve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedFloatCurve(string name, AnimationCurveFlags flags, AnimationCurve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Name of the curve.</summary>
		public string Name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags Flags;
		/// <summary>Actual curve containing animation data.</summary>
		public AnimationCurve Curve;
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
			value.Name = "";
			value.Flags = (AnimationCurveFlags)0;
			value.Curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector3Curve(string name, Vector3Curve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector3Curve(string name, AnimationCurveFlags flags, Vector3Curve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Name of the curve.</summary>
		public string Name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags Flags;
		/// <summary>Actual curve containing animation data.</summary>
		public Vector3Curve Curve;
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
			value.Name = "";
			value.Flags = (AnimationCurveFlags)0;
			value.Curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector2Curve(string name, Vector2Curve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedVector2Curve(string name, AnimationCurveFlags flags, Vector2Curve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Name of the curve.</summary>
		public string Name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags Flags;
		/// <summary>Actual curve containing animation data.</summary>
		public Vector2Curve Curve;
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
			value.Name = "";
			value.Flags = (AnimationCurveFlags)0;
			value.Curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedQuaternionCurve(string name, QuaternionCurve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedQuaternionCurve(string name, AnimationCurveFlags flags, QuaternionCurve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Name of the curve.</summary>
		public string Name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags Flags;
		/// <summary>Actual curve containing animation data.</summary>
		public QuaternionCurve Curve;
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
			value.Name = "";
			value.Flags = (AnimationCurveFlags)0;
			value.Curve = null;

			return value;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedIntegerCurve(string name, IntegerCurve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Constructs a new named animation curve.</summary>
		/// <param name="name">Name of the curve.</param>
		/// <param name="flags">Flags that describe the animation curve.</param>
		/// <param name="curve">Curve containing the animation data.</param>
		public NamedIntegerCurve(string name, AnimationCurveFlags flags, IntegerCurve curve)
		{
			this.Name = "";
			this.Flags = (AnimationCurveFlags)0;
			this.Curve = null;
		}

		/// <summary>Name of the curve.</summary>
		public string Name;
		/// <summary>Flags that describe the animation curve.</summary>
		public AnimationCurveFlags Flags;
		/// <summary>Actual curve containing animation data.</summary>
		public IntegerCurve Curve;
	}

	/** @} */
}
