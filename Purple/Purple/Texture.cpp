#include "Texture.h"
#include "DifferentialGeometry.h"
#include <nvImage.h>

namespace Purple {

std::map< std::string, TMipMap<ColorRGB>* > RGBImageTexture::msTextures;

RGBImageTexture::RGBImageTexture( const std::string& filename, TextureAddressMode warpU, TextureAddressMode warpV, MIPFilterType mipFilter /*= MFT_ENearest*/, float gamma /*= 0.0f*/, float maxAniso /*= 1.0f*/ )
{
	mMipMap = CreateOrReuseMipMap(filename);
	mSampleState.AddressU = warpU;
	mSampleState.AddressV = warpV;
	mSampleState.MaxAnisotropy = mMaxAnisotropy;
	mSampleState.Filter = mipFilter;

}

ColorRGB RGBImageTexture::Evaluate( const DifferentialGeometry& dg ) const
{
	return mMipMap->Eval(mSampleState, dg.UV, float2(dg.dudx, dg.dudy), float2(dg.dvdx, dg.dvdy));
}

void RGBImageTexture::ClearCache()
{
	for (auto iter = msTextures.begin(); iter != msTextures.end(); ++iter)
		delete iter->second;
	msTextures.erase(msTextures.begin(), msTextures.end());
}

	//TMipMap<ColorRGB>* RGBImageTexture::CreateOrReuseMipMap( const std::string& filename )
	//{
	//	if (msTextures.find(filename) != msTextures.end())
	//		return msTextures[filename];

	//	nv::Image image;
	//	assert(image.loadImageFromFile(filename.c_str()));

	//	int width = image.getWidth();
	//	int height = image.getHeight();
	//	int numMipmaps = image.getMipLevels();

	//	auto fmt = image.getFormat();

	//	if (fmt == GL_RGB || fmt == GL_BGR || fmt == GL_RGBA || fmt == GL_BGRA)
	//	{	
	//		std::cout << fmt << std::endl;

	//		int count = 0;
	//		int w = width;
	//		int h = height;
	//		for(int level = 0; level < numMipmaps; level++)
	//		{
	//			count += w * h;
	//			w = std::max(1, w / 2);
	//			h = std::max(1, h / 2);
	//		}

	//		ColorRGB* textureData = new ColorRGB [count];


	//		auto type = image.getType();
	//		/*if (type == GL_FLOAT)
	//		{
	//			switch (image.getInternalFormat())
	//			{
	//			case GL_RGB8:
	//				{
	//					w = width; h = height;
	//					for(int level = 0; level < numMipmaps; level++)
	//					{
	//						float* levelData = image.getLevel(level);
	//						for (int y = 0; y < height; ++y)
	//						{
	//							for (int x = 0; x < width; ++x)
	//							{
	//								textureData[level] 
	//							}
	//						}
	//						memcpy(textureData, , image.getImageSize(level));
	//					}
	//					break;	
	//				}
	//				break;
	//			case GL_RGB32F:
	//				{
	//					for(int level = 0; level < numMipmaps; level++)
	//						memcpy(textureData, image.getLevel(level), image.getImageSize(level));
	//					break;	
	//				}
	//				break;
	//			default:
	//				break;
	//			}
	//		}
	//		else */if (type == GL_UNSIGNED_BYTE)
	//		{
	//			switch (image.getInternalFormat())
	//			{
	//			case GL_RGB8:
	//				{
	//					std::cout << "GL_RGB8" << std::endl;
	//					int levelWidth = width; 
	//					int levelHeight = height;
	//					for(int level = 0; level < numMipmaps; level++)
	//					{
	//						uint8_t* levelData = (uint8_t*)image.getLevel(level);

	//						for (int y = 0; y < levelHeight; ++y)
	//						{
	//							for (int x = 0; x < levelWidth; ++x)
	//							{
	//								float r = (*levelData++) / float(255);
	//								float g = (*levelData++) / float(255);
	//								float b = (*levelData++) / float(255);
	//								textureData[y * levelWidth + x] = ColorRGB(r, g, b);
	//							}
	//						}
	//					}
	//				}
	//				break;
	//			case GL_RGBA8:
	//				{
	//					//std::cout << "GL_RGBA8" << std::endl;

	//					int levelWidth = width; 
	//					int levelHeight = height;

	//					for(int level = 0; level < numMipmaps; level++)
	//					{
	//				
	//						uint8_t* levelData = (uint8_t*)image.getLevel(level);
	//					
	//						for (int y = 0; y < levelHeight; ++y)
	//						{
	//							for (int x = 0; x < levelWidth; ++x)
	//							{
	//								float r = (*levelData++) / float(255);
	//								float g = (*levelData++) / float(255);
	//								float b = (*levelData++) / float(255);
	//								levelData++;
	//								textureData[y * levelWidth + x] = ColorRGB(r, g, b);
	//							}
	//						}
	//					}
	//				}
	//				break;
	//			case GL_RGB32F:
	//				{
	//					/*for(int level = 0; level < numMipmaps; level++)
	//						memcpy(textureData, image.getLevel(level), image.getImageSize(level));*/
	//					break;	
	//				}
	//				break;
	//			default:
	//				break;
	//			}

	//		
	//			TMipMap<ColorRGB>* mipmap = new TMipMap<ColorRGB>(width, height, numMipmaps, textureData);
	//			msTextures[filename] = mipmap;

	//			delete[] textureData;

	//			printf("loaded\n");

	//			return mipmap;
	//		}
	//		else
	//		{
	//			fprintf(stderr, "Error: Only support GL_FLOAT or GL_UNSIGNED_INT\n");
	//			assert(false);
	//			return NULL;
	//		}
	//	}
	//	else
	//	{
	//		fprintf(stderr, "Error: Only support RGB or BGR image format\n");
	//		assert(false);
	//		return NULL;
	//	}
	//}

TMipMap<ColorRGB>* RGBImageTexture::CreateOrReuseMipMap( const std::string& filename )
{
	if (msTextures.find(filename) != msTextures.end())
		return msTextures[filename];

	FILE* pFile;
	if( fopen_s(&pFile, filename.c_str(), "rb") )
		return NULL;

	int width, height, levels, count;
	fread(&width, sizeof(int), 1, pFile);
	fread(&height, sizeof(int),1, pFile);
	fread(&levels, sizeof(int), 1, pFile);
	fread(&count, sizeof(int), 1, pFile);

	count = 0;
	int w = width;
	int h = height;
	for(int level = 0; level < levels; level++)
	{
		count += w * h;
		w = std::max(1, w / 2);
		h = std::max(1, h / 2);
	}

	std::vector<ColorRGB> textureData(count);

	fread(&textureData[0], sizeof(ColorRGB) * count, 1, pFile);
	fclose(pFile);

	TMipMap<ColorRGB>* mipmap = new TMipMap<ColorRGB>(width, height, levels, &textureData[0]);
	msTextures[filename] = mipmap;

	std::cout << width << " " << height << std::endl;

	return mipmap;
}
}