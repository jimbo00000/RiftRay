// BMFont.cpp

#include "BMFont.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include <glm/gtc/type_ptr.hpp>
#include "StringFunctions.h"
#include "TextureFunctions.h"

BMFont::BMFont(const std::string& sourceFile)
: m_chars()
, m_kerningPairs()
, m_pageNames()
, m_texturePages()
, m_pageSzx(0)
, m_pageSzy(0)
{
    LoadFromBinary(sourceFile);
}

BMFont::~BMFont()
{
    glDeleteTextures(m_texturePages.size(), &m_texturePages[0]);
}

void BMFont::LoadFromBinary(const std::string& fntFileName)
{
    std::ifstream fntFile(fntFileName.c_str(), std::ios::in | std::ios::binary);
    if (!fntFile.is_open())
        return;

    char header[4];
    fntFile.read(header, 4);
    if (
        (header[0] != 'B') ||
        (header[1] != 'M') ||
        (header[2] != 'F') ||
        (header[3] != 3)
        )
        return;

    while (!fntFile.eof())
    {
        fontBlock block;
        fntFile.read(reinterpret_cast<char*>(&block), sizeof(fontBlock));
        switch(block.blockType)
        {
        default:
            break;

        case 1:
            fontInfo info;
            fntFile.read(reinterpret_cast<char*>(&info), block.blockSize); //sizeof(fontInfo));
            // We can just ignore this block; no need to bother with the font name.
            break;

        case 2:
            fontCommon comm;
            fntFile.read(reinterpret_cast<char*>(&comm), block.blockSize);
            m_pageSzx = comm.scaleW;
            m_pageSzy = comm.scaleH;
            break;

        case 3:
            {
                std::vector<char> pageStrings(block.blockSize);
                fntFile.read(&pageStrings[0], block.blockSize);

                // http://stackoverflow.com/questions/7243723/simple-way-to-split-a-sequence-of-null-separated-strings-in-c
                const char* p = &pageStrings[0];
                do {
                    m_pageNames.push_back(std::string(&pageStrings[0]));
                    p += m_pageNames.back().size() + 1;
                } while ((p - &pageStrings[0]) < block.blockSize);
            }
            break;

        case 4:
            {
                std::vector<fontChar> chars;
                chars.resize(block.blockSize / sizeof(fontChar));
                fntFile.read(reinterpret_cast<char*>(&chars[0]), block.blockSize);
                size_t maxidx = 0;
                for (std::vector<fontChar>::const_iterator it = chars.begin();
                    it != chars.end();
                    ++it)
                {
                    const fontChar& fc = *it;
                    maxidx = std::max(maxidx, fc.id);
                }
                m_chars.resize(maxidx+1);
                for (std::vector<fontChar>::const_iterator it = chars.begin();
                    it != chars.end();
                    ++it)
                {
                    const fontChar& fc = *it;
                    m_chars[fc.id] = fc;
                }
            }
            break;

        case 5:
            m_kerningPairs.resize(block.blockSize / sizeof(kerningPair));
            fntFile.read(reinterpret_cast<char*>(&m_kerningPairs[0]), block.blockSize);
            break;
        }
    }
}

void BMFont::initGL()
{
    for (std::vector<std::string>::const_iterator it = m_pageNames.begin();
        it != m_pageNames.end();
        ++it)
    {
        const std::string& tf = *it;
        const std::string fp = "../textures/" + tf;
        if (hasEnding(tf, ".png"))
        {
            GLuint width = 0;
            GLuint height = 0;
            const GLuint texId = LoadTextureFromPng(fp.c_str(), &width, &height);
            ///@todo check width/height
            m_texturePages.push_back(texId);
        }
    }

    m_fontRender.initProgram("fontrender");
    m_fontRender.bindVAO();

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_fontRender.AddVbo("vPosition", vertVbo);

    glEnableVertexAttribArray(m_fontRender.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_fontRender.GetAttrLoc("vTexCoord"));

    GLuint triVbo = 0;
    glGenBuffers(1, &triVbo);
    m_fontRender.AddVbo("elements", triVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triVbo);

    glBindVertexArray(0);
}

void BMFont::DrawString(
    const std::string& text,
    int x,
    int y,
    const glm::mat4& modelview,
    const glm::mat4& projection) const
{
    if (text.empty())
        return;
    if (m_texturePages.empty())
        return;
    assert(m_texturePages.size() == 1);

    std::vector<float> verts;
    std::vector<unsigned int> indxs;
    for (std::string::const_iterator it = text.begin();
        it != text.end();
        ++it)
    {
        char id = *it;

        // Let's assume we are in glOrtho with pixel coordinates.
        const fontChar& c = m_chars[id];
        const float texX = static_cast<float>(c.x) / static_cast<float>(m_pageSzx);
        const float texY = static_cast<float>(c.y) / static_cast<float>(m_pageSzy);
        const float texX2 = static_cast<float>(c.x + c.width) / static_cast<float>(m_pageSzx);
        const float texY2 = static_cast<float>(c.y + c.height) / static_cast<float>(m_pageSzy);
        const float x1 = static_cast<float>(x + c.xoffset);
        const float y1 = static_cast<float>(y + c.yoffset);
        const float x2 = static_cast<float>(x1 + c.width);
        const float y2 = static_cast<float>(y1 + c.height);

        // 0
        verts.push_back(x1);
        verts.push_back(y1);
        verts.push_back(0.0f);
        verts.push_back(texX);
        verts.push_back(texY);
        indxs.push_back(indxs.size());

        // 1
        verts.push_back(x2);
        verts.push_back(y1);
        verts.push_back(0.0f);
        verts.push_back(texX2);
        verts.push_back(texY);
        indxs.push_back(indxs.size());

        // 2
        verts.push_back(x2);
        verts.push_back(y2);
        verts.push_back(0.0f);
        verts.push_back(texX2);
        verts.push_back(texY2);
        indxs.push_back(indxs.size());


        // 0
        verts.push_back(x1);
        verts.push_back(y1);
        verts.push_back(0.0f);
        verts.push_back(texX);
        verts.push_back(texY);
        indxs.push_back(indxs.size());

        // 2
        verts.push_back(x2);
        verts.push_back(y2);
        verts.push_back(0.0f);
        verts.push_back(texX2);
        verts.push_back(texY2);
        indxs.push_back(indxs.size());

        // 3
        verts.push_back(x1);
        verts.push_back(y2);
        verts.push_back(0.0f);
        verts.push_back(texX);
        verts.push_back(texY2);
        indxs.push_back(indxs.size());

        x += c.xadvance;
    }

    const GLuint prog = m_fontRender.prog();
    glUseProgram(prog);
    {
        glUniformMatrix4fv(m_fontRender.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_fontRender.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

        ///@todo Support multiple font pages
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texturePages[0]);
        glUniform1i(m_fontRender.GetUniLoc("texImage"), 0);

        m_fontRender.bindVAO();
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_fontRender.GetVboLoc("vPosition"));
            glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0], GL_STATIC_DRAW);
            glVertexAttribPointer(m_fontRender.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), NULL);
            glVertexAttribPointer(m_fontRender.GetAttrLoc("vTexCoord"), 2, GL_FLOAT, GL_FALSE, 5*sizeof(float),
                (void*)(3*sizeof(float)));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_fontRender.GetVboLoc("elements"));
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxs.size()*sizeof(unsigned int), &indxs[0], GL_STATIC_DRAW);

            glDrawElements(GL_TRIANGLES,
                           indxs.size(),
                           GL_UNSIGNED_INT,
                           0);
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);
}
