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

	/// <summary>Event that is triggered when animation reaches a certain point.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct AnimationEvent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static AnimationEvent Default()
		{
			AnimationEvent value = new AnimationEvent();
			value.name = "";
			value.time = 0f;

			return value;
		}

		/// <summary>Constructs a new animation event.</summary>
		/// <param name="name">Name used to identify the event when triggered.</param>
		/// <param name="time">Time at which to trigger the event, in seconds.</param>
		public AnimationEvent(string name, float time)
		{
			this.name = name;
			this.time = time;
		}

		/// <summary>Name used to identify the event when triggered.</summary>
		public string name;
		/// <summary>Time at which to trigger the event, in seconds.</summary>
		public float time;
	}

	/** @} */
}
