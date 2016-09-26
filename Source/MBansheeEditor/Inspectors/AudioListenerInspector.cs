//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="AudioListener"/> component.
    /// </summary>
    [CustomInspector(typeof(AudioListener))]
    internal class AudioListenerInspector : Inspector
    {
        /// <inheritdoc/>
        protected internal override void Initialize()
        {

        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            return InspectableState.NotModified;
        }
    }

    /** @} */
}
