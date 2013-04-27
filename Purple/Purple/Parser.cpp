#include "Parser.h"
#include <fstream>
#include <sstream>
#include <iterator>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include "Scene.h"
#include "Integrator.h"
#include "Sampler.h"
#include "Film.h"
#include "Camera.h"
#include "Filter.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "SimpleShape.h"
#include <Math.hpp>
#include <MathUtil.hpp>


namespace Purple {

using namespace RxLib;

// Convert string to other number type
template<typename Target, typename Source>
inline typename Target LexicalCast( const Source& value )
{
	Target re;
	std::stringstream str;
	str << value;
	str >> re;
	return re;
}

inline std::string AttributeString(rapidxml::xml_node<>* node, const char* attriName, const std::string& defaultValue)
{
	rapidxml::xml_attribute<>* attrib = node->first_attribute(attriName);
	
	if (attrib)
		return std::string(attrib->value(), attrib->value_size());

	return defaultValue;
}

inline float AttributeFloat(rapidxml::xml_node<>* node, const char* attriName, float defaultValue)
{
	rapidxml::xml_attribute<>* attrib = node->first_attribute(attriName);

	if (attrib)
		return LexicalCast<float>(std::string(attrib->value(), attrib->value_size()));

	return defaultValue;
}

inline int32_t AttributeInt(rapidxml::xml_node<>* node, const char* attriName, int32_t defaultValue)
{
	rapidxml::xml_attribute<>* attrib = node->first_attribute(attriName);

	if (attrib)
		return LexicalCast<int>(std::string(attrib->value(), attrib->value_size()));

	return defaultValue;
}

inline uint32_t AttributeUInt(rapidxml::xml_node<>* node, const char* attriName, uint32_t defaultValue)
{
	rapidxml::xml_attribute<>* attrib = node->first_attribute(attriName);

	if (attrib)
		return LexicalCast<int>(std::string(attrib->value(), attrib->value_size()));

	return defaultValue;
}

float44 CreateTransform(rapidxml::xml_node<>* node)
{
	rapidxml::xml_node<>* transNode = node->first_node("tranlation");
	rapidxml::xml_node<>* scaleNode = node->first_node("scale");
	rapidxml::xml_node<>* rotNode = node->first_node("rotation");

	float3 translation(0, 0, 0);
	float3 scale(1, 1, 1);
	Quaternionf rot = Quaternionf::Identity();

	if (transNode)
	{
		translation = float3(AttributeFloat(transNode, "x", 0.0f), AttributeFloat(transNode, "y", 0.0f), AttributeFloat(transNode, "z", 0.0f));
	}

	if (scaleNode)
	{
		scale = float3(AttributeFloat(scaleNode, "x", 0.0f), AttributeFloat(scaleNode, "y", 0.0f), AttributeFloat(scaleNode, "z", 0.0f));
	}

	if (rotNode)
	{
		rot = RxLib::Quaternionf(AttributeFloat(rotNode, "w", 0.0f), AttributeFloat(rotNode, "x", 0.0f), AttributeFloat(rotNode, "y", 0.0f), AttributeFloat(rotNode, "z", 0.0f));
	}


	return RxLib::CreateTransformMatrix(scale, rot, translation);
}

SurfaceIntegrator* CreateSurfaceIntegrator(rapidxml::xml_node<>* node)
{
	std::string name = AttributeString(node, "type", "whitted");
	int maxDepth = AttributeInt(node, "maxDepth", 5);

	if (name == "path")
		return new PathIntegrator(maxDepth);
	else if (name == "direct")
	{
		std::string lightStrategy = AttributeString(node, "lightStrategy", "sampleOneUniform");

		LightStrategy ls;
		if (lightStrategy == "sampleAllUniform")
			ls = LS_Sample_All_Uniform;
		else
			ls = LS_Sample_One_Uniform;

		return new DirectLightingIntegrator(ls,maxDepth);
	}		
	else if (name == "whitted")
		return new WhittedIntegrator(maxDepth);
	else
		assert(false);

	return new WhittedIntegrator();
}

Filter* CreateFilter(rapidxml::xml_node<>* node)
{
	std::string name = AttributeString(node, "type", "gaussian");
	
	assert(name == "gaussian");

	float radius = AttributeFloat(node, "radius", 4.0f);
	float alpha = AttributeFloat(node, "alpha", 1.0f);
	
	return new GaussianFilter(radius, alpha);
}

Film* CreateFilm(rapidxml::xml_node<>* node)
{
	int width = AttributeInt(node, "width", 512);
	int height = AttributeInt(node, "height", 512);

	return new Film(int2(width, height), CreateFilter(node->first_node("filter")));
}

Sampler* CreateSampler(rapidxml::xml_node<>* node)
{
	std::string name = AttributeString(node, "type", "stratified");
	
	if (name == "stratified")
	{
		rapidxml::xml_node<>* film = node->parent()->first_node("film");
		assert(film);

		int width = AttributeInt(film, "width", 512);
		int height = AttributeInt(film, "height", 512);

		rapidxml::xml_node<>*  sppNode = node->first_node("spp");
		int xSpp = AttributeInt(sppNode, "x", 4);
		int ySpp = AttributeInt(sppNode, "y", 4);

		return new StratifiedSampler(0, width, 0, height, xSpp, ySpp);
	}
	else
	{
		assert(false);
		return NULL;
	}
}

Camera* CreateCamera(rapidxml::xml_node<>* node, Film* film)
{
	std::string name = AttributeString(node, "type", "perspective");
	
	if (name == "perspective")
	{
		float fov = AttributeFloat(node->first_node("fov"), "value", 60.0f);

		rapidxml::xml_node<>* lookatNode = node->first_node("lookat"); 
		
		rapidxml::xml_node<>* tmpNode;

		float3 origin;
		tmpNode = lookatNode->first_node("origin"); 
		origin.X() = AttributeFloat(tmpNode, "x", 0.0f);
		origin.Y() = AttributeFloat(tmpNode, "y", 0.0f);
		origin.Z() = AttributeFloat(tmpNode, "z", 0.0f);

		float3 target;
		tmpNode = lookatNode->first_node("target"); 
		target.X() = AttributeFloat(tmpNode, "x", 0.0f);
		target.Y() = AttributeFloat(tmpNode, "y", 0.0f);
		target.Z() = AttributeFloat(tmpNode, "z", 0.0f);

		float3 up;
		tmpNode = lookatNode->first_node("up"); 
		up.X() = AttributeFloat(tmpNode, "x", 0.0f);
		up.Y() = AttributeFloat(tmpNode, "y", 0.0f);
		up.Z() = AttributeFloat(tmpNode, "z", 0.0f);

		float44 camTrans = MatrixInverse(CreateLookAtMatrixLH(origin, target, up));

		return new PerspectiveCamera(camTrans, ToRadian(fov), 0, 1, film);
	}
	else
	{
		assert(false);
		return NULL;
	}
}

shared_ptr<Material> CreateMaterial(rapidxml::xml_node<>* node)
{
	std::string type = AttributeString(node, "type", "");

	auto tex3Loader = [](rapidxml::xml_node<>* texNode) -> shared_ptr<Texture<ColorRGB>>
	{
		if (AttributeString(texNode, "image", "") == "image")
		{
			return std::make_shared<RGBImageTexture>(AttributeString(texNode, "filename", ""), TAM_Clamp, TAM_Clamp); 
		}
		else
		{
			float r = AttributeFloat(texNode, "r", 0.0f);
			float g = AttributeFloat(texNode, "r", 0.0f);
			float b = AttributeFloat(texNode, "r", 0.0f);
			return std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(r,g,b));
		}
	};

	auto tex1Loader = [](rapidxml::xml_node<>* texNode) -> shared_ptr<Texture<float>>
	{
		float v = AttributeFloat(texNode, "value", 0.0f);
		return std::make_shared<ConstantTexture<float>>(v);
	};

	if (type == "diffuse")
		return std::make_shared<DiffuseMaterial>(tex3Loader(node->first_node("kd")));
	else if (type == "mirror")
		return std::make_shared<MirrorMaterial>(tex3Loader(node->first_node("kd")));
	else if (type == "glass")
	{
		return std::make_shared<GlassMaterial>(
			tex3Loader(node->first_node("kd")),
			tex3Loader(node->first_node("kt")),
			tex1Loader(node->first_node("index")));
	}
	else if (type == "phong")
	{
		return std::make_shared<PhongMaterial>(
			tex3Loader(node->first_node("kd")),
			tex3Loader(node->first_node("ks")),
			tex1Loader(node->first_node("exponent")));
	}
	else
		return NULL;
}

shared_ptr<Shape> CreateShape(rapidxml::xml_node<>* node)
{
	std::string type = AttributeString(node, "type", "");

	shared_ptr<Shape> retVal;

	if (type == "mesh")
	{
		auto transNode = node->first_node("transform");

		bool ro = (AttributeString(transNode, "reverseOrientation", "false") == "false") ? false : true;
		float44 world = CreateTransform(transNode);

		shared_ptr<Material> material = CreateMaterial(node->first_node("material"));

		std::string filename = AttributeString(node->first_node("filename"), "value", "");

		FILE* pFile;
		if( fopen_s(&pFile, filename.c_str(), "rb") )
			return NULL;

		int nunIndices, numVertices, hasTanget;
		fread(&nunIndices, sizeof(int), 1, pFile);
		fread(&numVertices, sizeof(int),1, pFile);
		fread(&hasTanget, sizeof(int), 1, pFile);

		unsigned int* indices = new unsigned int[nunIndices];	
		float3* positions = new float3[numVertices];
		float3* normals = new float3[numVertices];
		float2* texcoords = new float2[numVertices];
		float3* tangets = hasTanget ? (new float3[numVertices]) : NULL;

		fread(indices, sizeof(unsigned int), nunIndices, pFile);
		fread(positions, sizeof(float3), numVertices, pFile);
		fread(normals, sizeof(float3), numVertices, pFile);
		fread(texcoords, sizeof(float3), numVertices, pFile);

		if (hasTanget)
		{
			fread_s(tangets, sizeof(float3)*numVertices, sizeof(float), numVertices*3, pFile);
		}

		fclose(pFile);

		return std::make_shared<Mesh>(world, ro, nunIndices/3, numVertices, indices, positions, normals, tangets, texcoords);
	}
	else if (type == "sphere")
	{
		auto transNode = node->first_node("transform");

		bool ro = (AttributeString(transNode, "reverseOrientation", "false") == "false") ? false : true;
		float44 world = CreateTransform(transNode);

		shared_ptr<Material> material = CreateMaterial(node->first_node("material"));

		return NULL;

	}
	else
	{

	}

	return NULL;
}

//Light* CreateLight(rapidxml::xml_node<>* node, Scene* scene)
//{
//
//}

Scene* LoadScene( const std::string& filename )
{
	std::ifstream sceneFile(filename.c_str());
	
	vector<char> buffer((std::istreambuf_iterator<char>(sceneFile)), std::istreambuf_iterator<char>());
	buffer.push_back('\0');

	rapidxml::xml_document<> doc;
	doc.parse<0>(&buffer[0]); 
	
	Scene* scene = new Scene;

	auto root = doc.first_node();

	scene->mSurfaceIntegrator = CreateSurfaceIntegrator(root->first_node("integrator"));
	scene->mCamera = CreateCamera( root->first_node("camera"), CreateFilm(root->first_node("film")) );
	scene->mSampler = CreateSampler( root->first_node("sampler") );

	for (auto shapeNode = root->first_node("shape"); shapeNode; shapeNode = shapeNode->next_sibling("shape"))
	{
		auto shape = CreateShape(shapeNode);
		if (shape)
			scene->mKDTree->AddShape(shape);
	}
		
	



	return scene;
}

}