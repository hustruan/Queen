#ifndef _PixelFormat__H
#define _PixelFormat__H

#include "Prerequisite.h"

enum PixelFormat
{
	/// <summary>
	/// Unknown pixel format.
	/// </summary>
	PF_Unknown = 0,

	/// <summary>
	/// 8-bit pixel format, all bits luminance.
	/// </summary>
	PF_Luminance8 = 1,

	/// <summary>
	/// 16-bit pixel format, all bits luminance.
	/// </summary>
	PF_Luminance16 = 2,
		
	/// <summary>
	/// 8-bit pixel format, all bits alpha.
	/// </summary>
	PF_Alpha8 = 3,

	/// <summary>
	/// 8-bit pixel format, 4 bits alpha, 4 bits luminance.
	/// </summary>
	PF_A4L4 = 4,
		
	/// <summary>
	/// 2 byte pixel format, 1 byte luminance, 1 byte alpha
	/// </summary>
	PF_A8L8 = 5,

	/// <summary>
	/// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
	/// </summary>
	PF_R5G6B5 = 6,
		
	/// <summary>
	/// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
	/// </summary>
	PF_B5G6R5 = 7,
		
	/// <summary>
	/// 16-bit pixel format, 4 bits for alpha, red, green and blue.
	/// </summary>
	PF_A4R4G4B4 = 8,
		
	/// <summary>
	/// 16-bit pixel format, 5 bits for blue, green, red and 1 for alpha.
	/// </summary>
	PF_A1R5G5B5 = 9,
		
	/// <summary>
	/// 24-bit pixel format, 8 bits for red, green and blue.
	/// </summary>
	PF_R8G8B8 = 10,
		
	/// <summary>
	/// 24-bit pixel format, 8 bits for blue, green and red.
	/// </summary>
	PF_B8G8R8 = 11,
		
	/// <summary>
	/// 32-bit pixel format, 8 bits for alpha, red, green and blue.
	/// </summary>
	PF_A8R8G8B8 = 12,
		
	/// <summary>
	/// 32-bit pixel format, 8 bits for blue, green, red and alpha.
	/// </summary>
	PF_A8B8G8R8 = 13,
		
	/// <summary>
	/// 32-bit pixel format, 8 bits for blue, green, red and alpha.
	/// </summary>
	PF_B8G8R8A8 = 14,
		
	/// <summary>
	/// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
	/// </summary>
	PF_A2R10G10B10 = 15,
		
	/// <summary>
	/// 32-bit pixel format, 10 bits for blue, green and red, 2 bits for alpha.
	/// </summary>
	PF_A2B10G10R10 = 16,
		
	/// <summary>
	/// DDS (DirectDraw Surface) DXT1 format
	/// </summary>
	PF_DXT1 = 17,
	
	/// <summary>
	/// DDS (DirectDraw Surface) DXT2 format
	/// </summary>
	PF_DXT2 = 18,

	/// <summary>
	/// DDS (DirectDraw Surface) DXT3 format
	/// </summary>
	PF_DXT3 = 19,

	/// <summary>
	/// DDS (DirectDraw Surface) DXT4 format
	/// </summary>
	PF_DXT4 = 20,

	/// <summary>
	/// DDS (DirectDraw Surface) DXT5 format
	/// </summary>
	PF_DXT5 = 21,
		
	/// <summary>
	// 48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue
	/// </summary>
	PF_B16G16R16F = 22,

	/// <summary>
	//64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits (float) for alpha
	/// </summary>
	PF_A16B16G16R16F = 23,
		
	/// <summary>
	// 96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue
	/// </summary>
	PF_B32G32R32F = 24,
		
	/// <summary>
	// 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha
	// </summary>
	PF_A32B32G32R32F = 25,
		
	/// <summary>
	/// 32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red
	/// like PF_A8R8G8B8, but alpha will get discarded
	// </summary>
	PF_X8R8G8B8 = 26,
		
	/// <summary>
	/// 32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red
	/// like PF_A8B8G8R8, but alpha will get discarded
	// </summary>
	PF_X8B8G8R8 = 27,  
		
	/// <summary>
	/// 32-bit pixel format, 8 bits for red, green, blue and alpha.
	// </summary>
	PF_R8G8B8A8 = 28,
		
	/// <summary>
	// 64-bit pixel format, 16 bits for red, green, blue and alpha
	/// </summary>
	PF_A16B16G16R16 = 29,
		
	/// <summary>
	/// 8-bit pixel format, 2 bits blue, 3 bits green, 3 bits red.
	/// </summary>
	PF_R3G3B2 = 30,
		
	/// <summary>
	// 16-bit pixel format, 16 bits (float) for red
	/// </summary>
	PF_R16F = 31,
		
	/// <summary>
	// 32-bit pixel format, 32 bits (float) for red
	/// </summary>
	PF_R32F = 32,
		
	/// <summary>
	// 32-bit pixel format, 16-bit green, 16-bit red
	/// </summary>
	PF_G16R16 = 33,
		
	/// <summary>
	// 32-bit, 2-channel s10e5 floating point pixel format, 16-bit green, 16-bit red
	/// </summary>
	PF_G16R16F = 34,

	/// <summary>
	// 64-bit, 2-channel floating point pixel format, 32-bit green, 32-bit red
	/// </summary>
	PF_G32R32F = 35,
		
	/// <summary>
	// 48-bit pixel format, 16 bits for red, green and blue
	/// </summary>
	PF_B16G16R16 = 36,

	/// <summary>
	// 48-bit pixel format, 16 bits for red, green and blue
	/// </summary>
	PF_Depth16 = 37,

	/// <summary>
	// 48-bit pixel format, 16 bits for red, green and blue
	/// </summary>
	PF_Depth24Stencil8 = 38,

	/// <summary>
	// 48-bit pixel format, 16 bits for red, green and blue
	/// </summary>
	PF_Depth32 = 39,

	/// <summary>
	// Number of pixel formats currently defined
	/// </summary>
	PF_Count = 40
};

/**
* Flags defining some on/off properties of pixel formats
*/
enum PixelFormatFlags {
	// This format has an alpha channel
	PFF_HasAlpha        = 0x00000001,      
	// This format is compressed. This invalidates the values in elemBytes,
	// elemBits and the bit counts as these might not be fixed in a compressed format.
	PFF_Compressed    = 0x00000002,
	// This is a floating point format
	PFF_Float           = 0x00000004,         
	// This is a depth format (for depth textures)
	PFF_Depth           = 0x00000008,
	// Format is in native endian. Generally true for the 16, 24 and 32 bits
	// formats which can be represented as machine integers.
	PFF_NativeEndian    = 0x00000010,
	// This is an intensity format instead of a RGB one. The luminance
	// replaces R,G and B. (but not A)
	PFF_Luminance       = 0x00000020
};

		
/** Pixel component format */
enum PixelComponentType
{
	PCT_Byte = 0,    /// Byte per component (8 bit fixed 0.0..1.0)
	PCT_Short = 1,   /// Short per component (16 bit fixed 0.0..1.0))
	PCT_Float16 = 2, /// 16 bit float per component
	PCT_Float32 = 3, /// 32 bit float per component
	PCT_Count = 4    /// Number of pixel types
};


class PixelFormatUtils
{
public:
		
	static std::string GetFormatName(PixelFormat srcformat);

	static uint32_t GetNumElemBytes(PixelFormat format);

	static uint32_t GetNumElemBits(PixelFormat format);

	static uint32_t GetFlags( PixelFormat format );

	static PixelComponentType GetComponentType(PixelFormat fmt);

	static uint32_t GetComponentCount(PixelFormat fmt);

	static uint32_t GetMemorySize(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format);

	/** Shortcut method to determine if the format has an alpha component */
	static bool HasAlpha(PixelFormat format);

	/** Shortcut method to determine if the format is floating point */
	static bool IsFloatingPoint(PixelFormat format);

	/** Shortcut method to determine if the format is compressed */
	static bool IsCompressed(PixelFormat format);

	/** Shortcut method to determine if the format is a depth format. */
	static bool IsDepthStencil(PixelFormat format);

	static bool IsStencil(PixelFormat format);

	/** Shortcut method to determine if the format is in native endian format. */
	static bool IsNativeEndian(PixelFormat format);

	/** Shortcut method to determine if the format is a luminance format. */
	static bool IsLuminance(PixelFormat format);

	static void GetNumDepthStencilBits(PixelFormat format, uint32_t& depth, uint32_t& stencil);

	static PixelFormat GetPixelFormat(const String& value);
		
};

#endif // PixelFormat_h__