//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	public partial class GUIUtility
	{
        /// <summary>
        /// Converts a value from physical pixels to logical pixels.
        /// </summary>
        /// <param name="value">Value in physical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in logical pixels.</returns>
        public static GUILogicalUnit PhysicalToLogicalPixels(GUIPhysicalUnit value, float DPIScale)
        {
            if (DPIScale == 0.0f)
                return new GUILogicalUnit((int)value);

            return new GUILogicalUnit(MathEx.RoundToInt((float)value * (1.0f / DPIScale)));
        }

        /// <summary>
        /// Converts a value from physical pixels to logical pixels.
        /// </summary>
        /// <param name="value">Value in physical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in logical pixels.</returns>
        public static GUILogicalUnitF PhysicalToLogicalPixels(GUIPhysicalUnitF value, float DPIScale)
        {
            if (DPIScale == 0.0f)
                return new GUILogicalUnitF((float)value);

            return new GUILogicalUnitF((float)value * (1.0f / DPIScale));
        }

        /// <summary>
        /// Converts a value from logical pixels to physical pixels.
        /// </summary>
        /// <param name="value">Value in logical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in physical pixels.</returns>
        public static GUIPhysicalUnit LogicalToPhysicalPixels(GUILogicalUnit value, float DPIScale)
        {
            return new GUIPhysicalUnit(MathEx.RoundToInt((float)value * DPIScale));
        }

        /// <summary>
        /// Converts a value from logical pixels to physical pixels.
        /// </summary>
        /// <param name="value">Value in logical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in physical pixels.</returns>
        public static GUIPhysicalUnitF LogicalToPhysicalPixels(GUILogicalUnitF value, float DPIScale)
        {
            return new GUIPhysicalUnitF((float)value * DPIScale);
        }

        /// <summary>
        /// Converts a value from physical pixels to logical pixels.
        /// </summary>
        /// <param name="value">Value in physical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in logical pixels.</returns>
        public static GUILogicalPoint PhysicalToLogicalPixels(GUIPhysicalPoint value, float DPIScale)
        {
            return new GUILogicalPoint(PhysicalToLogicalPixels(value.X, DPIScale), PhysicalToLogicalPixels(value.Y, DPIScale));
        }

        /// <summary>
        /// Converts a value from logical pixels to physical pixels.
        /// </summary>
        /// <param name="value">Value in logical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in physical pixels.</returns>
        public static GUIPhysicalPoint LogicalToPhysicalPixels(GUILogicalPoint value, float DPIScale)
        {
            return new GUIPhysicalPoint(LogicalToPhysicalPixels(value.X, DPIScale), LogicalToPhysicalPixels(value.Y, DPIScale));
        }

        /// <summary>
        /// Converts a value from physical pixels to logical pixels.
        /// </summary>
        /// <param name="value">Value in physical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in logical pixels.</returns>
        public static GUILogicalSize PhysicalToLogicalPixels(GUIPhysicalSize value, float DPIScale)
        {
            return new GUILogicalSize(PhysicalToLogicalPixels(value.Width, DPIScale), PhysicalToLogicalPixels(value.Height, DPIScale));
        }

        /// <summary>
        /// Converts a value from logical pixels to physical pixels.
        /// </summary>
        /// <param name="value">Value in logical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in physical pixels.</returns>
        public static GUIPhysicalSize LogicalToPhysicalPixels(GUILogicalSize value, float DPIScale)
        {
            return new GUIPhysicalSize(LogicalToPhysicalPixels(value.Width, DPIScale), LogicalToPhysicalPixels(value.Height, DPIScale));
        }

        /// <summary>
        /// Converts a value from physical pixels to logical pixels.
        /// </summary>
        /// <param name="value">Value in physical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in logical pixels.</returns>
        public static GUILogicalArea PhysicalToLogicalPixels(GUIPhysicalArea value, float DPIScale)
        {
            return new GUILogicalArea(PhysicalToLogicalPixels(value.X, DPIScale), PhysicalToLogicalPixels(value.Y, DPIScale),
                PhysicalToLogicalPixels(value.Width, DPIScale), PhysicalToLogicalPixels(value.Height, DPIScale));
        }

        /// <summary>
        /// Converts a value from logical pixels to physical pixels.
        /// </summary>
        /// <param name="value">Value in logical pixels.</param>
        /// <param name="DPIScale">DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.</param>
        /// <returns>Value in physical pixels.</returns>
        public static GUIPhysicalArea LogicalToPhysicalPixels(GUILogicalArea value, float DPIScale)
        {
            return new GUIPhysicalArea(LogicalToPhysicalPixels(value.X, DPIScale), LogicalToPhysicalPixels(value.Y, DPIScale),
                LogicalToPhysicalPixels(value.Width, DPIScale), LogicalToPhysicalPixels(value.Height, DPIScale));
        }
	}

	/** @} */
}
