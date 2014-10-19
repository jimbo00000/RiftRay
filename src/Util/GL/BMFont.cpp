// BMFont.cpp

#include "BMFont.h"
#include <fstream>
#include <iostream>
#include <sstream>

BMFont::BMFont(const std::string& sourceFile)
: m_chars()
, m_kerningPairs()
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
                char* pPageNames = new char[block.blockSize];
                fntFile.read(pPageNames, block.blockSize);
                delete pPageNames;
            }
            break;

        case 4:
            m_chars.resize(block.blockSize / sizeof(fontChar));
            fntFile.read(reinterpret_cast<char*>(&m_chars[0]), block.blockSize);
            break;

        case 5:
            m_kerningPairs.resize(block.blockSize / sizeof(kerningPair));
            fntFile.read(reinterpret_cast<char*>(&m_kerningPairs[0]), block.blockSize);
            break;
        }
    }
}
