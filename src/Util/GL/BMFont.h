// BMFont.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include "ShaderWithVariables.h"

// http://www.angelcode.com/products/bmfont/doc/file_format.html#bin

#pragma pack(push,1)

// Each block starts with a one byte block type identifier, followed by a 4 byte integer that gives the size of the block, 
struct fontBlock
{
    char blockType : 8;
    int blockSize : 32;
};

struct fontInfo
{
    int fontSize              : 16; // 	2 	int 	0 	
    unsigned int bitField     : 8; // 	1 	bits 	2 	bit 0: smooth, bit 1: unicode, bit 2: italic, bit 3: bold, bit 4: fixedHeigth, bits 5-7: reserved
    unsigned int charSet      : 8; // 	1 	uint 	3 	
    unsigned int stretchH     : 16; // 	2 	uint 	4 	
    unsigned int aa           : 8; // 	1 	uint 	6 	
    unsigned int paddingUp    : 8; // 	1 	uint 	7 	
    unsigned int paddingRight : 8; //	1 	uint 	8 	
    unsigned int paddingDown  : 8; // 	1 	uint 	9 	
    unsigned int paddingLeft  : 8; // 	1 	uint 	10	
    unsigned int spacingHoriz : 8; //	1 	uint 	11	
    unsigned int spacingVert  : 8; // 	1 	uint 	12	
    unsigned int outline      : 8; // 	1 	uint 	13	added with version 2
    char* fontName; // 	n+1	string 	14	null terminated string with length n
};

struct fontCommon
{
    unsigned int lineHeight: 16; // 	2 	uint 	0 	
    unsigned int base      : 16; // 	2 	uint 	2 	
    unsigned int scaleW    : 16; // 	2 	uint 	4 	
    unsigned int scaleH    : 16; // 	2 	uint 	6 	
    unsigned int pages     : 16; // 	2 	uint 	8 	
    unsigned int bitField  : 8; // 	1 	bits 	10 	bits 0-6: reserved, bit 7: packed
    unsigned int alphaChnl : 8; // 	1 	uint 	11 	
    unsigned int redChnl   : 8; // 	1 	uint 	12 	
    unsigned int greenChnl : 8; // 	1 	uint 	13 	
    unsigned int blueChnl  : 8; // 	1 	uint 	14
};

struct fontChar
{
    unsigned int id     : 32; //	4 	uint 	0+c*20 	These fields are repeated until all characters have been described
    unsigned int x      : 16; //	2 	uint 	4+c*20 	
    unsigned int y      : 16; //	2 	uint 	6+c*20 	
    unsigned int width  : 16; //	2 	uint 	8+c*20 	
    unsigned int height : 16; //	2 	uint 	10+c*20 	
    int xoffset         : 16; //	2 	int 	12+c*20 	
    int yoffset         : 16; //	2 	int 	14+c*20 	
    int xadvance        : 16; // 	2 	int 	16+c*20 	
    unsigned int page   : 8; //	1 	uint 	18+c*20 	
    unsigned int chnl   : 8; //	1 	uint 	19+c*20
};

struct kerningPair
{
    unsigned int first  : 32; // 	4 	uint 	0+c*10 	These fields are repeated until all kerning pairs have been described
    unsigned int second : 32; //	4 	uint 	4+c*10 	
    short amount        : 16; //	2 	int 	8+c*6
};

#pragma pack(pop)

class BMFont
{
public:
    BMFont(const std::string& sourceFile);
    virtual ~BMFont();

    void initGL();
    void DrawString(const std::string& text,
        int x, int y,
        const glm::mat4& modelview,
        const glm::mat4& projection) const;

protected:
    void LoadFromBinary(const std::string& fntFileName);
    void PopulateArrays(
        const std::string& text,
        int x,
        int y,
        std::vector<float>& verts,
        std::vector<unsigned int>& indxs) const;

    std::vector<fontChar> m_chars;
    std::vector<kerningPair> m_kerningPairs;
    std::vector<std::string> m_pageNames;
    std::vector<GLuint> m_texturePages;
    unsigned int m_pageSzx;
    unsigned int m_pageSzy;
    ShaderWithVariables m_fontRender;

private: // Disallow default, copy ctor and assignment operator
    BMFont();
    BMFont(const BMFont&);
    BMFont& operator=(const BMFont&);
};
