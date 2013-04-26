#include <nvImage.h>
#include <string>
#include <iostream>
#include <ColorRGBA.hpp>

using RxLib::ColorRGB;

int main(int argc, char** argv)
{
	std::string filename = "../../Media/wall.dds";

	nv::Image image;
	bool b = image.loadImageFromFile(filename.c_str());
	std::cout << b << std::endl;

	int width = image.getWidth();
	int height = image.getHeight();
	int numMipmaps = image.getMipLevels();

	auto fmt = image.getFormat();

	std::cout << width << height << numMipmaps; 

	if (fmt == GL_RGB || fmt == GL_BGR || fmt == GL_RGBA || fmt == GL_BGRA)
	{	
		std::cout << fmt << std::endl;

		int count = 0;
		int w = width;
		int h = height;
		for(int level = 0; level < numMipmaps; level++)
		{
			count += w * h;
			w = std::max(1, w / 2);
			h = std::max(1, h / 2);
		}

		std::vector<ColorRGB> textureData(count);

		auto type = image.getType();
		/*if (type == GL_FLOAT)
		{
			switch (image.getInternalFormat())
			{
			case GL_RGB8:
				{
					w = width; h = height;
					for(int level = 0; level < numMipmaps; level++)
					{
						float* levelData = image.getLevel(level);
						for (int y = 0; y < height; ++y)
						{
							for (int x = 0; x < width; ++x)
							{
								textureData[level] 
							}
						}
						memcpy(textureData, , image.getImageSize(level));
					}
					break;	
				}
				break;
			case GL_RGB32F:
				{
					for(int level = 0; level < numMipmaps; level++)
						memcpy(textureData, image.getLevel(level), image.getImageSize(level));
					break;	
				}
				break;
			default:
				break;
			}
		}
		else */if (type == GL_UNSIGNED_BYTE)
		{
			switch (image.getInternalFormat())
			{
			case GL_RGB8:
				{
					std::cout << "GL_RGB8" << std::endl;
					int levelWidth = width; 
					int levelHeight = height;
					for(int level = 0; level < numMipmaps; level++)
					{
						uint8_t* levelData = (uint8_t*)image.getLevel(level);

						for (int y = 0; y < levelHeight; ++y)
						{
							for (int x = 0; x < levelWidth; ++x)
							{
								float r = (*levelData++) / float(255);
								float g = (*levelData++) / float(255);
								float b = (*levelData++) / float(255);
								textureData[y * levelWidth + x] = ColorRGB(r, g, b);
							}
						}
					}
				}
				break;
			case GL_RGBA8:
				{
					//std::cout << "GL_RGBA8" << std::endl;

					int levelWidth = width; 
					int levelHeight = height;

					for(int level = 0; level < numMipmaps; level++)
					{
					
						uint8_t* levelData = (uint8_t*)image.getLevel(level);
						
						for (int y = 0; y < levelHeight; ++y)
						{
							for (int x = 0; x < levelWidth; ++x)
							{
								float b = (*levelData++) / float(255);
								float g = (*levelData++) / float(255);
								float r = (*levelData++) / float(255);
								levelData++;
								textureData[y * levelWidth + x] = ColorRGB(r, g, b);
							}
						}
					}
				}
				break;
			case GL_RGB32F:
				{
					/*for(int level = 0; level < numMipmaps; level++)
						memcpy(textureData, image.getLevel(level), image.getImageSize(level));*/
					break;	
				}
				break;
			default:
				break;
			}


			filename.replace(filename.begin() + filename.find_last_of('.'), filename.end(), ".img");
			FILE* pFile = fopen(filename.c_str(), "wb");

			fwrite(&width, sizeof(int), 1, pFile);

			// write num vertices
			fwrite(&height, sizeof(int), 1, pFile);

			// has tanget 
			fwrite(&numMipmaps, sizeof(int), 1, pFile);

			fwrite(&count, sizeof(int), 1, pFile);

			fwrite(&textureData[0], sizeof(ColorRGB) * count, 1, pFile);

			fclose(pFile);

		}
		else
		{
			fprintf(stderr, "Error: Only support GL_FLOAT or GL_UNSIGNED_INT\n");
			assert(false);
			return NULL;
		}
	}
	else
	{
		fprintf(stderr, "Error: Only support RGB or BGR image format\n");
		assert(false);
		return NULL;
	}

	return 0;
}


