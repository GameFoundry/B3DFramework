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

	/// <summary>Contains information about a currently playing animation clip.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct AnimationClipState
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static AnimationClipState Default()
		{
			AnimationClipState value = new AnimationClipState();
			value.layer = 0;
			value.time = 0f;
			value.speed = 1f;
			value.weight = 1f;
			value.wrapMode = AnimWrapMode.Loop;
			value.stopped = false;

			return value;
		}

		/// <summary>Layer the clip is playing on. Multiple clips can be played simulatenously on different layers.</summary>
		public int layer;
		/// <summary>Current time the animation is playing from.</summary>
		public float time;
		/// <summary>Speed at which the animation is playing.</summary>
		public float speed;
		/// <summary>Determines how much of an influence does the clip have on the final pose.</summary>
		public float weight;
		/// <summary>Determines what happens to other animation clips when a new clip starts playing.</summary>
		public AnimWrapMode wrapMode;
		/// <summary>
		/// Determines should the time be advanced automatically. Certain type of animation clips don&apos;t involve playback  
		/// (e.g. for blending where animation weight controls the animation).
		/// </summary>
		public bool stopped;
	}

	/** @} */
}
