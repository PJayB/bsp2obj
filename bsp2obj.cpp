#include "BSP.h"
#include "OBJ.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <assert.h>
#include <string.h>

using namespace std;

#define VERBOSE( x )	



string RemoveBase(const string& oldBase, string path)
{
	if ( strncmp(path.c_str(), oldBase.c_str(), oldBase.size()) == 0 )
		path = path.substr(oldBase.size());
	return path;
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
		string name = VFS::MakeFullyQualifiedFileName( t.Name );
		
		// Try and find variants - HACK - should really read these from shader files
		string targa = name + ".tga";
		string jpeg = name + ".jpg";
		string png = name + ".png";
		string& path = name;

		if ( VFS::FileExists( targa.c_str() ) )
			path = targa;
		else if ( VFS::FileExists( jpeg.c_str() ) )
			path = jpeg;
		else if ( VFS::FileExists( png.c_str() ) ) 
			path = png;

		remapping[t.Name] = RemoveBase( 
			VFS::GetRootDirectory(),
			path );
	}
}

bool ExportTexture(const char* source, const char* destination)
{
	vector<uint8_t> texData;
	if (!VFS::ReadWholeBinaryFile(source, texData))
	{
		return false;
	}

	// Make sure the output directory is present
	VFS::MakeNestedDirectories( VFS::BasePath( destination ).c_str() );

	ofstream out(destination, ios::out | ios::binary);
	if (!out.is_open())
	{
		return false;
	}

	out.write((const char*)&texData[0], texData.size());
	out.close();

	return true;
}

bool ParseBSP( const char* bspFile, const char* objFile, VFS::FileListing& texturesToExport )
{
	vector<uint8_t> bspData;

	if ( !VFS::ReadWholeBinaryFile( bspFile, bspData ) )
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

	// Make sure the output directory is present
	string objPath = VFS::BasePath( objFile );
	VFS::MakeNestedDirectories( objPath.c_str() );

	// Collect textures and find their real identities
	StringMap textureRemap;
	RemapTextures( bsp, textureRemap );

	for (auto& i : textureRemap)
	{
		// Add it to the export list
		texturesToExport.insert(i.second);
	}

	// Open the material definition file
	string mtlFile = VFS::ReplaceExtension( objFile, ".mtl" );
	string entFile = VFS::ReplaceExtension( objFile, "_entities.txt" );

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

void MountPakFiles()
{
	// Enumerate the pak files
	VFS::FileListing pakFiles;

	VFS::EnumerateFiles( 
		"/",
		[&] (const char* f) 
		{
			if ( strstr(f, ".pk3") != nullptr )
				pakFiles.insert( f );
		}, 
		true );

	for ( auto& f : pakFiles )
	{
		cout << "Mounting " << f << endl;

		VFS::AddZip( f.c_str() );
	}
}

int ExtractShaderSource(string& aggregatedShaderSource)
{
	int count = 0; 

	VFS::EnumerateFiles(
		"/base/shaders/",
		[&] (const char* f)
		{
			if ( strstr( f, ".shader" ) != nullptr )
			{
				VFS::ReadWholeTextFile( f, aggregatedShaderSource );
				count++;
			}
		});

	return count;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc < 2 )
		VFS::SetRootDirectory( _getcwd( NULL, 0 ) );
	else
		VFS::SetRootDirectory( argv[1] );

	MountPakFiles();

	int totalFiles = 0;
	VFS::FileListing bspFiles;
	VFS::EnumerateFiles(
		"/",
		[&] (const char* f) 
		{
			if ( strstr(f, ".bsp") != nullptr )
				bspFiles.insert( f );
			totalFiles++;
		});

	cout << "Found " << bspFiles.size() << " BSP files in " << totalFiles << " game files." << endl;

	if ( !bspFiles.size() )
		return 0;

	/*
	string aggregatedShaderSource;
	int shaderCount = ExtractShaderSource(aggregatedShaderSource);
	cout << "Read " << aggregatedShaderSource.size() << " bytes in " << shaderCount << " shaders." << endl;

	ShaderDB shaderDB;
	if ( !shaderDB.Parse( aggregatedShaderSource ) )
		cout << "WARNING: failed to parse shaders! You may be missing some textures." << endl;
	*/
	//return 0;


	string basePath = VFS::GetRootDirectory();
	string outputPath = "output/";

	_mkdir( outputPath.c_str() );


	/*
	string shaderDumpFile = outputPath + "shaderDump.txt";
	ofstream shaderDump( shaderDumpFile.c_str() );
	if ( shaderDump.is_open() )
	{
		shaderDump.write(aggregatedShaderSource.c_str(), aggregatedShaderSource.size());
		shaderDump.close();
	}
	*/


	VFS::FileListing texturesToExport;

	int bspIndex = 0;
	for (auto& bspFile : bspFiles)
	{
		string objFile = outputPath + VFS::BaseName( bspFile ) + ".obj";

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

		string dest = Rebase(basePath, outputPath, t);
		ExportTexture(t.c_str(), dest.c_str());
	}

	cout << "Export complete." << endl;

	return 0;
}

