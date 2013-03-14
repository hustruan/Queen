#include "RenderFactory.h"
#include "GraphicsBuffer.h"
#include "PixelFormat.h"
#include "Texture.h"
#include <nvImage.h>
//#include <GL/GL.h>

namespace {

	PixelFormat UnMapping( GLint internalformat, GLenum format, GLenum type )
	{
		switch(internalformat)
		{
		case GL_LUMINANCE8:
			if(type == GL_UNSIGNED_BYTE)
				return PF_Luminance8;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE16:
			if(type == GL_UNSIGNED_SHORT)
				return PF_Luminance16;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_ALPHA8:
			if(type == GL_UNSIGNED_BYTE)
				return PF_Alpha8;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE4_ALPHA4:
			assert(format == GL_LUMINANCE_ALPHA);
			if(type == GL_UNSIGNED_BYTE)
				return PF_A4L4;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE8_ALPHA8:
			assert(format == GL_LUMINANCE_ALPHA);
			if(type == GL_UNSIGNED_SHORT)
				return PF_A8L8;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGB5:
			assert(type == GL_UNSIGNED_SHORT_5_6_5);
			if(format == GL_RGB)
				return PF_R5G6B5;
			else if(format == GL_BGR)
				return PF_B5G6R5;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGBA4:
			// assert(type == GL_UNSIGNED_SHORT_4_4_4_4_REV);
			if(format == GL_BGRA)
				return PF_A4R4G4B4;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGB5_A1:
			// assert(type == GL_UNSIGNED_SHORT_1_5_5_5_REV);

			if(format == GL_BGRA)
				return PF_A1R5G5B5;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGB8: 
			assert(type == GL_UNSIGNED_BYTE);

			switch(format)
			{
			case GL_BGR:
				return PF_R8G8B8;
			case GL_RGB:
				return PF_B8G8R8;
			case GL_BGRA:
				return PF_X8R8G8B8;
			case GL_RGBA:
				return PF_X8B8G8R8;
			default:
				// not supported
				assert(false);
				return PF_Unknown;	
			}

		case GL_RGBA8:
			assert(type == GL_UNSIGNED_BYTE);
			switch(format)
			{
			case GL_BGR:
				return PF_X8R8G8B8;
			case GL_RGB:
				return PF_X8B8G8R8;
			case GL_BGRA:
				return PF_A8R8G8B8;
			case GL_RGBA:
				return PF_A8B8G8R8;
			default:
				// not supported
				assert(false);
				return PF_Unknown;	
			}

			//if(format == GL_BGRA)
			//	return PF_A8R8G8B8;
			//else if(format == GL_RGBA)
			//	return PF_A8B8G8R8;
			//else
			//{
			//	// not supported
			//	assert(false);
			//	return PF_Unknown;
			//}

		case PF_A2R10G10B10:
			//assert(type == GL_UNSIGNED_INT_2_10_10_10_REV);
			if(format == GL_BGRA)
				return PF_A2R10G10B10;
			else if(format == GL_RGBA)
				return PF_A2B10G10R10;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGB16F:
			assert(type == GL_HALF_FLOAT);
			if(format == GL_RGB)
				return PF_B16G16R16F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}


		case GL_RGBA16F:
			assert(type == GL_HALF_FLOAT);
			if(format == GL_RGBA)
				return PF_A16B16G16R16F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGB32F:
			assert(type == GL_FLOAT);
			if(format == GL_RGB)
				return PF_B32G32R32F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGBA32F:
			assert(type == GL_FLOAT);
			if(format == GL_RGBA)
				return PF_A32B32G32R32F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

			//////////////////////////////////////////////////////////////////////////

			//case PF_R8G8B8A8:
			//	outInternalformat = GL_RGBA8;
			//	outFormat = GL_RGBA;
			//	outType = GL_UNSIGNED_BYTE;
			//	break;

			//case PF_B8G8R8A8:
			//	outInternalformat = GL_RGBA8;
			//	outFormat = GL_BGRA;
			//	outType = GL_UNSIGNED_BYTE;
			//	break;

			//////////////////////////////////////////////////////////////////////////

		case GL_R3_G3_B2:
			assert(type == GL_UNSIGNED_BYTE_3_3_2);
			if(format == GL_RGB)
				return PF_R3G3B2;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE16F_ARB:
			assert(type == GL_HALF_FLOAT);
			if(format == GL_LUMINANCE)
				return PF_R16F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE32F_ARB:
			assert(type == GL_FLOAT);
			if(format == GL_LUMINANCE)
				return PF_R32F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE16_ALPHA16:
			assert(type == GL_UNSIGNED_SHORT);
			if(format == GL_LUMINANCE_ALPHA)
				return PF_G16R16;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE_ALPHA16F_ARB:
			assert(type == GL_HALF_FLOAT);
			if(format == GL_LUMINANCE_ALPHA)
				return PF_G16R16F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_LUMINANCE_ALPHA32F_ARB:
			assert(type == GL_FLOAT);
			if(format == GL_LUMINANCE_ALPHA)
				return PF_G32R32F;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_RGB16:
			assert(type == GL_UNSIGNED_SHORT);
			if(format == GL_RGB)
				return PF_B16G16R16;
			else
			{
				// not supported
				assert(false);
				return PF_Unknown;
			}

		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
			return PF_DXT1;

		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
			return PF_DXT3;

		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
			return PF_DXT5;

		default:
			// not supported
			assert(false);
			return PF_Unknown;
		}
	}

}

RenderFactory::RenderFactory(void)
{
}


RenderFactory::~RenderFactory(void)
{
}

shared_ptr<GraphicsBuffer> RenderFactory::CreateVertexBuffer( ElementInitData* initData )
{
	assert(initData && initData->pData && initData->RowPitch > 0);
	
	shared_ptr<GraphicsBuffer> retVal =  std::make_shared<GraphicsBuffer>(initData->RowPitch);

	void* buffer = retVal->Map(0, initData->RowPitch, BA_Write_Only);
	memcpy(buffer, initData->pData, initData->RowPitch);
	retVal->UnMap();

	return retVal;
}

shared_ptr<GraphicsBuffer> RenderFactory::CreateIndexBuffer( ElementInitData* initData )
{
	ASSERT(initData && initData->pData && initData->RowPitch > 0);
	shared_ptr<GraphicsBuffer> retVal =  std::make_shared<GraphicsBuffer>(initData->RowPitch);

	void* buffer = retVal->Map(0, initData->RowPitch, BA_Write_Only);
	memcpy(buffer, initData->pData, initData->RowPitch);
	retVal->UnMap();

	return retVal;
}

shared_ptr<VertexDeclaration> RenderFactory::CreateVertexDeclaration( const std::vector<VertexElement>& elems )
{
	return std::make_shared<VertexDeclaration>(elems);
}

shared_ptr<VertexDeclaration> RenderFactory::CreateVertexDeclaration( VertexElement* elems, uint32_t count )
{
	return std::make_shared<VertexDeclaration>(elems, count);
}

shared_ptr<Texture> RenderFactory::CreateTextureFromFile( const std::string& texFileName )
{
	TextureType type;

	nv::Image image;

	if (!image.loadImageFromFile(texFileName.c_str()))
	{
		ASSERT(false);
	}

	int32_t numMipmaps = image.getMipLevels();
	int32_t imageWidth = image.getWidth();
	int32_t imageHeight = image.getHeight();
	int32_t imageDepth = image.getDepth();
	int32_t numCubeFaces = image.getFaces();

	bool isCompressed = image.isCompressed();
	bool isCubeMap = image.isCubeMap();
	bool isVolume = image.isVolume();


	if( isCubeMap )
	{
		type = TT_TextureCube;
	}
	else if (isVolume && imageDepth>0)
	{
		type = TT_Texture3D;
	}
	else if(imageHeight>0 && imageWidth>0)
	{
		type = TT_Texture2D;
	}else if(imageWidth>0 && imageHeight==0)
	{
		type = TT_Texture1D;
	}else
	{
		assert(false);
	}

	ASSERT(type == TT_Texture2D);


	PixelFormat format;
	format = UnMapping(image.getInternalFormat(), image.getFormat(), image.getType());

	uint32_t elementSize;
	elementSize = isCubeMap ? (6*numMipmaps) : numMipmaps;
	std::vector<ElementInitData> imageData(elementSize);

	if(!isCubeMap)
	{
		for (int32_t level = 0; level < numMipmaps; ++level)
		{
			imageData[level].pData = image.getLevel(level);
			imageData[level].RowPitch = image.getImageSize(level);
			imageData[level].SlicePitch = 0;
		}
	}
	else
	{
		// A texture cube is a 2D texture array that contains 6 textures, one for each face of the cube
		for(uint32_t face = 0; face < 6; ++face)
		{
			for (int32_t level = 0; level < numMipmaps; ++level)
			{
				imageData[face*numMipmaps+level].pData = image.getLevel(level, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face);
				imageData[face*numMipmaps+level].RowPitch = image.getImageSize(level);
				imageData[face*numMipmaps+level].SlicePitch = 0;
			}
		}
	}

	return std::make_shared<Texture2D>(format, imageWidth, imageHeight, numMipmaps,  1, 0, 0, &imageData[0]);


	/*switch(type)
	{
	case TT_Texture1D:
	return CreateTexture1D(imageWidth, format, 1, numMipmaps, 1, 0, accessHint, &imageData[0]);
	case TT_Texture2D:
	return CreateTexture2D(imageWidth, imageHeight, format, 1, numMipmaps, 1, 0, accessHint, &imageData[0]);
	case TT_Texture3D:
	return CreateTexture3D(imageWidth, imageHeight, imageDepth, format, 1, numMipmaps, 1, 0, accessHint, &imageData[0]);
	case TT_TextureCube:
	return CreateTextureCube(imageWidth, imageHeight, format, 1, numMipmaps, 1, 0, accessHint, &imageData[0]);
	};*/

	//ENGINE_EXCEPT(Exception::ERR_RT_ASSERTION_FAILED, "Unsupported Texture Format", "OpenGLRenderFactory::CreateTextureFromFile");
}


void ExportToPfm(const std::string& filename, uint32_t width, uint32_t height, PixelFormat format, void* data)
{
	vector<float> temp;
	temp.resize(width * height * 3);
	float* temPixel = &temp[0];

	uint8_t* pixel = (uint8_t*)data;

	if (format == PF_X8R8G8B8)
	{
		for (uint32_t j = 0; j < height; j++)
			for(uint32_t i = 0; i < width; i ++)
			{	
				uint8_t r = pixel[(j * width + i)*4 + 0];
				uint8_t g = pixel[(j * width + i)*4 +1];
				uint8_t b = pixel[(j * width + i)*4 +2];
				uint8_t a = pixel[(j * width + i)*4 +3];

				temPixel[0] = r / 255.0f;
				temPixel[1] = g / 255.0f;
				temPixel[2] = b / 255.0f;
				temPixel += 3;
			}

			FILE* file = fopen(filename.c_str(), "wb");
			assert(file != NULL);
			fprintf(file, "PF\n%d %d\n-1.0\n", width, height);
			fwrite(&temp[0],sizeof(float) * temp.size(), 1,file);
			fclose(file);
	}
}