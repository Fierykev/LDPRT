#define len(in) sizeof in - 1ULL
#define len1(in) sizeof in

// OBJ
#define MTLLIB "mtllib"
#define V "v "
#define VN "vn"
#define VT "vt"
#define USEMTL "usemtl"
#define F "f"

// Material
#define NEW_MTL "newmtl"
#define KA "Ka"
#define KD "Kd"
#define KS "Ks"
#define NS "Ns"
#define NI "Ni"
#define D "d "
#define TR "Tr"
#define MAP_KS "map_Ks"
#define MAP_NS "map_Ns"
#define MAP_KD "map_Kd"
#define MAP_AL "map_Al"
#define DISP "disp"
#define DM "-dm"
#define MAP_BUMP "map_bump"
#define BM "-bm"
#define BUMP "bump"

#include <GL/glew.h>
#include "ObjectFileLoader.h" // link to the header
#include "ShaderHelper.h"
#include "PRT.h"
#include "mat.h"
#include <climits>

// TMP
#include "SphericalHarmonic.h"

#define EPSILON .00001

float angleX = 0, angleY = 0;

float getAttrib(char*& ptr, const char* attrib)
{
	size_t index = string(ptr).find(attrib);
	
	if (index == string::npos)
		return 1.f;

	ptr += index + strlen(attrib);

	// search for number
	while ((!isdigit(*ptr) && *ptr != '.' && *ptr != '-') && *ptr != '\0')
		ptr++;

	// search for end of number
	char* ptrEnd = ptr;
	while ((isdigit(*ptrEnd) || *ptrEnd == '.' || *ptr == '-') && *ptrEnd != '\0')
		ptrEnd++;

	if (*ptrEnd == '\0')
	{
		cout << "Error (OBJECT LOADER): spacing issue" << endl;
		return 1.f;
	}

	// force null terminator for stof
	char cEnd = *ptrEnd;
	*ptrEnd = '\0';

	float val = stof(ptr);

	// restore end
	*ptrEnd = cEnd;
	ptr = ptrEnd;

	// filter out whitespace
	while (iswspace(*ptr))
		ptr++;

	return val;
}

/***************************************************************************
OBJ Loading
***************************************************************************/

ObjLoader::ObjLoader()
{

}

ObjLoader::~ObjLoader()
{
	// delete all data

	reset();
}

void ObjLoader::reset()
{
	delete[] vertex_final_array;
	vertex_final_array = nullptr;

	for (auto i : material)
	{
		if (i.texID != 0)
		{
			glDeleteTextures(1, &i.texID);
			i.texID = 0;
		}
	}
}

void ObjLoader::Base_Mat(Material *mat)
{
	mat->ambient = Vec4f(0.2f, 0.2f, 0.2f, 1.f);
	mat->diffuse = Vec4f(0.8f, 0.8f, 0.8f, 1.f);
	mat->specular = Vec4f(1.0f, 1.0f, 1.0f, 1.f);
	mat->shininess = 0;
	mat->opticalDensity = 0;
	mat->alpha = 1.0f;
	mat->specularb = false;
	mat->texID = 0;
	mat->bumpID = 0;
}

void ObjLoader::Material_File(string filename, string matfile, unsigned long* tex_num)
{
	// find the directory to the material file

	string directory = filename.substr(0, filename.find_last_of('/') + 1);

	matfile = directory + matfile; // the location of the material file to the program

	// open the file
	ifstream matFile_2(matfile);

	if (matFile_2.is_open()) // If obj file is open, continue
	{
		string line_material;// store each line of the file here

		while (!matFile_2.eof()) // Start reading file data as long as we have not reached the end
		{
			getline(matFile_2, line_material); // Get a line from file
											   // convert to a char to do pointer arithmetics

			char* ptr = (char*)line_material.c_str();
			
			// remove leading white space
			while (iswspace(ptr[0]))
				ptr++;
			
			// This program is for standard Wavefront Objects that are triangulated and have normals stored in the file.  This reader has been tested with 3ds Max and Blender.

			if (strncmp(ptr, NEW_MTL, len(NEW_MTL)) == 0) // new material
			{
				ptr += len1(NEW_MTL); // move address up

				Material mat; // allocate memory to create a new material

				Base_Mat(&mat); // init the material

				mat.name = ptr; // set the name of the material

				material.push_back(mat); // add to the vector

				*tex_num = material.size() - 1;
			}
			else if (strncmp(ptr, KA, len(KA)) == 0) // ambient
			{
				ptr += len1(KA);// move address up

				sscanf(ptr, "%f %f %f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).ambient[0],
					&material.at(*tex_num).ambient[1],
					&material.at(*tex_num).ambient[2]);

				material.at(*tex_num).ambient[3] = 1.f;
			}
			else if (strncmp(ptr, KD, len(KD)) == 0) // diffuse
			{
				ptr += len1(KD);// move address up

				sscanf(ptr, "%f %f %f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).diffuse[0],
					&material.at(*tex_num).diffuse[1],
					&material.at(*tex_num).diffuse[2]);

				material.at(*tex_num).diffuse[3] = 1.f;
			}
			else if (strncmp(ptr, KS, len(KS)) == 0) // specular
			{
				ptr += len1(KS);// move address up

				sscanf(ptr, "%f %f %f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).specular[0],
					&material.at(*tex_num).specular[1],
					&material.at(*tex_num).specular[2]);

				material.at(*tex_num).specular[3] = 1.f;
			}
			else if (strncmp(ptr, NS, len(NS)) == 0) // shininess
			{
				ptr += len1(NS);// move address up

				sscanf(ptr, "%f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).shininess);

				material.at(*tex_num).shininess /= 100.f;
			}
			else if (strncmp(ptr, NI, len(NI)) == 0) // refraction
			{
				ptr += len1(NI);// move address up

				sscanf(ptr, "%f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).opticalDensity);
			}
			else if (strncmp(ptr, D, len(D)) == 0) // transparency
			{
				ptr += len(D);// move address up

				sscanf(ptr, "%f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).alpha);
			}
			else if (strncmp(ptr, TR, len(TR)) == 0) // another way to store transparency
			{
				ptr += len1(TR);// move address up

				sscanf(ptr, "%f ",							// Read floats from the line: v X Y Z
					&material.at(*tex_num).alpha);
			}
			else // assume image texture
			{
				IMAGE_TYPE type;

				// find out which map we are setting
				if (strncmp(ptr, MAP_KS, len(MAP_KS)) == 0)
				{
					type = I_KS;
					ptr += len1(MAP_KS);// move address up
				}
				else if (strncmp(ptr, MAP_NS, len(MAP_NS)) == 0)
				{
					type = I_NS;
					ptr += len1(MAP_NS);// move address up
				}
				else if (strncmp(ptr, MAP_KD, len(MAP_KD)) == 0)
				{
					type = I_KD;
					ptr += len1(MAP_KD);// move address up
				}
				else if (strncmp(ptr, MAP_AL, len(MAP_AL)) == 0)
				{
					type = I_AL;
					ptr += len1(MAP_AL);// move address up
				}
				else if (strncmp(ptr, DISP, len(DISP)) == 0)
				{
					type = I_DISP;
					ptr += len1(DISP);// move address up

					// check for property
					material.at(*tex_num).displaceMult = getAttrib(ptr, DM);
				}
				else if (strncmp(ptr, MAP_BUMP, len(MAP_BUMP)) == 0)
				{
					type = I_BUMP;
					ptr += len1(MAP_BUMP);// move address up

					// check for property
					material.at(*tex_num).bumpMult = getAttrib(ptr, BM);
				}
				else if (strncmp(ptr, BUMP, len(BUMP)) == 0)
				{
					type = I_BUMP;
					ptr += len1(BUMP);// move address up

					// check for property
					material.at(*tex_num).bumpMult = getAttrib(ptr, BM);
				}
				else // bad format line
				{
					continue;
				}

				std::string texture_path = ptr; // the material file
												// load the file
												// convert to a LPWSTR

				string filename;
				filename = directory + texture_path;

				Image* image = nullptr;
				Vec4f* radiance = nullptr;
				GLuint* texID;

				switch (type)
				{
				case I_KS:
					image = &material.at(*tex_num).specTex;
					texID = &material.at(*tex_num).specID;
					break;
				case I_NS:
					image = &material.at(*tex_num).specHighTex;
					texID = &material.at(*tex_num).specHighID;
					break;
				case I_KD:
					image = &material.at(*tex_num).texture;
					radiance = &material.at(*tex_num).radiance;
					texID = &material.at(*tex_num).texID;
					break;
				case I_AL:
					image = &material.at(*tex_num).albedoTex;
					texID = &material.at(*tex_num).albedoID;
					break;
				case I_DISP:
					image = &material.at(*tex_num).dispTex;
					texID = &material.at(*tex_num).dispID;
					break;
				case I_BUMP:
					image = &material.at(*tex_num).bumpTex;
					texID = &material.at(*tex_num).bumpID;
					break;
				default:
					continue;
				}
				
				// load image file
				if (!image->load(filename.c_str()))
					cout << "Error (OBJECT LOADER): Cannot load image " << filename << endl;

				// calc radiance for diffuse
				if (radiance != nullptr)
				{
					if (!image->convert4f())
						cout << "Error (OBJECT LOADER): Cannot convert to vec4 image " << filename << endl;

					// find avg color for radiance
					*radiance = Vec4f(0.f, 0.f, 0.f, 0.f);
					
					size_t numPixels = image->getWidth()
						* image->getHeight();

					for (size_t i = 0; i < numPixels; i++)
					{
						*radiance +=
							image->getData4f()[i];
					}

					*radiance /= numPixels;
				}

				// setup texture
				*texID = image->genGlImage();
			}
		}

		matFile_2.close(); // close the file
	}
	else
	{
		cout << "Error (OBJECT LOADER): Cannot Find Material File- " << matfile << endl;
	}
}

void ObjLoader::Load_Geometry(const char* filename)
{
	// delete past memory

	if (vertex_final_array != nullptr)
		reset();

	// allocate memory to the vectors on the heap

	vx_array_i.clear();

	vertexToColor.clear();

	vx_array.clear();

	material.clear();

	unordered_map <Vec3f, vector<VertexDataforMap>> vertexmap; // map for removing doubles

	mesh_num = 0;

	minBB = { FLT_MAX, FLT_MAX, FLT_MAX };
	maxBB = { FLT_MIN, FLT_MIN, FLT_MIN };

	// create maps to store the lighting values for the material

	ifstream objFile(filename); // open the object file

	if (objFile.is_open()) // If the obj file is open, continue
	{
		// initialize the strings needed to read the file

		string line;

		string mat;

		// the material that is used

		unsigned long material_num = 0;

		unsigned long tex_num = 0;

		numVerts = 0;

		// Store the coordinates

		vector <float> vn_array;

		vector <float> vt_array;

		while (!objFile.eof()) // start reading file data
		{
			getline(objFile, line);	// get line from file

									// convert to a char to do pointers

			const char* ptr = line.c_str();

			if (strncmp(ptr, MTLLIB, len(MTLLIB)) == 0) // load the material file
			{
				ptr += len1(MTLLIB); // move the address up

				const string material_file = ptr;// the material file

				Material_File(filename, material_file, &tex_num); // read the material file and update the number of materials
			}
			if (strncmp(ptr, V, len(V)) == 0) // the first character is a v: on this line is a vertex stored.
			{
				ptr += len(V); // move address up

						  // store the three tmp's into the verticies

				float tmp[3];

				sscanf(ptr, "%f %f %f ", // read floats from the line: X Y Z
					&tmp[0],
					&tmp[1],
					&tmp[2]);

				vx_array.push_back(tmp[0]);
				vx_array.push_back(tmp[1]);
				vx_array.push_back(tmp[2]);
			}

			else if (strncmp(ptr, VN, len(VN)) == 0) // the vertex normal
			{
				ptr += len1(VN);

				// store the three tmp's into the verticies

				float tmp[3];

				sscanf(ptr, "%f %f %f ", // read floats from the line: X Y Z
					&tmp[0],
					&tmp[1],
					&tmp[2]);

				vn_array.push_back(tmp[0]);
				vn_array.push_back(tmp[1]);
				vn_array.push_back(tmp[2]);
			}

			else if (strncmp(ptr, VT, len(VT)) == 0) // texture coordinate for a vertex
			{
				ptr += len1(VT);

				// store the two tmp's into the verticies

				float tmp[2];

				sscanf(ptr, "%f %f ",	// read floats from the line: X Y Z
					&tmp[0],
					&tmp[1]);

				vt_array.push_back(tmp[0]);
				vt_array.push_back(tmp[1]);
			}
			else if (strncmp(ptr, USEMTL, len(USEMTL)) == 0) // which material is being used
			{
				mat = line.substr(len1(USEMTL), line.length());// save so the comparison will work
														// add new to the material name so that it matches the names of the materials in the mtl file

				for (unsigned long num = 0; num < tex_num + 1; num++)// find the material
				{
					if (mat == material.at(num).name)// matches material in mtl file
					{
						material_num = num;
					}
				}
			}
			else if (strncmp(ptr, F, len(F)) == 0) // store the faces in the object
			{
				ptr += len1(F);

				int vertexNumber[3] = { 0, 0, 0 };
				int normalNumber[3] = { 0, 0, 0 };
				int textureNumber[3] = { 0, 0, 0 };

				// no texture
				if (string(ptr).find("//") != -1)
				{
					sscanf(ptr, "%i//%i %i//%i %i//%i ",
						&vertexNumber[0],
						&normalNumber[0],
						&vertexNumber[1],
						&normalNumber[1],
						&vertexNumber[2],
						&normalNumber[2]
					);

					textureNumber[0] = INT_MAX;
					textureNumber[1] = INT_MAX;
					textureNumber[2] = INT_MAX;
				}
				else
				{
					sscanf(ptr, "%i/%i/%i %i/%i/%i %i/%i/%i ",
						&vertexNumber[0],
						&textureNumber[0],
						&normalNumber[0],
						&vertexNumber[1],
						&textureNumber[1],
						&normalNumber[1],
						&vertexNumber[2],
						&textureNumber[2],
						&normalNumber[2]
					); // each point represents an X,Y,Z.
				}

				// create a vertex for this area

				for (int i = 0; i < 3; i++) // loop for each triangle
				{
					Vertex vert;

					vert.position = Vec3f(vx_array.at((vertexNumber[i] - 1) * 3), vx_array.at((vertexNumber[i] - 1) * 3 + 1), vx_array.at((vertexNumber[i] - 1) * 3 + 2));

					vert.normal = Vec3f(vn_array[(normalNumber[i] - 1) * 3], vn_array[(normalNumber[i] - 1) * 3 + 1], vn_array[(normalNumber[i] - 1) * 3 + 2]);

					if (textureNumber[i] != INT_MAX)
						vert.texcoord = Vec2f(vt_array[(textureNumber[i] - 1) * 2], vt_array[(textureNumber[i] - 1) * 2 + 1]);
					else
						vert.texcoord = Vec2f(0, 0);

					unsigned int index = 0;

					bool indexupdate = false;

					if (vertexmap.find(vert.position) != vertexmap.end())
						for (VertexDataforMap vdm : vertexmap[vert.position])
						{
							if (vert.normal == vdm.normal && vert.texcoord == vdm.texcoord) // found the index
							{
								index = vdm.index;

								indexupdate = true;
								break;
							}
						}

					// nothing found

					if (!indexupdate)
					{
						VertexDataforMap tmp;

						index = numVerts;
						tmp.normal = vert.normal;

						tmp.texcoord = vert.texcoord;

						tmp.index = index;

						minBB = {
							min(minBB[0], vert.position[0]),
							min(minBB[1], vert.position[1]),
							min(minBB[2], vert.position[2])
						};

						maxBB = {
							max(maxBB[0], vert.position[0]),
							max(maxBB[1], vert.position[1]),
							max(maxBB[2], vert.position[2])
						};

						vertexmap[vert.position].push_back(tmp);

						numVerts++;
					}

					vx_array_i[material_num].push_back(index);
				}
			}
		}

		// create the final verts

		Vertex vert;

		vertex_final_array = new Vertex[numVerts];

		for (unordered_map<Vec3f, vector<VertexDataforMap>>::iterator i = vertexmap.begin(); i != vertexmap.end(); i++)
		{
			for (VertexDataforMap vdm : i->second)
			{
				vertex_final_array[vdm.index].position = i->first;

				vertex_final_array[vdm.index].normal = vdm.normal;

				vertex_final_array[vdm.index].texcoord = vdm.texcoord;

				vertex_final_array[vdm.index].tangent = { 0.f, 0.f, 0.f };
			}
		}

		// compute tangent
		Vec3f* sTmp = new Vec3f[numVerts];
		Vec3f* tTmp = new Vec3f[numVerts];

		for (size_t i = 0; i < numVerts; i++)
			*sTmp = *tTmp = Vec3f(0, 0, 0);

		for (size_t mat = 0; mat < vx_array_i.size(); mat++)
		{
			for (size_t index = 0; index < vx_array_i[mat].size(); index += 3)
			{
				Vertex* verts[3] = {
					&vertex_final_array[vx_array_i[mat][index]],
					&vertex_final_array[vx_array_i[mat][index + 1]],
					&vertex_final_array[vx_array_i[mat][index + 2]]
				};

				Vec3f delta0 = verts[1]->position - verts[0]->position;
				Vec3f delta1 = verts[2]->position - verts[0]->position;

				Vec2f deltaUV0 = verts[1]->texcoord - verts[0]->texcoord;
				Vec2f deltaUV1 = verts[2]->texcoord - verts[0]->texcoord;

				float denom = deltaUV0[0] * deltaUV1[1] - deltaUV0[1] * deltaUV1[0];

				float scale = 1.f / denom;
				
				// div 0 check
				if (denom == 0.f)
					scale = 1.f;

				Vec3f dirS = {
					deltaUV1[1] * delta0[0] - deltaUV0[1] * delta1[0],
					deltaUV1[1] * delta0[1] - deltaUV0[1] * delta1[1],
					deltaUV1[1] * delta0[2] - deltaUV0[1] * delta1[2]
				};
				dirS *= scale;
				
				sTmp[vx_array_i[mat][index]] += dirS;
				sTmp[vx_array_i[mat][index + 1]] += dirS;
				sTmp[vx_array_i[mat][index + 2]] += dirS;

				Vec3f dirT = {
					deltaUV0[0] * delta1[0] - deltaUV1[0] * delta0[0],
					deltaUV0[0] * delta1[1] - deltaUV1[0] * delta0[1],
					deltaUV0[0] * delta1[2] - deltaUV1[0] * delta0[2]
				};
				dirT *= scale;

				tTmp[vx_array_i[mat][index]] += dirT;
				tTmp[vx_array_i[mat][index + 1]] += dirT;
				tTmp[vx_array_i[mat][index + 2]] += dirT;
			}
		}

		// run tangent calc
		for (size_t i = 0; i < numVerts; i++)
		{
			vertex_final_array[i].tangent =
				sTmp[i] - vertex_final_array[i].normal * (
					vertex_final_array[i].normal * sTmp[i]);

			vertex_final_array[i].tangent.normalize();

			// hardness not needed
		}

		delete[] sTmp, tTmp;

		// associate color with each vertex (for PRT only)
		for (int i = 0; i < getNumMaterials(); i++)
		{
			for (int j = 0; j < vx_array_i[i].size(); j ++)
			{
				if (material[i].texture.empty())
					vertexToColor[vertex_final_array[vx_array_i[i][j]].position] =
						material[i].diffuse;
				else
					vertexToColor[vertex_final_array[vx_array_i[i][j]].position] =
						material[i].radiance;
			}
		}

	}
	else
	{
		printf("Error (OBJECT LOADER):  Cannot Find Object File- %s\n", filename);
	}
}

bool ObjLoader::intersectLocal(Ray ray, isect& isect, size_t index)
{
	// for indices not faces
	Vec2i indexData = findByIndex(index);
	index = indexData[0];
	int mat = indexData[1];

	size_t vIndex[3] =
	{
		vx_array_i[mat][index],
		vx_array_i[mat][index + 1],
		vx_array_i[mat][index + 2]
	};

	Vertex* verts[] =
	{
		&vertex_final_array[vIndex[0]],
		&vertex_final_array[vIndex[1]],
		&vertex_final_array[vIndex[2]]
	};

	const Vec3f& a = verts[0]->position;
	const Vec3f& b = verts[1]->position;
	const Vec3f& c = verts[2]->position;

	// check edge deltas
	const Vec3f edge1 = b - a;
	const Vec3f edge2 = c - a;

	// cross product into a tmpVar (used a lot)
	Vec3f tmpVar = ray.direction ^ edge2;

	// grab determinant (abs(dx) < E means no intersection)
	const double determinantX = edge1 * tmpVar;

	if (abs(determinantX) < EPSILON)
		return false;

	// get inverse determinent
	const double inverseDeterminant = 1.f / determinantX;

	if (std::isnan(inverseDeterminant))
		return false;

	const Vec3f rayToTri = ray.origin - a;

	// UV and distance stored here
	Vec3f uvd;

	// inverse x (should be between 0 and 1)
	uvd[0] = (rayToTri * tmpVar) * inverseDeterminant;

	if (uvd[0] < 0.f || 1.f < uvd[0])
		return false;

	// re-use tmpvar for cross product
	// far enough away does not cause out of order
	// speculation error
	tmpVar = rayToTri ^ edge1;

	// compute inverse y
	uvd[1] = (ray.direction * tmpVar) * inverseDeterminant;

	// intersection found
	if (uvd[1] < 0.f || 1.f < uvd[0] + uvd[1])
		return false;

	// calculate z (distance)
	uvd[2] = (edge2 * tmpVar) * inverseDeterminant;

	// ray intersects triangle
	if (EPSILON < uvd[2])
	{
		isect.t = uvd[2];
		isect.bary = { 1 - uvd[0] - uvd[1], uvd[0], uvd[1] };

		isect.normal =
			verts[0]->normal * isect.bary[0] +
			verts[1]->normal * isect.bary[1] +
			verts[2]->normal * isect.bary[2];

		isect.index[0] = vIndex[0];
		isect.index[1] = vIndex[1];
		isect.index[2] = vIndex[2];

		// get material TODO: use texture
		isect.color =
			material[mat].diffuse * isect.bary[0] +
			material[mat].diffuse * isect.bary[1] +
			material[mat].diffuse * isect.bary[2]
			;

		return true;
	}

	return false;
}

void ObjLoader::Load(const char *filename)
{
	Load_Geometry(filename);
}

void ObjLoader::draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_AMBIENT);
	glEnable(GL_DIFFUSE);
	glEnable(GL_SPECULAR);
	glEnable(GL_SHININESS);

	// setup normals, verts, and texcroods
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &vertex_final_array[0].position);
	glNormalPointer(GL_FLOAT, sizeof(Vertex), &vertex_final_array[0].normal);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertex_final_array[0].texcoord);

	for (int i = 0; i < getNumMaterials(); i++)
	{
		// set texture
		glBindTexture(GL_TEXTURE_2D, material[i].texID);

		// setup mat
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &material[i].ambient[0]);

		if (material[i].texID == 0)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &material[i].diffuse[0]);
		else
		{
			Vec4f white = { 1, 1, 1, 1 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&white);
		}
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material[i].specular[0]);
		glMaterialf(GL_FRONT, GL_SHININESS, material[i].shininess);

		glDrawElements(GL_TRIANGLES, vx_array_i[i].size(), GL_UNSIGNED_INT, &vx_array_i[i].at(0));
	}

	glDisable(GL_SHININESS);
	glDisable(GL_SPECULAR);
	glDisable(GL_DIFFUSE);
	glDisable(GL_AMBIENT);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void ObjLoader::drawPRT(const Vec3f** coeffs)
{
	Mat4f rotationX;
	rotationX = Mat4f::createRotation(angleX, 1, 0, 0);

	Mat4f rotationY;
	rotationY = Mat4f::createRotation(angleY, 0, 1, 0);

	Vec3f lightDir = { 0, 0, 1 };

	lightDir = rotationX * lightDir * rotationY;

	// Render a cube
	glPushMatrix();
	{
		glTranslatef(
			lightDir[0],
			lightDir[1],
			lightDir[2]
		);

		lightDir.normalize();

		glScalef(.1, .1, .1);

		glBegin(GL_QUADS);
		{
			// Top face
			glColor3f(0.0f, 1.0f, 0.0f);  // Green
			glVertex3f(1.0f, 1.0f, -1.0f);  // Top-right of top face
			glVertex3f(-1.0f, 1.0f, -1.0f);  // Top-left of top face
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Bottom-left of top face
			glVertex3f(1.0f, 1.0f, 1.0f);  // Bottom-right of top face

										   // Bottom face
			glColor3f(1.0f, 0.5f, 0.0f); // Orange
			glVertex3f(1.0f, -1.0f, -1.0f); // Top-right of bottom face
			glVertex3f(-1.0f, -1.0f, -1.0f); // Top-left of bottom face
			glVertex3f(-1.0f, -1.0f, 1.0f); // Bottom-left of bottom face
			glVertex3f(1.0f, -1.0f, 1.0f); // Bottom-right of bottom face

										   // Front face
			glColor3f(1.0f, 0.0f, 0.0f);  // Red
			glVertex3f(1.0f, 1.0f, 1.0f);  // Top-Right of front face
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Top-left of front face
			glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom-left of front face
			glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom-right of front face

											// Back face
			glColor3f(1.0f, 1.0f, 0.0f); // Yellow
			glVertex3f(1.0f, -1.0f, -1.0f); // Bottom-Left of back face
			glVertex3f(-1.0f, -1.0f, -1.0f); // Bottom-Right of back face
			glVertex3f(-1.0f, 1.0f, -1.0f); // Top-Right of back face
			glVertex3f(1.0f, 1.0f, -1.0f); // Top-Left of back face

										   // Left face
			glColor3f(0.0f, 0.0f, 1.0f);  // Blue
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Top-Right of left face
			glVertex3f(-1.0f, 1.0f, -1.0f);  // Top-Left of left face
			glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom-Left of left face
			glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom-Right of left face

											 // Right face
			glColor3f(1.0f, 0.0f, 1.0f);  // Violet
			glVertex3f(1.0f, 1.0f, 1.0f);  // Top-Right of left face
			glVertex3f(1.0f, 1.0f, -1.0f);  // Top-Left of left face
			glVertex3f(1.0f, -1.0f, -1.0f);  // Bottom-Left of left face
			glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom-Right of left face
		}
		glEnd();
	}
	glPopMatrix();

	float light[NUM_BANDS * NUM_BANDS];
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, light);

	glBegin(GL_TRIANGLES);

	for (int i = 0; i < getNumMaterials(); i++)
	{
		for (int j = 0; j < vx_array_i[i].size(); j += 3)
		{
			size_t index[3] = {
				vx_array_i[i][j],
				vx_array_i[i][j + 1],
				vx_array_i[i][j + 2]
			};

			for (int k = 0; k < 3; k++)
			{
				Vec3f color = { 0, 0, 0 };

				for (int l = 0; l < NUM_BANDS * NUM_BANDS; l++)
				{
					color += coeffs[index[k]][l] * light[l] * 2.f;
				}

				glColor3f(color[0], color[1], color[2]);
				
				glNormal3f(
					vertex_final_array[index[k]].normal[0],
					vertex_final_array[index[k]].normal[1],
					vertex_final_array[index[k]].normal[2]
				);

				glVertex3f(
					vertex_final_array[index[k]].position[0],
					vertex_final_array[index[k]].position[1],
					vertex_final_array[index[k]].position[2]
				);
			}
		}
	}

	glEnd();
}

void ObjLoader::drawLDPRT(const Vec3f** coeffs)
{
	Mat4f rotationX;
	rotationX = Mat4f::createRotation(angleX, 1, 0, 0);

	Mat4f rotationY;
	rotationY = Mat4f::createRotation(angleY, 0, 1, 0);

	Vec3f lightDir = { 0, 0, 1 };

	lightDir = rotationX * lightDir * rotationY;

	// Render a cube
	glPushMatrix();
	{
		glTranslatef(
			lightDir[0],
			lightDir[1],
			lightDir[2]
		);

		lightDir.normalize();

		glScalef(.1, .1, .1);

		glBegin(GL_QUADS);
		{
			// Top face
			glColor3f(0.0f, 1.0f, 0.0f);  // Green
			glVertex3f(1.0f, 1.0f, -1.0f);  // Top-right of top face
			glVertex3f(-1.0f, 1.0f, -1.0f);  // Top-left of top face
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Bottom-left of top face
			glVertex3f(1.0f, 1.0f, 1.0f);  // Bottom-right of top face

										   // Bottom face
			glColor3f(1.0f, 0.5f, 0.0f); // Orange
			glVertex3f(1.0f, -1.0f, -1.0f); // Top-right of bottom face
			glVertex3f(-1.0f, -1.0f, -1.0f); // Top-left of bottom face
			glVertex3f(-1.0f, -1.0f, 1.0f); // Bottom-left of bottom face
			glVertex3f(1.0f, -1.0f, 1.0f); // Bottom-right of bottom face

										   // Front face
			glColor3f(1.0f, 0.0f, 0.0f);  // Red
			glVertex3f(1.0f, 1.0f, 1.0f);  // Top-Right of front face
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Top-left of front face
			glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom-left of front face
			glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom-right of front face

											// Back face
			glColor3f(1.0f, 1.0f, 0.0f); // Yellow
			glVertex3f(1.0f, -1.0f, -1.0f); // Bottom-Left of back face
			glVertex3f(-1.0f, -1.0f, -1.0f); // Bottom-Right of back face
			glVertex3f(-1.0f, 1.0f, -1.0f); // Top-Right of back face
			glVertex3f(1.0f, 1.0f, -1.0f); // Top-Left of back face

										   // Left face
			glColor3f(0.0f, 0.0f, 1.0f);  // Blue
			glVertex3f(-1.0f, 1.0f, 1.0f);  // Top-Right of left face
			glVertex3f(-1.0f, 1.0f, -1.0f);  // Top-Left of left face
			glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom-Left of left face
			glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom-Right of left face

											 // Right face
			glColor3f(1.0f, 0.0f, 1.0f);  // Violet
			glVertex3f(1.0f, 1.0f, 1.0f);  // Top-Right of left face
			glVertex3f(1.0f, 1.0f, -1.0f);  // Top-Left of left face
			glVertex3f(1.0f, -1.0f, -1.0f);  // Bottom-Left of left face
			glVertex3f(1.0f, -1.0f, 1.0f);  // Bottom-Right of left face
		}
		glEnd();
	}
	glPopMatrix();

	float light[NUM_BANDS * NUM_BANDS];
	SphericalHarmonic::evaluateSphericalHarmonic6(lightDir, light);

	glBegin(GL_TRIANGLES);

	for (int i = 0; i < getNumMaterials(); i++)
	{
		for (int j = 0; j < vx_array_i[i].size(); j += 3)
		{
			size_t index[3] = {
				vx_array_i[i][j],
				vx_array_i[i][j + 1],
				vx_array_i[i][j + 2]
			};

			float sh[6 * 6];

			for (int k = 0; k < 3; k++)
			{
				Vec3f color = { 0, 0, 0 };

				Vec3f normal = vertex_final_array[index[k]].normal;
				normal.normalize();

				// get harmonic in norm dir
				SphericalHarmonic::evaluateSphericalHarmonic6(
					normal, sh);

				size_t shIndex = 0;
				Vec3f tmp = 0;
				for (int l = 0; l < NUM_BANDS; l++)
				{
					float weight = 0;

					for (int m = -l; m <= l; m++)
					{
						weight += sh[shIndex] * light[shIndex];

						shIndex++;
					}

					color += coeffs[index[k]][l] * weight * 2.f;
				}

				glColor3f(color[0], color[1], color[2]);

				glNormal3f(
					vertex_final_array[index[k]].normal[0],
					vertex_final_array[index[k]].normal[1],
					vertex_final_array[index[k]].normal[2]
				);

				glVertex3f(
					vertex_final_array[index[k]].position[0],
					vertex_final_array[index[k]].position[1],
					vertex_final_array[index[k]].position[2]
				);
			}
		}
	}

	glEnd();
}