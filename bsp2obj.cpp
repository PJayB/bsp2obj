#include <id3bsp/BSP.h>
#include <id3bsp/Entities.h>
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
#include <tclap/CmdLine.h>

#include <missing.tga.h>

using namespace std;
using namespace id3bsp;

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
		string name = t.Name.data();

		// Skip nodraw shaders
		if (t.Flags & BSP::kSurfNoDraw) {
			continue;
		}
		
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
		else // redirect to missing.tga
			path = "missing.tga";

		remapping[t.Name.data()] = path;
	}
}

bool ExportTexture(const char* source, const char* destination)
{
	// If it already exists, don't do anything
	filesystem::path outputPath(destination);
	if (filesystem::exists(outputPath)) {
		return true;
	}

	// Make sure the output directory is present
	if (outputPath.has_parent_path()) {
		filesystem::create_directories(outputPath.parent_path());
	}

	// Output the file
    ofstream out(destination, ios::out | ios::binary);
    if (!out.is_open()) {
		return false;
	}

	// Read the entire input file
	vector<uint8_t> texData;
	if (ReadWholeBinaryFile(source, texData))
	{
    	out.write((const char*)&texData[0], texData.size());
	}
	else
	{
		// Failed to read it? Fall back to the "missing" texture
    	out.write(reinterpret_cast<const char*>(missing_tga), missing_tga_len);
	}

    out.close();

	return true;
}

bool ParseBSP( const char* bspFile, const char* objFile, FileListing& texturesToExport, int tesselationLevel, bool sanityCheck )
{
	vector<uint8_t> bspData;

	if ( !ReadWholeBinaryFile( bspFile, bspData ) )
	{
		cout << "FAILED" << endl; 
		return false;
	}

	BSP* bsp = BSP::Create( &bspData[0], bspData.size() );
	if ( !bsp )
	{
		cout << "FAILED" << endl;
		return false;
	}

	// Here just to sanity check the parser -- doesn't do anything
	if (sanityCheck) {
		std::vector<id3bsp::Entity> entities;
		id3bsp::Entity::Parse(bsp->EntityString, bspFile, entities);
	}

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
    if ( DumpObj( objFile, mtlFile.c_str(), bsp, tesselationLevel ) )
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
	std::string fsRoot;
	bool sanityCheck = false;
	int subdivisions = 16;
	std::vector<std::string> mapGlobs;
	try {
		TCLAP::CmdLine cmd("bsp2obj - convert idTech 3-era maps to OBJs", ' ',
			"0.3");

		auto fsRootArg = TCLAP::ValueArg<std::string>("g", "gamedata",
			"Game data path (or cwd is used)",
			false, "", "string");

		auto sanityCheckArg = TCLAP::ValueArg<bool>("c", "check",
			"Perform sanity checks on the BSP",
			false, false, "bool");

		auto subdivisionArg = TCLAP::ValueArg<int>("t", "tessellation",
			"Number of subdivisions for patch tessellation",
			false, 16, "int");

		auto patterns = TCLAP::UnlabeledMultiArg<std::string>("maps",
			"Maps to export, e.g. 'maps/q3*.bsp', or empty for all maps",
			false, "path/glob");

		cmd.add(fsRootArg);
		cmd.add(sanityCheckArg);
		cmd.add(subdivisionArg);
		cmd.add(patterns);
		cmd.parse(argc, argv);

		fsRoot = fsRootArg.getValue();
		sanityCheck = sanityCheckArg.getValue();
		subdivisions = subdivisionArg.getValue();
		mapGlobs = patterns.getValue();
	}
	catch (const TCLAP::ArgException& ex) {
		cerr << ex.argId() << ": " << ex.error() << endl;
		return EXIT_FAILURE;
	}

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

	const char* root = fsRoot.empty() ? cwd : fsRoot.c_str();
	if (!PHYSFS_mount(root, nullptr, 0)) {
		cerr << "Failed to mount " << root << endl;
		PHYSFS_deinit();
		return EXIT_FAILURE;
	}

	MountPakFiles();

	FileListing bspFiles;
	if (mapGlobs.empty()) {
		EnumerateFilesGlob("maps/*.bsp", [&] (const char* file) {
			bspFiles.insert(file);
		});
	} else {
		for (const auto& glob : mapGlobs) {
			EnumerateFilesGlob(glob, [&] (const char* file) {
				bspFiles.insert(file);
			});
		}
	}

	FileListing texturesToExport;

	int bspIndex = 0;
	for (auto& bspFile : bspFiles)
	{
		string objFile = BaseName( bspFile ) + ".obj";

		cout << "Converting map #" << ++bspIndex << " " << bspFile << " ... ";

		if ( !ParseBSP( bspFile.c_str(), objFile.c_str(), texturesToExport,
			subdivisions, sanityCheck ) ) {
			return EXIT_FAILURE;
		}
	}

	cout << "Exporting " << texturesToExport.size() << " textures..." << endl;

	size_t textureIndex = 0;
	size_t lastStrLen = 0;
	for (auto& t : texturesToExport)
	{
		size_t percent = (textureIndex * 100 / texturesToExport.size());

		auto thisStrLen = t.size() + 5;
		cout << percent << "%: " << t;
		for (auto i = thisStrLen; i < lastStrLen; ++i)
			cout << " ";
		cout << "\r";

		lastStrLen = thisStrLen;
		ExportTexture(t.c_str(), t.c_str());
	}

	cout << "Export complete." << endl;

	PHYSFS_deinit();
	return EXIT_SUCCESS;
}

