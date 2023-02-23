#pragma once

#include <map>
#include <string>

class BSP;

typedef std::map<std::string, std::string> StringMap;

bool DumpObj( const char* filename, const char* mtlFilename, const BSP* bsp );
bool DumpMtl( const char* filename, const BSP* bsp, const StringMap& textureRemap );
bool DumpEnts( const char* filename, const BSP* bsp );

