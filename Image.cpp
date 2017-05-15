#include "Image.h"

Image::Image()
{

}

Image::~Image()
{
	if (imageID != 0)
		ilDeleteImage(imageID);
}

bool Image::empty()
{
	return imageID == 0;
}

bool Image::load(const char* file)
{
	// clear out old image
	if (!empty())
		ilDeleteImage(imageID);

	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	
	if (!ilLoadImage(file))
	{
		printf("Could not load %s\n", file);
		return false;
	}

	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);

	data = ilGetData();

	// unbind the image and delete it
	ilBindImage(0);
	
	return true;
}

bool Image::convert(ILenum format, ILenum type)
{
	ilBindImage(imageID);

	if (!ilConvertImage(format, type))
		return false;

	data = ilGetData();

	ilBindImage(0);

	return true;
}

bool Image::convert4f()
{
	return convert(IL_RGBA, IL_FLOAT);
}

GLuint Image::genGlImage()
{
	ilBindImage(imageID);

	glGenTextures(1, &glImageID);
	glBindTexture(GL_TEXTURE_2D, glImageID);

	// set wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT),
		width, height,
		0, ilGetInteger(IL_IMAGE_FORMAT),
		ilGetInteger(IL_IMAGE_TYPE), ilGetData());

	glBindTexture(GL_TEXTURE_2D, 0);

	return glImageID;
}

ILubyte* Image::getData()
{
	return data;
}

Vec4f* Image::getData4f()
{
	return (Vec4f*)data;
}

int Image::getWidth()
{
	return width;
}

int Image::getHeight()
{
	return height;
}
