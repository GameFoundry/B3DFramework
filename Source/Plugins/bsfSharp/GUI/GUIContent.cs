//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.CompilerServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    public partial struct GUIContent
    {
        /// <summary>
        /// Returns image content (if any).
        /// </summary>
        public SpriteTexture GetImage(GUIElementState state = GUIElementState.Normal)
        {
            switch (state)
            {
            case GUIElementState.Normal:
                return images.normal;
            case GUIElementState.Hover:
                return images.hover;
            case GUIElementState.Active:
                return images.active;
            case GUIElementState.Focused:
                return images.focused;
            case GUIElementState.NormalOn:
                return images.normalOn;
            case GUIElementState.HoverOn:
                return images.hoverOn;
            case GUIElementState.ActiveOn:
                return images.activeOn;
            case GUIElementState.FocusedOn:
                return images.focusedOn;
            default:
                return images.normal;
            }
        }

        /// <summary>
        /// Implicitly converts a localized string into a GUI content containing only text.
        /// </summary>
        /// <param name="text">Localized string to initialize the GUI content with.</param>
        /// <returns>GUI content containing only a string.</returns>
        public static implicit operator GUIContent(LocString text)
        {
            return new GUIContent(text);
        }

        /// <summary>
        /// Implicitly converts a string into a GUI content containing only text.
        /// </summary>
        /// <param name="text">String to initialize the GUI content with.</param>
        /// <returns>GUI content containing only a string.</returns>
        public static implicit operator GUIContent(string text)
        {
            return new GUIContent(new LocString(text));
        }
    }

    public partial struct GUIContentImages
    {
        /// <summary>
        /// Creates a new object where content images for on and off states are different.
        /// </summary>
        /// <param name="imageOff">Image to assign to all off states.</param>
        /// <param name="imageOn">Image to assign to all on states.</param>
        public GUIContentImages(SpriteTexture imageOff, SpriteTexture imageOn)
        {
            normal = imageOff;
            hover = imageOff;
            active = imageOff;
            focused = imageOff;
            normalOn = imageOn;
            hoverOn = imageOn;
            activeOn = imageOn;
            focusedOn = imageOn;
        }

        /// <summary>
        /// Implicitly converts a sprite texture into a GUI content images object.
        /// </summary>
        /// <param name="image">Image to instantiate the GUI content images with.</param>
        /// <returns>GUI content images with all states set to the provided image.</returns>
        public static implicit operator GUIContentImages(SpriteTexture image)
        {
            return new GUIContentImages(image);
        }
    }

    /** @} */
}
