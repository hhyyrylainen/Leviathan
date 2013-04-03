#ifndef LEVIATHAN_GEOMETRYADVANCEDLOADER
#define LEVIATHAN_GEOMETRYADVANCEDLOADER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "FileSystem.h"
#include "TimingMonitor.h"
//#include <Assimp/DefaultLogger.hpp>
//#include <Assimp/LogStream.hpp>
////#include <Assimp/Logger.hpp>
//#include <assimp/Importer.hpp> // C++ importer interface
//#include <assimp/scene.h> // Output data structure
//#include <assimp/postprocess.h> // Post processing flags


//#define ASSIMP_LOGGER_SEVERINITY_Debugging	1
//#define ASSIMP_LOGGER_SEVERINITY_Info		2 	
//#define ASSIMP_LOGGER_SEVERINITY_Warn		4
//#define ASSIMP_LOGGER_SEVERINITY_Err		8	


namespace Leviathan{
	//const unsigned int AssimpLogSeverenity = ASSIMP_LOGGER_SEVERINITY_Debugging|ASSIMP_LOGGER_SEVERINITY_Info|ASSIMP_LOGGER_SEVERINITY_Warn|
	//	ASSIMP_LOGGER_SEVERINITY_Err;

	//// class to get 
	//class ASSimpToLogBridge : public Assimp::LogStream {
	//public:
	//	// Constructor
	//	ASSimpToLogBridge();
	//	// Destructor
	//	~ASSimpToLogBridge();
	//	
	//	void write(const char* message);
	//};

	class GeometryAdvancedLoader : public Object{
	public:
		DLLEXPORT GeometryAdvancedLoader::GeometryAdvancedLoader();
		DLLEXPORT GeometryAdvancedLoader::~GeometryAdvancedLoader();

		DLLEXPORT bool ProcessFile(const wstring& file);

		DLLEXPORT void ProcessAllModels();

		//DLLEXPORT int ScanGeometryFile(const wstring& file);

	private:

	};

}
#endif