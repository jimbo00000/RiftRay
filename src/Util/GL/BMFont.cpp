// BMFont.cpp

#include "BMFont.h"
#include <fstream>
#include <iostream>
#include "StringFunctions.h"
#include "TextureFunctions.h"

BMFont::BMFont(const std::string& sourceFile)
: m_chars()
, m_kerningPairs()
, m_pageNames()
, m_texturePages()
{
    LoadFromBinary(sourceFile);
}

BMFont::~BMFont()
{
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
}

void BMFont::DrawString(const std::string& text, int x, int y)
{
    if (text.empty())
        return;

    for (std::string::const_iterator it = text.begin();
        it != text.end();
        ++it)
    {
        char c = *it;
    }
}
