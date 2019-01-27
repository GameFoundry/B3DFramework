//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>
	/// Controls spring parameters for a physics joint limits. If a limit is soft (body bounces back due to restition when  
	/// the limit is reached) the spring will pull the body back towards the limit using the specified parameters.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Spring
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Spring Default()
		{
			Spring value = new Spring();
			value.stiffness = 0f;
			value.damping = 0f;

			return value;
		}

		/// <summary>Constructs a spring.</summary>
		/// <param name="stiffness">Spring strength. Force proportional to the position error.</param>
		/// <param name="damping">Damping strength. Force propertional to the velocity error.</param>
		public Spring(float stiffness, float damping)
		{
			this.stiffness = stiffness;
			this.damping = damping;
		}

		/// <summary>Spring strength. Force proportional to the position error.</summary>
		public float stiffness;
		/// <summary>Damping strength. Force propertional to the velocity error.</summary>
		public float damping;
	}

	/** @} */
}
