#ifndef OBJECT_FILE_LOADER_H
#define OBJECT_FILE_LOADER_H

#include <GL/glew.h>

// loading files

#include <fstream>

// get line in file

#include <sstream>

// Need these headers to support the array types I want

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <string>
#include <stdint.h>
#include <algorithm>

#include <iostream>
#include <fstream>

#include "Geometry.h"

#include "Helper.h"
#include "vec.h"
#include "mat.h"
#include "Image.h"

// TMP

extern float angleX, angleY;

// namespace time

using namespace std;// load all std:: things

namespace std
{
	template <>
	struct equal_to<Vec3f> : public unary_function<Vec3f, bool>
	{
		bool operator() (const Vec3f &a, const Vec3f &b) const
		{
			return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
		}
	};

	template<>
	struct hash<Vec3f> : public unary_function<Vec3f, size_t>
	{
		std::size_t operator() (const Vec3f& a) const
		{
			return std::hash<float>{}(a[0]) ^ std::hash<float>{}(a[1]) ^ std::hash<float>{}(a[2]);
		}
	};
};

struct Material
{
	std::string name;

	Vec4f ambient;

	Vec4f diffuse;

	Vec4f specular;

	Vec4f radiance;

	float shininess;

	float opticalDensity;

	float alpha;

	bool specularb;

	GLuint specID;

	Image specTex;

	GLuint specHighID;

	Image specHighTex;

	GLuint texID;

	Image texture;

	GLuint albedoID;

	Image albedoTex;

	GLuint dispID;

	float displaceMult;

	Image dispTex;

	GLuint bumpID;

	float bumpMult;

	Image bumpTex;

	bool usesAlbedo()
	{
		return !albedoTex.empty();
	}
	
	bool usesSpec()
	{
		return !specTex.empty();
	}

	bool usesSpecHigh()
	{
		return !specHighTex.empty();
	}

	bool usesTex()
	{
		return !texture.empty();
	}

	bool usesDispTex()
	{
		return !dispTex.empty();
	}

	bool usesBumpTex()
	{
		return !bumpTex.empty();
	}
};

struct MaterialUpload
{
	Vec4f ambient;

	Vec4f diffuse;

	Vec4f specular;

	float shininess;

	float opticalDensity;

	float alpha;

	bool specularb;

	GLuint texID;
};

struct Vertex
{
	Vec3f position;
	Vec3f normal;
	Vec2f texcoord;
	Vec3f tangent;
};

struct VertexDataforMap
{
	Vec3f normal;
	Vec2f texcoord;
	unsigned int index;
};

class ObjLoader : public Geometry
{
public:

	ObjLoader();

	~ObjLoader(); // destruction method

	void Load(const char *filename); // Load the object with its materials
									 // get the number of materials used

	void reset();

	void draw(); // regular draw

	void drawPRT(const Vec3f** coeffs);

	void drawLDPRT(const Vec3f** coeffs);

	const size_t getMatNum()
	{
		return material.size();
	}

	// get the material pointer

	Material* getMaterials()
	{
		return &material.at(0);
	}

	// get the number of vertices in the object

	const unsigned int getNumVertices()
	{
		return numVerts;
	}

	// get the number of indices in the object

	const size_t getNumIndices()
	{
		size_t indices = 0;

		for (size_t i = 0; i < vx_array_i.size(); i++)
			indices += vx_array_i[i].size();

		return indices;
	}

	// get the number of material indices in the object

	const size_t getNumMaterialIndices(size_t mat_num)
	{
		return vx_array_i[mat_num].size();
	}

	// get the number of materials in the object

	const size_t getNumMaterials()
	{
		return material.size();
	}

	// get a pointer to the verticies

	const Vertex* getVertices()
	{
		return vertex_final_array;
	}

	// get the vertex stride

	unsigned int getVertexStride()
	{
		return sizeof(Vertex);
	}

	// get a pointer to the indices

	const unsigned int* getIndices(size_t mat_num)
	{
		return &vx_array_i[mat_num].at(0);
	}

	// get the number of meshes used to draw the object

	const unsigned int getNumMesh()
	{
		return mesh_num;
	}

	// get a pointer to a certain material

	Material* getMaterial(unsigned int mat_num)
	{
		return &material.at(mat_num);
	}

	// setup Geometry accessors
	Vec3f getBoundingBoxMin()
	{
		return minBB;
	}

	Vec3f getBoundingBoxMax()
	{
		return maxBB;
	}

	Vec2i findByIndex(size_t index)
	{
		index *= 3;

		// find index
		size_t mat;
		for (mat = 0; mat < vx_array_i.size(); mat++)
		{
			if (vx_array_i[mat].size() <= index)
				index -= vx_array_i[mat].size();
			else
				break;
		}

		return Vec2i(int(index), int(mat));
	}

	Vec3f getMinBB(size_t index)
	{
		Vec2i indexData = findByIndex(index);
		index = indexData[0];
		int mat = indexData[1];

		size_t vIndex[3] =
		{
			vx_array_i[mat][index],
			vx_array_i[mat][index + 1],
			vx_array_i[mat][index + 2]
		};

		Vec3f bb = { FLT_MAX, FLT_MAX, FLT_MAX };
		for (size_t i = 0; i < 3; i++)
		{
			bb = {
				min(bb[i], vertex_final_array[vIndex[i]].position[0]),
				min(bb[i], vertex_final_array[vIndex[i]].position[1]),
				min(bb[i], vertex_final_array[vIndex[i]].position[2])
			};
		}

		return bb;
	}

	Vec3f getMaxBB(size_t index)
	{
		Vec2i indexData = findByIndex(index);
		index = indexData[0];
		int mat = indexData[1];

		size_t vIndex[3] =
		{
			vx_array_i[mat][index],
			vx_array_i[mat][index + 1],
			vx_array_i[mat][index + 2]
		};

		Vec3f bb = { FLT_MIN, FLT_MIN, FLT_MIN };
		for (size_t i = 0; i < 3; i++)
		{
			bb = {
				max(bb[i], vertex_final_array[vIndex[i]].position[0]),
				max(bb[i], vertex_final_array[vIndex[i]].position[1]),
				max(bb[i], vertex_final_array[vIndex[i]].position[2])
			};
		}

		return bb;
	}

	Vec3f getCentroid(size_t index)
	{
		return (getBoundingBoxMin() + getBoundingBoxMax()) / 2.f;
	}

	Vec3f getNormal(size_t vertID)
	{
		return vertex_final_array[vertID].normal;
	}

	Vec3f getPosition(size_t vertID)
	{
		return vertex_final_array[vertID].position;
	}

	Vec3f getColor(size_t vertID)
	{
		return vertexToColor[vertex_final_array[vertID].position];
	}

	size_t numberOfObjects()
	{
		return getNumIndices() / 3.f;
	}

	size_t numberofVerts()
	{
		return getNumVertices();
	}

	bool intersectLocal(Ray ray, isect& isect, size_t index);

private:

	// Create a vector to store the verticies

	void Load_Geometry(const char *filename); // load the verticies and indices

	void Material_File(string filename, string matfile, unsigned long* tex_num); // load the material file

	void Base_Mat(Material *mat); // the basic material

	unordered_map <unsigned int, vector<unsigned int>> vx_array_i; // store the indecies for the vertex

	unordered_map  <Vec3f, Vec3f> vertexToColor; // associate color with vertex for PRT

	vector <float> vx_array; // store the verticies in the mesh

	Vertex* vertex_final_array = nullptr; // the final verticies organized for Direct3D to draw

	vector <Material> material; // the materials used on the object

	unsigned int numVerts;

	unsigned int mesh_num; // the number of meshes

	Vec3f minBB, maxBB;

	enum IMAGE_TYPE
	{
		I_KS = 0,
		I_NS,
		I_KD,
		I_AL,
		I_DISP,
		I_BUMP
	};
};

#endif