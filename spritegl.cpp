//////////////////////////////////////////////////////////////////////
// Yet Another Tibia Client
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GL/glu.h>
#ifdef WIN32
#include <GL/glext.h>
#endif
#include "spritegl.h"

SpriteGL::SpriteGL(const std::string& filename, int index) :
Sprite()
{
	m_texture = GL_INVALID_VALUE;
	SpriteGL::loadFromFile(filename, index);
}

SpriteGL::~SpriteGL()
{
	if(m_texture != GL_INVALID_VALUE){
		glDeleteTextures(1, &m_texture);
	}
}

void SpriteGL::loadFromFile(const std::string& filename, int index)
{
	if(m_texture != GL_INVALID_VALUE){
		glDeleteTextures(1, &m_texture);
	}

	if(!loadSurfaceFromFile(filename, index)){
		return;
	}

	SDL_LockSurface(getImage());

	//TODO : Do not create an openGL texture for every sprite!!
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D,
			 GL_RGBA,
			 getImage()->w, getImage()->h,
			 getPixelFormat(),
			 GL_UNSIGNED_BYTE,
			 getImage()->pixels);

	SDL_UnlockSurface(getImage());
}

void SpriteGL::Blit(float destx, float desty, float srcx, float srcy, float width, float height)
{
	// TODO (mips_act#3#): Configure glict to dont enable/disable all the time
	// GL_TEXTURE_2D
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, m_texture);

	float spriteWidth = getImage()->w;
	float spriteHeight = getImage()->h;

	glBegin(GL_QUADS);
		glTexCoord2f(srcx/spriteWidth, srcy/spriteHeight);
		glVertex2f(destx, desty);

		glTexCoord2f(srcx/spriteWidth, (srcy + height)/spriteHeight);
		glVertex2f(destx, desty + height);

		glTexCoord2f((srcx + width)/spriteWidth, (srcy + height)/spriteHeight);
		glVertex2f(destx + width, desty + height);

		glTexCoord2f((srcx + width)/spriteWidth, srcy/spriteHeight);
		glVertex2f(destx + width, desty);
	glEnd();

	glPopMatrix();
	/*
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(destx, desty);
		glTexCoord2f(0, height/m_image->h);
		glVertex2f(destx, desty + height);
		glTexCoord2f(width/m_image->w, height/m_image->h);
		glVertex2f(destx + width, desty + height);
		glTexCoord2f(width/m_image->w, 0);
		glVertex2f(destx + width, desty);
	glEnd();
	*/
}