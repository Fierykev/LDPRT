#include "MorphTarget.h"
#include "ObjectFileLoader.h"
#include "ShaderHelper.h"
#include "LDPRT.h"
#include "SphericalHarmonic.h"
#include "LightProb.h"
#include "Camera.h"

GLuint Morphs::morphSampler = 0;
GLuint Morphs::morphProgram = 0, Morphs::morphVS = 0,
	Morphs::morphGS = 0, Morphs::morphFS = 0;
GLuint Morphs::morphBuffer = 0;
GLfloat Morphs::viewportMorph[4 * MORPH_CHAN];

GLuint MorphTarget::morphTargetSampler = 0;
GLuint MorphTarget::morphTargetProgram, MorphTarget::morphTargetVS,
	MorphTarget::morphTargetTCS, MorphTarget::morphTargetTES,
	MorphTarget::morphTargetGS, MorphTarget::morphTargetFS;

size_t MorphTarget::toggle = 0;
bool MorphTarget::animate = false;
bool MorphTarget::tess = false;

Morphs::Morphs()
{

}

Morphs::~Morphs()
{
	if (tex != 0)
		glDeleteTextures(1, &tex);
}

void Morphs::setupViewport()
{
	// setup viewports
	for (unsigned int i = 0; i < 4 * MORPH_CHAN; i += 4)
	{
		viewportMorph[i] = 0;
		viewportMorph[i + 1] = 0;
		viewportMorph[i + 2] = size;
		viewportMorph[i + 3] = size;
	}
}

void Morphs::genTex(ObjLoader* obj, ObjLoader* baseObj)
{
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	
	// transfer buffer
	size = sqrt(obj->getNumVertices()) + 1;

	Vec3f* morphUpload = new Vec3f[size * size * MORPH_CHAN];

	const Vertex* verts = obj->getVertices();

	const Vertex* baseVerts = nullptr;
	if (baseObj != NULL)
		baseVerts = baseObj->getVertices();
	
	for (size_t i = 0; i < obj->getNumVertices(); i++)
	{
		if (baseObj != NULL)
		{
			morphUpload[i] = verts[i].position - baseVerts[i].position;
			morphUpload[i + size * size] = verts[i].normal - baseVerts[i].normal;
			morphUpload[i + size * size * 2] = verts[i].tangent - baseVerts[i].tangent;
		}
		else
		{
			morphUpload[i] = verts[i].position;
			morphUpload[i + size * size] = verts[i].normal;
			morphUpload[i + size * size * 2] = verts[i].tangent;
		}
	}

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1,
		GL_RGB32F, size, size, MORPH_CHAN);
	
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		size, size, MORPH_CHAN, GL_RGB, GL_FLOAT, morphUpload);

	delete[] morphUpload;
}

void Morphs::setMorph(float amount, GLuint sceneTex, bool alpha)
{
	if (morphProgram == 0)
	{
		// create program
		createProgram(&morphProgram, &morphVS, nullptr, nullptr, &morphFS, &morphGS,
			"RenderMorph.vs", "", "", "RenderMorph.gs", "RenderMorph.fs");

		// gen plane for render
		glGenBuffers(1, &morphBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, morphBuffer);

		glBufferData(GL_ARRAY_BUFFER,
			6 * sizeof(PlaneVerts),
			planeData,
			GL_STATIC_DRAW
		);
		
		// gen sampler
		glGenSamplers(1, &morphSampler);
		glSamplerParameteri(morphSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(morphSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(morphSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(morphSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(morphSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	
	glUseProgram(morphProgram);

	if (alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	// setup uniforms

	glUniform1f(
		glGetUniformLocation(morphProgram, "blend"),
		amount
	);

	glUniform1i(
		glGetUniformLocation(morphProgram, "sourceTex"),
		0
	);

	// set texture

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glBindSampler(0, morphSampler);
	
	for (int i = 0; i < 2; i++)
		glEnableVertexAttribArray(i);

	glBindBuffer(GL_ARRAY_BUFFER, morphBuffer);

	glVertexAttribPointer(glGetAttribLocation(morphProgram, "pos"), 3, GL_FLOAT, GL_FALSE, sizeof(PlaneVerts), BUFFER_OFFSET(0));
	glVertexAttribPointer(glGetAttribLocation(morphProgram, "texCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(PlaneVerts), BUFFER_OFFSET(sizeof(Vec3f)));
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	for (int i = 0; i < 2; i++)
		glDisableVertexAttribArray(i);

	glDisable(GL_BLEND);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

GLuint Morphs::getMorphID()
{
	return tex;
}

MorphTarget::MorphTarget()
{

}

MorphTarget::~MorphTarget()
{
	reset();
}

void MorphTarget::reset()
{
	// reset time
	timeM = 0.f;

	delete[] morphs;

	if (sceneBuffer != nullptr)
	{
		glDeleteBuffers(baseObj.getNumMaterials(), sceneBuffer);
	}

	if (sceneBuffer != 0)
	{
		glDeleteFramebuffers(1, &sceneFB);
		sceneFB = 0;
	}

	if (sceneTex != 0)
	{
		glDeleteTextures(1, &sceneTex);
		sceneTex = 0;
	}

	baseObj.reset();
	delete[] sceneBuffer;
	sceneBuffer = nullptr;
}

void MorphTarget::load(const char** morphList, size_t size)
{
	if (size == 0)
	{
		printf("MorphTarget: Nothing Specified for Load\n");
		return;
	}

	morphs = new Morphs[size];

	numMorphs = size;

	// set oily
	oily = .5f;

	// load environment
	if (environTex == 0)
	{
		Image lightProbeTex;
		lightProbeTex.load("HDRI/White_Room.jpg");
		environTex = lightProbeTex.genGlImage();
	}
	
	// setup buffer
	{
		baseObj.Load(morphList[0]);
		
		string baseName(morphList[0]);
		size_t startIndex = baseName.find_last_of("/\\");

		if (startIndex == string::npos)
			startIndex = 0;

		size_t lastIndex = baseName.find_last_of('.');

		if (lastIndex == string::npos)
			lastIndex = baseName.length();

		if (baseName.length() <= startIndex + 1 || lastIndex <= startIndex + 1)
		{
			cout << "ERROR Morph Target: cannot find name of file" << endl;
			return;
		}

		ldprt.load(baseName.substr(startIndex + 1, lastIndex - startIndex - 1).c_str(),
			&baseObj);

		// create morph
		morphs[0].genTex(&baseObj, NULL);
		morphs[0].setupViewport();
		
		sceneBuffer = new GLuint[baseObj.getNumMaterials()];
		
		for (size_t i = 0; i < baseObj.getNumMaterials(); i++)
		{
			MorphTargetUpload* upload =
				new MorphTargetUpload[baseObj.getNumMaterialIndices(i)];

			Vec4f pack[2];
			const Vec3f** coeffs = ldprt.getCoeffs();

			for (size_t j = 0; j < baseObj.getNumMaterialIndices(i); j++)
			{
				const GLuint vertID = baseObj.getIndices(i)[j];
				upload[j].vertID = vertID;
				upload[j].texCoord = baseObj.getVertices()[baseObj.getIndices(i)[j]].texcoord;
				
				// upload coeffs
				for (size_t k = 0; k < 3; k++)
				{
					upload[j].coeffs[k][0] = Vec4f(
						coeffs[vertID][0][k],
						coeffs[vertID][1][k],
						coeffs[vertID][2][k],
						coeffs[vertID][3][k]
					);

					upload[j].coeffs[k][1] = Vec4f(
						coeffs[vertID][4][k],
						coeffs[vertID][5][k],
						0,
						0
					);
				}
			}
			
			// create scene buffer
			glGenBuffers(1, &sceneBuffer[i]);

			glBindBuffer(GL_ARRAY_BUFFER, sceneBuffer[i]);
			glBufferData(GL_ARRAY_BUFFER,
				baseObj.getNumMaterialIndices(i) * sizeof(MorphTargetUpload),
				upload,
				GL_STATIC_DRAW
			);

			delete[] upload;
		}
	}

	// unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	for (size_t i = 1; i < size; i++)
	{
		ObjLoader obj;
		obj.Load(morphList[i]);

		// create morph
		morphs[i].genTex(&obj, &baseObj);
	}
	
	glGenFramebuffers(1, &sceneFB);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFB);

	// create rtv / scene tex
	glGenTextures(1, &sceneTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, sceneTex);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1,
		GL_RGB32F, morphs[0].width(), morphs[0].width(), MORPH_CHAN);

	for (size_t i = 0; i < MORPH_CHAN; i++)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			sceneTex, 0);
	}

	// attatch buffer
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		NULL, 0);

	// unbind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MorphTarget::applyMorphs()
{
	// store old viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// update morphs
	// bind frame
	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, sceneFB);

	glViewportArrayv(0, MORPH_CHAN, Morphs::viewportMorph);

	glViewport(0, 0, morphs[0].width(), morphs[0].width());
	glClearColor(0, 0, 0, 1);
	
	glPushMatrix();
	{
		morphs[0].setMorph(1.f, sceneTex, false);
		
		for (size_t i = 1; i < numMorphs; i++)
		{
			const float startOffset = -acos(.5);
			float t = sin(timeM * float(i) * .0025f + startOffset);
			t *= .5;
			t += .5;

			if (animate)
				timeM++;

			morphs[i].setMorph(t, sceneTex, true);
		}
	}
	glPopMatrix();

	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void MorphTarget::draw()
{
	switch (toggle)
	{
	case 0:
		baseObj.draw();
		return;
		break;
	case 1:
		ldprt.prtDraw();
		return;
		break;
	case 2:
		ldprt.draw();
		return;
		break;
	case 3:
		// continue to drawing below
		break;
	}

	// apply morphs
	applyMorphs();
	
	// load program if needed
	if (morphTargetProgram == 0)
	{
		// create program
		createProgram(&morphTargetProgram, &morphTargetVS, &morphTargetTCS, &morphTargetTES, &morphTargetFS, &morphTargetGS,
			"Render.vs", "Render.tcs", "Render.tes", "Render.gs", "Render.fs");
		
		// gen sampler
		glGenSamplers(1, &morphTargetSampler);
		glSamplerParameteri(morphTargetSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(morphTargetSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(morphTargetSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(morphTargetSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	
	// setup render
	glUseProgram(morphTargetProgram);

	// tess

	// environment
	glUniform1f(
		glGetUniformLocation(morphTargetProgram, "innerTess"),
		tess ? 4 : 1
	);

	// environment
	glUniform1f(
		glGetUniformLocation(morphTargetProgram, "outerTess"),
		tess ? 4 : 1
	);

	// environment
	glUniform1ui(
		glGetUniformLocation(morphTargetProgram, "environment"),
		0
		);

	// light position
	glUniform3fv(
		glGetUniformLocation(morphTargetProgram, "lightDir"),
		1, (GLfloat*)&LightProb::getLightDir()
	);

	// light intensity
	glUniform1f(
		glGetUniformLocation(morphTargetProgram, "lightIntensity"),
		LightProb::getLightIntensity()
	);
	
	// bind orig position
	glUniform1ui(
		glGetUniformLocation(morphTargetProgram, "size"),
		morphs[0].width()
	);

	glUniform1i(
		glGetUniformLocation(morphTargetProgram, "objTex"),
		1
	);

	glUniform1i(
		glGetUniformLocation(morphTargetProgram, "origTex"),
		2
	);

	// TODO: ADD IN TEXCOORD SUPPORT IN SHADER
	for (size_t i = 0; i < 8; i++)
		glEnableVertexAttribArray(i);

	Mat4f worldViewProjection =
		glGetMatrix(GL_PROJECTION_MATRIX)
		* glGetMatrix(GL_MODELVIEW_MATRIX);

	glUniformMatrix4fv(
		glGetUniformLocation(morphTargetProgram, "worldViewProjection"),
		1, GL_TRUE,
		&worldViewProjection[0][0]
	);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, environTex);
	glBindSampler(0, morphTargetSampler);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, sceneTex);
	glBindSampler(0, Morphs::morphSampler);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, morphs[0].getMorphID());
	glBindSampler(1, Morphs::morphSampler);

	// set patch to triangles
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	// draw scene
	for (size_t i = 0; i < baseObj.getNumMaterials(); i++)
	{
		size_t texID = TEX_SPLIT;

		// diffuse
		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "useDiffuseTex"),
			baseObj.getMaterial(i)->usesTex()
		);

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "diffuseTex"),
			texID
		);
		texID++;

		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "diffuseCol"),
			1,
			(const GLfloat*)&baseObj.getMaterial(i)->diffuse
		);

		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "diffuseCol"),
			1,
			(const GLfloat*)&baseObj.getMaterial(i)->diffuse
		);

		// specular
		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "useSpecTex"),
			baseObj.getMaterial(i)->usesSpec()
		);

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "specTex"),
			texID
		);
		texID++;

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "useSpecHighTex"),
			baseObj.getMaterial(i)->usesSpecHigh()
		);

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "specHighTex"),
			texID
		);
		texID++;

		glUniform1f(
			glGetUniformLocation(morphTargetProgram, "shininess"),
			baseObj.getMaterial(i)->shininess
		);

		// albedo
		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "useAlbedo"),
			baseObj.getMaterial(i)->usesAlbedo()
		);

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "albedoTex"),
			texID
		);
		texID++;

		// displacement
		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "useDispTex"),
			baseObj.getMaterial(i)->usesDispTex()
		);

		glUniform1f(
			glGetUniformLocation(morphTargetProgram, "displaceMult"),
			baseObj.getMaterial(i)->displaceMult
		);

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "dispTex"),
			texID
		);
		texID++;

		// bump
		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "useBumpTex"),
			baseObj.getMaterial(i)->usesBumpTex()
		);

		glUniform1f(
			glGetUniformLocation(morphTargetProgram, "bumpMult"),
			baseObj.getMaterial(i)->bumpMult
		);

		glUniform1i(
			glGetUniformLocation(morphTargetProgram, "bumpTex"),
			texID
		);
		texID++;
		
		// cubemaps
		size_t numCubemaps = (NUM_BANDS * NUM_BANDS >> 2);

		for (size_t i = 0; i < numCubemaps; i++)
		{
			glUniform1i(
				glGetUniformLocation(morphTargetProgram, "harmonics") + i,
				texID
			);
			texID++;
		}

		// light dir -> spherical harmonic basis
		Vec4f light[3][LIGHT_PROB_GRPS];

		LightProb::getHarmonics(light[0], light[1], light[2]);

		// lighting
		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "lightR"),
			LIGHT_PROB_GRPS, (float*)light[0]
		);
		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "lightG"),
			LIGHT_PROB_GRPS, (float*)light[1]
		);
		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "lightB"),
			LIGHT_PROB_GRPS, (float*)light[2]
		);

		// back-light dir -> spherical harmonic basis
		Vec4f backLight[3][LIGHT_PROB_GRPS];

		LightProb::getBackLightHarmonics(backLight[0], backLight[1], backLight[2]);

		// lighting
		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "backLightR"),
			LIGHT_PROB_GRPS, (float*)backLight[0]
		);
		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "backLightG"),
			LIGHT_PROB_GRPS, (float*)backLight[1]
		);
		glUniform4fv(
			glGetUniformLocation(morphTargetProgram, "backLightB"),
			LIGHT_PROB_GRPS, (float*)backLight[2]
		);

		// camera
		glUniform3fv(
			glGetUniformLocation(morphTargetProgram, "camPos"),
			1, (GLfloat*)&Camera::getCameraLoc()
		);

		// oily
		glUniform1f(
			glGetUniformLocation(morphTargetProgram, "oily"),
			oily
		);

		// TEXTURES
		texID = TEX_SPLIT;

		// bind diffuse
		glActiveTexture(GL_TEXTURE0 + texID);
		texID++;
		glBindTexture(GL_TEXTURE_2D, baseObj.getMaterial(i)->texID);
		glBindSampler(texID, morphTargetSampler);

		// bind specular
		glActiveTexture(GL_TEXTURE0 + texID);
		texID++;
		glBindTexture(GL_TEXTURE_2D, baseObj.getMaterial(i)->specID);
		glBindSampler(texID, morphTargetSampler);

		// bind specular highlight
		glActiveTexture(GL_TEXTURE0 + texID);
		texID++;
		glBindTexture(GL_TEXTURE_2D, baseObj.getMaterial(i)->specHighID);
		glBindSampler(texID, morphTargetSampler);

		// bind albedo
		glActiveTexture(GL_TEXTURE0 + texID);
		texID++;
		glBindTexture(GL_TEXTURE_2D, baseObj.getMaterial(i)->albedoID);
		glBindSampler(texID, morphTargetSampler);

		// bind displacement
		glActiveTexture(GL_TEXTURE0 + texID);
		texID++;
		glBindTexture(GL_TEXTURE_2D, baseObj.getMaterial(i)->dispID);
		glBindSampler(texID, morphTargetSampler);

		// bind bump
		glActiveTexture(GL_TEXTURE0 + texID);
		texID++;
		glBindTexture(GL_TEXTURE_2D, baseObj.getMaterial(i)->bumpID);
		glBindSampler(texID, morphTargetSampler);

		// bind cubemaps
		for (size_t i = 0; i < numCubemaps; i++)
		{
			glActiveTexture(GL_TEXTURE0 + texID);
			texID++;
			glBindTexture(GL_TEXTURE_CUBE_MAP, ldprt.getCubeMaps()[i]);
			glBindSampler(texID, morphTargetSampler);
		}

		glBindBuffer(GL_ARRAY_BUFFER, sceneBuffer[i]);

		size_t offset = 0;

		glVertexAttribIPointer(glGetAttribLocation(morphTargetProgram, "vertID"),
			1, GL_UNSIGNED_INT, sizeof(MorphTargetUpload), BUFFER_OFFSET(0));
		offset += sizeof(unsigned int);

		glVertexAttribPointer(glGetAttribLocation(morphTargetProgram, "texCoord"),
			2, GL_FLOAT, GL_FALSE, sizeof(MorphTargetUpload), BUFFER_OFFSET(offset));
		offset += sizeof(Vec2f);

		// setup rgb coeffs
		for (size_t j = 0; j < NUM_COEFF_GRPS; j++)
		{
			glVertexAttribPointer(glGetAttribLocation(morphTargetProgram, "coeffsR") + j,
				4, GL_FLOAT, GL_FALSE, sizeof(MorphTargetUpload), BUFFER_OFFSET(offset));
			offset += sizeof(Vec4f);
		}

		for (size_t j = 0; j < NUM_COEFF_GRPS; j++)
		{
			glVertexAttribPointer(glGetAttribLocation(morphTargetProgram, "coeffsG") + j,
				4, GL_FLOAT, GL_FALSE, sizeof(MorphTargetUpload), BUFFER_OFFSET(offset));
			offset += sizeof(Vec4f);
		}

		for (size_t j = 0; j < NUM_COEFF_GRPS; j++)
		{
			glVertexAttribPointer(glGetAttribLocation(morphTargetProgram, "coeffsB") + j,
				4, GL_FLOAT, GL_FALSE, sizeof(MorphTargetUpload), BUFFER_OFFSET(offset));
			offset += sizeof(Vec4f);
		}

		glDrawArrays(GL_PATCHES, 0, baseObj.getNumMaterialIndices(i));
	}

	// cleanup
	for (size_t i = 0; i < 8; i++)
		glDisableVertexAttribArray(i);

	glActiveTexture(GL_TEXTURE0);

	// reset sampler
	glBindSampler(0, 0);

	// reset buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// set to default program
	glUseProgram(0);

	// draw light
	LightProb::draw();
	
	//ldprt[0].draw();
	
	// TMP
	/*
	Vec3f* windowDepth = new Vec3f[morphs[0].width() * morphs[0].width() * MORPH_CHAN];
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, sceneTex);
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, GL_FLOAT, windowDepth);

	size_t index = morphs[0].width() * morphs[0].width() * 1;

	printf("%f %f %f\n", windowDepth[index][0], windowDepth[index][1], windowDepth[index][2]);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, morphs[0].getMorphID());
	glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, GL_FLOAT, windowDepth);

	printf("%f %f %f\n", windowDepth[index][0], windowDepth[index][1], windowDepth[index][2]);
	
	//system("PAUSE");

	delete[] windowDepth;*/
}