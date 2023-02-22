#include "BSP.h"
#include "OBJ.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <assert.h>
#include <fnmatch.h>
#include <string.h>
#include <unistd.h>

#include <physfs.h>

using namespace std;

#define VERBOSE( x )	

using FileListing = unordered_set<string>;

string BaseName(string in)
{
	// Find the location of the file and extension
	string tmp = in;
	string::size_type dotpos = tmp.find_last_of(".");
	string::size_type dirpos = tmp.find_last_of("/\\");
	
	// If there were no path slashes, start from the beginning
	if (dirpos == string::npos) { dirpos = 0; }
	// Otherwise start from the character after
	else { dirpos++; }
	
	// If there was no dot, read to the end
	// Ensure the dot comes before the slash
	if (dotpos == string::npos || dotpos < dirpos) 
	{ 
		dotpos = tmp.size(); 
	}
	
	// Strip the file and extension
	return tmp.substr(dirpos, dotpos - dirpos);
}

// Read a whole file. Free the blob afterwards.
bool ReadWholeBinaryFile(const char* fullpath, vector<uint8_t>& out)
{
	auto file = PHYSFS_openRead(fullpath);
	if ( !file )
	{
		return false;
	}

	// Get the size of the file
	auto size = PHYSFS_fileLength(file);
	if (size < 0) {
		return false;
	}

	out.resize(size);
	if (size > 0) {
		size = PHYSFS_readBytes(file, out.data(), size);

		// trim the vector if only partial data was read
		if (size >= 0) {
			out.resize(size);
		}
	}

	PHYSFS_close(file);
	return size >= 0;
}

template<typename TFunc> void EnumerateFiles(
	const char* path,
	TFunc&& func)
{
	char** files = PHYSFS_enumerateFiles(path);
	for (char** file = files; *file != nullptr; ++file) {
		func(*file);
	}
	PHYSFS_freeList(files);
}

template<typename TFunc> void EnumerateFilesGlob(
	const filesystem::path& path,
	const char* glob,
	TFunc&& func)
{
	EnumerateFiles(path.c_str(),
		[glob, &path, func = move(func)] (const char* file) {
			if (fnmatch(glob, file, FNM_CASEFOLD) == 0) {
				func((path / file).c_str());
			}
		});
}

// Note: globbing is on filename only, not on path
template<typename TFunc> void EnumerateFilesGlob(
	const filesystem::path& glob,
	TFunc&& func)
{
	filesystem::path path = glob.has_parent_path() ? glob.parent_path() : "/";
	EnumerateFilesGlob(path, glob.filename().c_str(), std::forward<TFunc>(func));
}

// Replace an extension
string ReplaceExtension(string in, const char* const extension)
{
	// Find the last .
	string::size_type dotPos = in.find_last_of(".");
	if (dotPos == string::npos)
	{
		return in + extension;
	}
	
	// Replace the string
	return in.substr(0, dotPos) + extension;
}

string Rebase(const string& oldBase, const string& newBase, string path)
{
	if ( strncmp(path.c_str(), oldBase.c_str(), oldBase.size()) == 0 )
		path = path.substr(oldBase.size());
	return newBase + path;
}


void RemapTextures( const BSP* bsp, StringMap& remapping )
{
	for (auto& t : bsp->Materials)
	{
		string name = t.Name;
		
		// Try and find variants - HACK - should really read these from shader files
		string targa = name + ".tga";
		string jpeg = name + ".jpg";
		string png = name + ".png";
		string& path = name;

		if ( PHYSFS_exists( targa.c_str() ) )
			path = targa;
		else if ( PHYSFS_exists( jpeg.c_str() ) )
			path = jpeg;
		else if ( PHYSFS_exists( png.c_str() ) ) 
			path = png;

		remapping[t.Name] = path;
	}
}

bool ExportTexture(const char* source, const char* destination)
{
	vector<uint8_t> texData;
	if (!ReadWholeBinaryFile(source, texData))
	{
		return false;
	}

	// Make sure the output directory is present
	PHYSFS_mkdir(destination);

	auto file = PHYSFS_openWrite(destination);
	if (!file)
	{
		return false;
	}

	PHYSFS_writeBytes(file, (const char*)&texData[0], texData.size());
	PHYSFS_close(file);

	return true;
}

bool ParseBSP( const char* bspFile, const char* objFile, FileListing& texturesToExport )
{
	vector<uint8_t> bspData;

	if ( !ReadWholeBinaryFile( bspFile, bspData ) )
	{
		cout << "FAILED" << endl; 
		return false;
	}

	VERBOSE( cout << "OK! " << bspData.size() << " bytes read." << endl );
	VERBOSE( cout << "Parsing ... " );

	BSP* bsp = BSP::Create( &bspData[0], bspData.size() );
	if ( !bsp )
	{
		cout << "FAILED" << endl;
		return false;
	}

	VERBOSE( cout << "OK!" << endl );
	VERBOSE( cout << "Summary:" << endl );

	VERBOSE( cout << " * " << bsp->Materials.size() << " Materials" << endl );
	VERBOSE( cout << " * " << bsp->Planes.size() << " Planes" << endl );
	VERBOSE( cout << " * " << bsp->Nodes.size() << " Nodes" << endl );
	VERBOSE( cout << " * " << bsp->Leaves.size() << " Leaves" << endl );
	VERBOSE( cout << " * " << bsp->LeafFaces.size() << " Leaf Faces" << endl );
	VERBOSE( cout << " * " << bsp->LeafBrushes.size() << " Leaf Brushes" << endl );
	VERBOSE( cout << " * " << bsp->Models.size() << " Models" << endl );
	VERBOSE( cout << " * " << bsp->Brushes.size() << " Brushes" << endl );
	VERBOSE( cout << " * " << bsp->BrushSides.size() << " Brush Sides" << endl );
	VERBOSE( cout << " * " << bsp->Vertices.size() << " Vertices" << endl );
	VERBOSE( cout << " * " << bsp->Indices.size() << " Indices" << endl );
	VERBOSE( cout << " * " << bsp->Fogs.size() << " Fogs" << endl );
	VERBOSE( cout << " * " << bsp->Faces.size() << " Faces" << endl );
	VERBOSE( cout << " * " << bsp->LightMaps.size() << " Light Maps" << endl );
	VERBOSE( cout << " * " << bsp->LightVolumes.size() << " Light Volumes" << endl );
	VERBOSE( cout << " * " << bsp->NumClusters << " Clusters" << endl );

	VERBOSE( cout << "Saving to " << objFile < " ... " );

	// Collect textures and find their real identities
	StringMap textureRemap;
	RemapTextures( bsp, textureRemap );

	for (auto& i : textureRemap)
	{
		// Add it to the export list
		texturesToExport.insert(i.second);
	}

	// Open the material definition file
	string mtlFile = ReplaceExtension( objFile, ".mtl" );
	string entFile = ReplaceExtension( objFile, "_entities.txt" );

    int result = 1;
    if ( DumpObj( objFile, mtlFile.c_str(), bsp ) )
    {
	    if ( DumpMtl( mtlFile.c_str(), bsp, textureRemap ) )
		{
			if ( DumpEnts( entFile.c_str(), bsp ) )
				result = 0;
		}
    }

    delete bsp;

    if ( result == 0 )
        cout << "OK!" << endl;

	return true;
}

static bool StringEndsWith(const char* str, const char* with, size_t withLen)
{
	size_t strLen = strlen(str);
	if (strLen < withLen) {
		return false;
	}

	return strcasecmp(str + (strLen - withLen), with) == 0;
}

void MountPakFiles()
{
	// Enumerate the pak files
	EnumerateFiles("/",
		[] (const char* file) {
			if (StringEndsWith(file, ".pk3", 4)) {
				filesystem::path path(PHYSFS_getRealDir(file));
				path /= file;

				if (!PHYSFS_mount(path.c_str(), nullptr, 0)) {
					cerr << "Warning: failed to mount " << file << endl;
				}
			}
		});
}

int main(int argc, char* argv[])
{
	if (!PHYSFS_init(argv[0])) {
		cerr << "Failed to init phsyfs" << endl;
		return EXIT_FAILURE;
	}

	auto cwd = getcwd(nullptr, 0);
	if (!PHYSFS_setWriteDir(cwd)) {
		cerr << "Failed to set the current write directory" << endl;
		PHYSFS_deinit();
		return EXIT_FAILURE;
	}

	const char* root = argc < 2 ? cwd : argv[1];
	if (!PHYSFS_mount(root, nullptr, 0)) {
		cerr << "Failed to mount " << root << endl;
		PHYSFS_deinit();
		return EXIT_FAILURE;
	}

	MountPakFiles();

	FileListing bspFiles;
	if (argc > 2) {
		for (int i = 2; i < argc; ++i) {
			EnumerateFilesGlob("/", argv[i], [&] (const char* file) {
				bspFiles.insert(file);
			});
		}
	} else {
		EnumerateFilesGlob("maps/*.bsp", [&] (const char* file) {
			bspFiles.insert(file);
		});
	}

	FileListing texturesToExport;

	int bspIndex = 0;
	for (auto& bspFile : bspFiles)
	{
		string objFile = BaseName( bspFile ) + ".obj";

		cout << "Converting map #" << ++bspIndex << " " << bspFile << " ... ";

		if ( !ParseBSP( bspFile.c_str(), objFile.c_str(), texturesToExport ) )
			return 1;
	}

	cout << "Exporting " << texturesToExport.size() << " textures..." << endl;

	size_t textureIndex = 0;
	for (auto& t : texturesToExport)
	{
		size_t percent = (textureIndex * 100 / texturesToExport.size());
		cout << percent << "%: " << t << "\r";

		ExportTexture(t.c_str(), t.c_str());
	}

	cout << "Export complete." << endl;

	PHYSFS_deinit();
	return EXIT_SUCCESS;
}

