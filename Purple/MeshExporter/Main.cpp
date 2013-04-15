#include <iostream>
#include <fstream>
#include <nvModel.h>
#include <vector>
#include <string>
#include <Vector.hpp>

using namespace std;
using namespace RxLib;

int main(int argc, char** argv)
{
	string filename = "../../Media/sphere.obj";
	
	nv::Model model;
	if( !model.loadModelFromFile(filename.c_str()) )
	{
		printf("File: %s not found...\n", filename.c_str());
		exit(0);
	}

	if(!model.hasNormals())
		model.computeNormals();

	model.compileModel( nv::Model::eptTriangles);
	//model.rescale(1.0f);

	int numIndices = model.getCompiledIndexCount();
	int numVertices = model.getCompiledVertexCount();
	int vertexSize = model.getCompiledVertexSize();

	int hasTangent = model.hasTangents();

	std::vector<float> positions(numVertices*3);
	std::vector<float> normals(numVertices*3);
	std::vector<float> texcoords(numVertices*2);

	const float* pVertices = model.getCompiledVertices();
	
	int offsetPos = model.getCompiledPositionOffset();
	int offsetNormal = model.getCompiledNormalOffset();
	int offsetTex = model.getCompiledTexCoordOffset();
	
	float* pTempPos = &positions[0];
	float* pTempNormal = &normals[0];
	float* pTempTex = &texcoords[0];

	for (int i = 0; i < numVertices; ++i)
	{
		*pTempPos++ = pVertices[offsetPos+0];  
		*pTempPos++ = pVertices[offsetPos+1];
		*pTempPos++ = pVertices[offsetPos+2]; 

		*pTempNormal++ = pVertices[offsetNormal+0];  
		*pTempNormal++ = pVertices[offsetNormal+1];
		*pTempNormal++ = pVertices[offsetNormal+2]; 

		*pTempTex++ = pVertices[offsetTex+0];  
		*pTempTex++ = pVertices[offsetTex+1];

		pVertices += vertexSize;
	}


	// output to binary
	
	filename.replace(filename.begin() + filename.find_last_of('.'), filename.end(), ".md");

	FILE* pFile;
	fopen_s(&pFile, filename.c_str(), "wb");

	// write num indices
	fwrite(&numIndices, sizeof(int), 1, pFile);

	// write num vertices
	fwrite(&numVertices, sizeof(int), 1, pFile);

	// has tanget 
	fwrite(&hasTangent, sizeof(int), 1, pFile);

	// write indices
	fwrite(model.getCompiledIndices(), sizeof(unsigned int) * numIndices, 1, pFile);
	
	// write position
	fwrite(&positions[0], sizeof(float) * positions.size(), 1, pFile);

	// write normal
	fwrite(&normals[0], sizeof(float) * normals.size(), 1, pFile);

	// write tex
	fwrite(&texcoords[0], sizeof(float) * texcoords.size(), 1, pFile);

	if (hasTangent)
	{
		int offsetTanget = model.getCompiledTangentOffset();

		std::vector<float> tangents(numVertices*3);
		float* pTempTanget = &tangents[0];
		for (int i = 0; i < numVertices; ++i)
		{
			*pTempTanget++ = pVertices[offsetTanget+0];  
			*pTempTanget++ = pVertices[offsetTanget+1];
			*pTempTanget++ = pVertices[offsetTanget+2]; 

			pVertices += vertexSize;
		}

		fwrite(&tangents[0], sizeof(float) * tangents.size(), 1, pFile);
	}

	fclose(pFile);

	fclose(pFile);
	return 0;

}