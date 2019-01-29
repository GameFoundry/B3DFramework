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
	/// <summary>
	/// A buffer describing a volume (3D), image (2D) or line (1D) of pixels in memory. Pixels are stored as a succession  of 
	/// "depth" slices, each containing "height" rows of "width" pixels.
	/// </summary>
	[ShowInInspector]
	public partial class PixelData : ScriptObject
	{
		private PixelData(bool __dummy0) { }
		protected PixelData() { }

		public PixelData(PixelVolume volume, PixelFormat format = PixelFormat.BGRA8)
		{
			Internal_create(this, ref volume, format);
		}

		public PixelData(uint width, uint height, uint depth = 1, PixelFormat pixelFormat = PixelFormat.BGRA8)
		{
			Internal_create0(this, width, height, depth, pixelFormat);
		}

		/// <summary>
		/// Returns the number of pixels that offsets one row from another. This can be "width", but doesn't have to be as some 
		/// buffers require padding.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint RawRowPitch
		{
			get { return Internal_getRowPitch(mCachedPtr); }
		}

		/// <summary>
		/// Returns the number of pixels that offsets one depth slice from another. This can be "width * height", but  doesn't 
		/// have to be as some buffers require padding.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint RawSlicePitch
		{
			get { return Internal_getSlicePitch(mCachedPtr); }
		}

		/// <summary>Returns the pixel format used by the internal buffer for storing the pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public PixelFormat Format
		{
			get { return Internal_getFormat(mCachedPtr); }
		}

		/// <summary>Returns extents of the pixel volume this object is capable of holding.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public PixelVolume Extents
		{
			get
			{
				PixelVolume temp;
				Internal_getExtents(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Return whether this buffer is laid out consecutive in memory (meaning the pitches are equal to the dimensions).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool RawIsConsecutive
		{
			get { return Internal_isConsecutive(mCachedPtr); }
		}

		/// <summary>Return the size (in bytes) of the buffer this image requires.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint RawSize
		{
			get { return Internal_getSize(mCachedPtr); }
		}

		/// <summary>Returns a pixel at the specified location in the buffer.</summary>
		/// <param name="x">X coordinate of the pixel.</param>
		/// <param name="y">Y coordinate of the pixel.</param>
		/// <param name="z">Z coordinate of the pixel.</param>
		/// <returns>Value of the pixel, or undefined value if coordinates are out of range.</returns>
		public Color GetPixel(int x, int y, int z = 0)
		{
			Color temp;
			Internal_getPixel(mCachedPtr, x, y, z, out temp);
			return temp;
		}

		/// <summary>Sets a pixel at the specified location in the buffer.</summary>
		/// <param name="value">Color of the pixel to set.</param>
		/// <param name="x">X coordinate of the pixel.</param>
		/// <param name="y">Y coordinate of the pixel.</param>
		/// <param name="z">Z coordinate of the pixel.</param>
		public void SetPixel(Color value, int x, int y, int z = 0)
		{
			Internal_setPixel(mCachedPtr, ref value, x, y, z);
		}

		/// <summary>Returns values of all pixels.</summary>
		/// <returns>
		/// All pixels in the buffer ordered consecutively. Pixels are stored as a succession of "depth" slices,  each containing 
		/// "height" rows of "width" pixels.
		/// </returns>
		public Color[] GetPixels()
		{
			return Internal_getPixels(mCachedPtr);
		}

		/// <summary>
		/// Sets all pixels in the buffer.Caller must ensure that number of pixels match the extends of the buffer.
		/// </summary>
		/// <param name="value">
		/// All pixels in the buffer ordered consecutively. Pixels are stored as a succession of "depth" slices,  each containing 
		/// "height" rows of "width" pixels.
		/// </param>
		public void SetPixels(Color[] value)
		{
			Internal_setPixels(mCachedPtr, value);
		}

		/// <summary>Returns all pixels in the buffer as raw bytes.</summary>
		/// <returns>
		/// Raw pixel bytes. It is up to the caller to interpret the pixel format and account for potential  row and slice pitch 
		/// values.
		/// </returns>
		public char[] GetRawPixels()
		{
			return Internal_getRawPixels(mCachedPtr);
		}

		/// <summary>Sets all pixels in the buffer as raw bytes.</summary>
		/// <param name="value">
		/// Raw pixel bytes. It is up to the caller to interpret the pixel format and account for  potential row and slice pitch 
		/// values.
		/// </param>
		public void SetRawPixels(char[] value)
		{
			Internal_setRawPixels(mCachedPtr, value);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getRowPitch(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getSlicePitch(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PixelFormat Internal_getFormat(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getExtents(IntPtr thisPtr, out PixelVolume __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isConsecutive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getSize(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(PixelData managedInstance, ref PixelVolume volume, PixelFormat format);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(PixelData managedInstance, uint width, uint height, uint depth, PixelFormat pixelFormat);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPixel(IntPtr thisPtr, int x, int y, int z, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPixel(IntPtr thisPtr, ref Color value, int x, int y, int z);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Color[] Internal_getPixels(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPixels(IntPtr thisPtr, Color[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern char[] Internal_getRawPixels(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRawPixels(IntPtr thisPtr, char[] value);
	}
}
