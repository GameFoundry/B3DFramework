//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Animation
     *  @{
     */

    /// <summary>
    /// Name and weight of a single shape in a morph target animation. Each shape internally represents a set of vertices
    /// that describe the morph shape.
    /// </summary>
    public class MorphShape
    {
        /// <summary>
        /// Unique name of the shape within a channel.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// Weight of the shape, determining how are different shapes within a channel blended.
        /// </summary>
        public float Weight { get; }

        /// <summary>
        /// Constructor for internal runtime use.
        /// </summary>
        /// <param name="name">Unique name of the shape within a channel.</param>
        /// <param name="weight">Weight in range [0, 1]. Determines how are sequential shapes animated between within a
        ///                      morph channel.e.g. if there is a shape A with weight 0.3 and shape B with weight 0.8
        ///                      then shape A will be displayed first and then be blended towards shape B as time passes.
        ///                      </param>
        private MorphShape(string name, float weight)
        {
            Name = name;
            Weight = weight;
        }
    }

    /// <summary>
    /// A collection of morph shapes that are sequentially blended together. Each shape has a weight in range [0, 1] which
    /// determines at what point is that shape blended. As the channel percent moves from 0 to 1, different shapes will be
    /// blended with those before or after them, depending on their weight.
    /// </summary>
    public class MorphChannel
    {
        /// <summary>
        /// Unique name of the channel, within a single morph animation.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// All morph shapes within a channel, in order from lowest to highest weight.
        /// </summary>
        public MorphShape[] Shapes { get; }

        /// <summary>
        /// Constructor for internal runtime use.
        /// </summary>
        /// <param name="name">Unique name of the channel, within a single morph animation.</param>
        /// <param name="shapes">A set of shapes beloning to a channel.</param>
        private MorphChannel(string name, MorphShape[] shapes)
        {
            Name = name;
            Shapes = shapes;
        }
    }

    /// <summary>
    /// Contains a set of morph shapes, used for morph target animation. Each morph shape contains a single possible shape
    /// that can be added on top of the base shape in order to create the animation.
    /// </summary>
    public class MorphShapes : ScriptObject
    {
        /// <summary>
        /// Constructor for internal runtime use only.
        /// </summary>
        private MorphShapes()
        { }

        /// <summary>
        /// Returns a list of all available morph channels.
        /// </summary>
        public MorphChannel[] Channels
        {
            get { return Internal_GetChannels(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern MorphChannel[] Internal_GetChannels(IntPtr instance);
    }

    /** @} */
}
