//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Descriptor structure used for initialization of a SpriteVectorPath.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct SpriteVectorPathCreateInformation
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static SpriteVectorPathCreateInformation Default()
		{
			SpriteVectorPathCreateInformation value = new SpriteVectorPathCreateInformation();
			value.VectorPath = null;
			value.Size = Size2UI.Default();
			value.ScalingMode = VectorGraphicsRasterizationScaling.StretchToFit;
			value.UVRange = new Rect2(0f, 0f, 1f, 1f);
			value.AnimationPlayback = SpriteAnimationPlayback.None;
			value.Animation = SpriteSheetGridAnimation.Default();

			return value;
		}

		public SpriteVectorPathCreateInformation(SpriteImageInformation spriteImageInformation, RRef<VectorPath> vectorPath, Size2UI size)
		{
			this.VectorPath = vectorPath;
			this.Size = size;
			this.ScalingMode = VectorGraphicsRasterizationScaling.StretchToFit;
			this.UVRange = new Rect2(0f, 0f, 1f, 1f);
			this.AnimationPlayback = SpriteAnimationPlayback.None;
			this.Animation = SpriteSheetGridAnimation.Default();
		}

		///<summary>
		/// Returns a subset of this struct. This subset usually contains common fields shared with another struct.
		///</summary>
		public SpriteImageInformation GetBase()
		{
			SpriteImageInformation value;
			value.UVRange = UVRange;
			value.AnimationPlayback = AnimationPlayback;
			value.Animation = Animation;
			return value;
		}

		///<summary>
		/// Assigns values to a subset of fields of this struct. This subset usually contains common field shared with 
		/// another struct.
		///</summary>
		public void SetBase(SpriteImageInformation value)
		{
			UVRange = value.UVRange;
			AnimationPlayback = value.AnimationPlayback;
			Animation = value.Animation;
		}

		/// <summary>Vector path to render on the sprite.</summary>
		public RRef<VectorPath> VectorPath;
		/// <summary>Size of the rasterized path, in pixels.</summary>
		public Size2UI Size;
		/// <summary>How to scale the path canvas onto the rasterized destination.</summary>
		public VectorGraphicsRasterizationScaling ScalingMode;
		/// <summary>Range in the atlas texture that the image maps to.</summary>
		public Rect2 UVRange;
		/// <summary>Determines if animation is enabled and how should it play.</summary>
		public SpriteAnimationPlayback AnimationPlayback;
		/// <summary>Describes the sprite sheet grid used for animation, if animation is used.</summary>
		public SpriteSheetGridAnimation Animation;
	}

	/** @} */
}
