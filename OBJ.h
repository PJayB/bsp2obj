#pragma once

#include <map>
#include <string>

namespace id3bsp
{
    class BSP;
}

typedef std::map<std::string, std::string> StringMap;

bool DumpObj( const char* filename, const char* mtlFilename, const id3bsp::BSP* bsp, int tesselationLevel );
bool DumpMtl( const char* filename, const id3bsp::BSP* bsp, const StringMap& textureRemap );
bool DumpEnts( const char* filename, const id3bsp::BSP* bsp );

