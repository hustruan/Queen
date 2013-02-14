#include "PixelFormat.h"
#include <exception>

//-----------------------------------------------------------------------
/**
* A record that describes a pixel format in detail.
*/
struct PixelFormatDescription {
	/* Name of the format, as in the enum */
	const char *name;
	/* Number of bytes one element (colour value) takes. */
	unsigned char elemBytes;
	/* Pixel format flags, see enum PixelFormatFlags for the bit field
	* definitions
	*/
	uint32_t flags;
	/** Component type
	*/
	PixelComponentType componentType;
	/** Component count
	*/
	unsigned char componentCount;
	/* Number of bits for red(or luminance), green, blue, alpha
	*/
	unsigned char rbits,gbits,bbits,abits; /*, ibits, dbits, ... */

	/* Masks and shifts as used by packers/unpackers */
	uint32_t rmask, gmask, bmask, amask;
	unsigned char rshift, gshift, bshift, ashift;
};

//-----------------------------------------------------------------------
/** Pixel format database */
PixelFormatDescription _pixelFormats[PF_Count] = {
	//-----------------------------------------------------------------------
	{"PF_Unknown",
	/* Bytes per element */
	0,
	/* Flags */
	0,
	/* Component type and count */
	PCT_Byte, 0,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_Luminance8",
	/* Bytes per element */
	1,
	/* Flags */
	PFF_Luminance | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 1,
	/* rbits, gbits, bbits, abits */
	8, 0, 0, 0,
	/* Masks and shifts */
	0xFF, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_Luminance16",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_Luminance | PFF_NativeEndian,
	/* Component type and count */
	PCT_Short, 1,
	/* rbits, gbits, bbits, abits */
	16, 0, 0, 0,
	/* Masks and shifts */
	0xFFFF, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_Alpha8",
	/* Bytes per element */
	1,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 1,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 8,
	/* Masks and shifts */
	0, 0, 0, 0xFF, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A4L4",
	/* Bytes per element */
	1,
	/* Flags */
	PFF_HasAlpha | PFF_Luminance | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 2,
	/* rbits, gbits, bbits, abits */
	4, 0, 0, 4,
	/* Masks and shifts */
	0x0F, 0, 0, 0xF0, 0, 0, 0, 4
	},
	//-----------------------------------------------------------------------
	{"PF_A8L8",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_HasAlpha | PFF_Luminance| PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 2,
	/* rbits, gbits, bbits, abits */
	8, 0, 0, 8,
	/* Masks and shifts */
	0x00FF, 0, 0, 0xFF00,
	0, 0, 0, 8
	},
	//-----------------------------------------------------------------------
	{"PF_R5G6B5",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	5, 6, 5, 0,
	/* Masks and shifts */
	0xF800, 0x07E0, 0x001F, 0,
	11, 5, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_B5G6R5",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	5, 6, 5, 0,
	/* Masks and shifts */
	0x001F, 0x07E0, 0xF800, 0,
	0, 5, 11, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A4R4G4B4",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	4, 4, 4, 4,
	/* Masks and shifts */
	0x0F00, 0x00F0, 0x000F, 0xF000,
	8, 4, 0, 12
	},
	//-----------------------------------------------------------------------
	{"PF_A1R5G5B5",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	5, 5, 5, 1,
	/* Masks and shifts */
	0x7C00, 0x03E0, 0x001F, 0x8000,
	10, 5, 0, 15,
	},
	//-----------------------------------------------------------------------
	{"PF_R8G8B8",
	/* Bytes per element */
	3,  // 24 bit integer -- special
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 0,
	/* Masks and shifts */
	0xFF0000, 0x00FF00, 0x0000FF, 0,
	16, 8, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_B8G8R8",
	/* Bytes per element */
	3,  // 24 bit integer -- special
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 0,
	/* Masks and shifts */
	0x0000FF, 0x00FF00, 0xFF0000, 0,
	0, 8, 16, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A8R8G8B8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 8,
	/* Masks and shifts */
	0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
	16, 8, 0, 24
	},
	//-----------------------------------------------------------------------
	{"PF_A8B8G8R8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 8,
	/* Masks and shifts */
	0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
	0, 8, 16, 24,
	},
	//-----------------------------------------------------------------------
	{"PF_B8G8R8A8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 8,
	/* Masks and shifts */
	0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF,
	8, 16, 24, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A2R10G10B10",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	10, 10, 10, 2,
	/* Masks and shifts */
	0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000,
	20, 10, 0, 30
	},
	//-----------------------------------------------------------------------
	{"PF_A2B10G10R10",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	10, 10, 10, 2,
	/* Masks and shifts */
	0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000,
	0, 10, 20, 30
	},
	//-----------------------------------------------------------------------
	{"PF_DXT1",
	/* Bytes per element */
	0,
	/* Flags */
	PFF_Compressed | PFF_HasAlpha,
	/* Component type and count */
	PCT_Byte, 3, // No alpha
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_DXT2",
	/* Bytes per element */
	0,
	/* Flags */
	PFF_Compressed | PFF_HasAlpha,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_DXT3",
	/* Bytes per element */
	0,
	/* Flags */
	PFF_Compressed | PFF_HasAlpha,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_DXT4",
	/* Bytes per element */
	0,
	/* Flags */
	PFF_Compressed | PFF_HasAlpha,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_DXT5",
	/* Bytes per element */
	0,
	/* Flags */
	PFF_Compressed | PFF_HasAlpha,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_B16G16R16F",
	/* Bytes per element */
	6,
	/* Flags */
	PFF_Float,
	/* Component type and count */
	PCT_Float16, 3,
	/* rbits, gbits, bbits, abits */
	16, 16, 16, 0,
	/* Masks and shifts */
	0, 0, 0, 0,
	0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A16B16G16R16F",
	/* Bytes per element */
	8,
	/* Flags */
	PFF_Float | PFF_HasAlpha,
	/* Component type and count */
	PCT_Float16, 4,
	/* rbits, gbits, bbits, abits */
	16, 16, 16, 16,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_B32G32R32F",
	/* Bytes per element */
	12,
	/* Flags */
	PFF_Float,
	/* Component type and count */
	PCT_Float32, 3,
	/* rbits, gbits, bbits, abits */
	32, 32, 32, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A32B32G32R32F",
	/* Bytes per element */
	16,
	/* Flags */
	PFF_Float | PFF_HasAlpha,
	/* Component type and count */
	PCT_Float32, 4,
	/* rbits, gbits, bbits, abits */
	32, 32, 32, 32,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_X8R8G8B8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 0,
	/* Masks and shifts */
	0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
	16, 8, 0, 24
	},
	//-----------------------------------------------------------------------
	{"PF_X8B8G8R8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 0,
	/* Masks and shifts */
	0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
	0, 8, 16, 24
	},
	//-----------------------------------------------------------------------
	{"PF_R8G8B8A8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_HasAlpha | PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 4,
	/* rbits, gbits, bbits, abits */
	8, 8, 8, 8,
	/* Masks and shifts */
	0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF,
	24, 16, 8, 0
	},
	//-----------------------------------------------------------------------
	{"PF_A16B16G16R16",
	/* Bytes per element */
	8,
	/* Flags */
	PFF_HasAlpha,
	/* Component type and count */
	PCT_Short, 4,
	/* rbits, gbits, bbits, abits */
	16, 16, 16, 16,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_R3G3B2",
	/* Bytes per element */
	1,
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Byte, 3,
	/* rbits, gbits, bbits, abits */
	3, 3, 2, 0,
	/* Masks and shifts */
	0xE0, 0x1C, 0x03, 0,
	5, 2, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_R16F",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_Float,
	/* Component type and count */
	PCT_Float16, 1,
	/* rbits, gbits, bbits, abits */
	16, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_R32F",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_Float,
	/* Component type and count */
	PCT_Float32, 1,
	/* rbits, gbits, bbits, abits */
	32, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_G16R16",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_NativeEndian,
	/* Component type and count */
	PCT_Short, 2,
	/* rbits, gbits, bbits, abits */
	16, 16, 0, 0,
	/* Masks and shifts */
	0x0000FFFF, 0xFFFF0000, 0, 0, 
	0, 16, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_G16R16F",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_Float,
	/* Component type and count */
	PCT_Float16, 2,
	/* rbits, gbits, bbits, abits */
	16, 16, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_G32R32F",
	/* Bytes per element */
	8,
	/* Flags */
	PFF_Float,
	/* Component type and count */
	PCT_Float32, 2,
	/* rbits, gbits, bbits, abits */
	32, 32, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_B16G16R16",
	/* Bytes per element */
	6,
	/* Flags */
	0,
	/* Component type and count */
	PCT_Short, 3,
	/* rbits, gbits, bbits, abits */
	16, 16, 16, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_Depth16",
	/* Bytes per element */
	2,
	/* Flags */
	PFF_Depth,
	/* Component type and count */
	PCT_Float16, 1,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_Depth24Stencil8",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_Depth,
	/* Component type and count */
	PCT_Float32, 1,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
	//-----------------------------------------------------------------------
	{"PF_Depth32",
	/* Bytes per element */
	4,
	/* Flags */
	PFF_Depth,
	/* Component type and count */
	PCT_Float32, 1,
	/* rbits, gbits, bbits, abits */
	0, 0, 0, 0,
	/* Masks and shifts */
	0, 0, 0, 0, 0, 0, 0, 0
	},
};

static inline const PixelFormatDescription &GetDescriptionFor(const PixelFormat fmt)
{
	const int32_t ord = (int32_t)fmt;
	assert(ord>=0 && ord<PF_Count);

	return _pixelFormats[ord];
}

uint32_t PixelFormatUtils::GetNumElemBytes( PixelFormat format )
{
	return GetDescriptionFor(format).elemBytes;
}


uint32_t PixelFormatUtils::GetNumElemBits( PixelFormat format )
{
	return GetDescriptionFor(format).elemBytes * 8;
}

uint32_t PixelFormatUtils::GetMemorySize( uint32_t width, uint32_t height, uint32_t depth, PixelFormat format )
{
	if(IsCompressed(format))
	{
		switch(format)
		{
			// DXT formats work by dividing the image into 4x4 blocks, then encoding each
			// 4x4 block with a certain number of bytes. 
		case PF_DXT1:
			return ((width+3)/4)*((height+3)/4)*8 * depth;
		case PF_DXT2:
		case PF_DXT3:
		case PF_DXT4:
		case PF_DXT5:
			return ((width+3)/4)*((height+3)/4)*16 * depth;

		default:
			throw std::exception("Invalid compressed pixel format") ;
		}
	}
	else
	{
		return width*height*depth*GetNumElemBytes(format);
	}
}

uint32_t PixelFormatUtils::GetFlags( PixelFormat format )
{	
	return GetDescriptionFor(format).flags;
}

bool PixelFormatUtils::HasAlpha( PixelFormat format )
{
	return ( GetFlags(format) & PFF_HasAlpha ) > 0;
}

bool PixelFormatUtils::IsFloatingPoint( PixelFormat format )
{
	return ( GetFlags(format) & PFF_Float) > 0;
}

bool PixelFormatUtils::IsCompressed( PixelFormat format )
{
	return ( GetFlags(format) & PFF_Compressed) > 0;
}

bool PixelFormatUtils::IsDepthStencil( PixelFormat format )
{
	return ( GetFlags(format) & PFF_Depth) > 0;
}

bool PixelFormatUtils::IsNativeEndian( PixelFormat format )
{
	return ( GetFlags(format) & PFF_NativeEndian) > 0;
}

bool PixelFormatUtils::IsLuminance( PixelFormat format )
{
	return ( GetFlags(format) & PFF_Luminance) > 0;
}

std::string PixelFormatUtils::GetFormatName( PixelFormat srcformat )
{
	return GetDescriptionFor(srcformat).name;
}

PixelComponentType PixelFormatUtils::GetComponentType( PixelFormat fmt )
{
	const PixelFormatDescription &des =  GetDescriptionFor(fmt);
	return des.componentType;
}

uint32_t PixelFormatUtils::GetComponentCount( PixelFormat fmt )
{
	const PixelFormatDescription &des = GetDescriptionFor(fmt);
	return des.componentCount;
}


void PixelFormatUtils::GetNumDepthStencilBits( PixelFormat format, uint32_t& depth, uint32_t& stencil )
{
	switch(format)
	{
	case PF_Depth16:
		depth = 16;
		stencil = 0;
		break;
	case PF_Depth24Stencil8:
		depth = 24;
		stencil = 8;
		break;
	case PF_Depth32:
		depth = 32;
		stencil = 0;
		break;

	default:
		depth = 0;
		stencil = 0;
	}
}

bool PixelFormatUtils::IsStencil( PixelFormat format )
{
	if( format == PF_Depth24Stencil8 )
		return true;
	return false;
}

PixelFormat PixelFormatUtils::GetPixelFormat( const String& value )
{
	String full = "PF_" + value;

	for (size_t i = 0; i < PF_Count; ++i)
	{
		if (String(_pixelFormats[i].name) == full)
		{
			return PixelFormat(PF_Unknown + i);
		}
	}
	
	throw std::exception("Unsupported pixel format") ;
}