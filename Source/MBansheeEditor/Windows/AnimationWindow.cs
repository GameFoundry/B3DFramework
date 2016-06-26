//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window.
    /// </summary>
    internal class AnimationWindow : EditorWindow
    {
        private GUICanvas canvas;

        /// <summary>
        /// Opens the animation window.
        /// </summary>
        [MenuItem("Windows/Animation", ButtonModifier.CtrlAlt, ButtonCode.A, 6000)]
        private static void OpenGameWindow()
        {
            OpenWindow<AnimationWindow>();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Animation");
        }

        private void OnInitialize()
        {
            canvas = new GUICanvas(GUIOption.FixedWidth(200), GUIOption.FixedHeight(200));

            {
                Vector2I a = new Vector2I(0, 0);
                Vector2I b = new Vector2I(200, 0);
                Vector2I c = new Vector2I(200, 200);
                Vector2I d = new Vector2I(0, 200);

                canvas.DrawTriangleStrip(new Vector2I[] { b, c, a, d }, Color.BansheeOrange);
            }

            {
                Vector2I a = new Vector2I(50, 20);
                Vector2I b = new Vector2I(100, 20);
                Vector2I c = new Vector2I(240, 60);

                Vector2I[] vertices = {a, b};
                canvas.DrawPolyLine(vertices, 1.0f);
            }

            GUI.AddElement(canvas);
        }

        private void OnEditorUpdate()
        {
            //int position = (int)(MathEx.Sin(Time.RealElapsed)*50.0f + 50.0f);
            //canvas.SetPosition(position, 0);
        }

        private void OnDestroy()
        {
            // TODO
        }
    }

    /** @} */
}
