#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GEOMETRYADVANCEDLOADER
#include "GeometryAdvancedLoader.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Engine.h"

Leviathan::GeometryAdvancedLoader::GeometryAdvancedLoader(){

	// set logger to be used for Assimp //
	//Assimp::DefaultLogger::get()->attachStream( new ASSimpToLogBridge(), AssimpLogSeverenity);
}
Leviathan::GeometryAdvancedLoader::~GeometryAdvancedLoader(){

}
// ------------------------------------ //

// ------------------------------------ //
void Leviathan::GeometryAdvancedLoader::ProcessAllModels(){

//	//// loop through model files and if valid check do proper files exist, if not generate //
//	//const vector<shared_ptr<FileDefinitionType>>& MdlFiles = FileSystem::GetModelFiles();
//
//	//// create ids //
//	//vector<int> LookForIDS;
//	//wstring lookfor = L"dae|obj|ms3d|ply|3ds|blend";
//
//	//FileSystem::GetExtensionIDS(lookfor, LookForIDS);
//
//	//wstring validresultexts = L"levmd";
//
//
//	//for(unsigned int i = 0; i < MdlFiles.size(); i++){
//	//	// check if it's extension is one of the ones that we are looking for //
//	//	if(FileSystem::DoesExtensionMatch(MdlFiles[i].get(), LookForIDS)){
//	//		// check does a matching proper file exist //
//	//		wstring checkpath = FileSystem::SearchForFile(FILEGROUP_MODEL, MdlFiles[i]->Name, validresultexts, false);
//	//		if(checkpath == L""){
//	//			// not found call generation //
//	//			if(!ProcessFile(MdlFiles[i]->RelativePath)){
//	//				Logger::Get()->Error(L"GeometryAdvancedLoader: ProcessAllModels: Could not process "+MdlFiles[i]->RelativePath+
//	//					L" into a valid model, MODEL MIGHT BE MISSING");
//	//				continue;
//	//			}
//	//		}
//	//	}
//	//}
//
//
//}
//// ------------------------------------ //
//bool Leviathan::GeometryAdvancedLoader::ProcessFile(const wstring& file){
//
//	TimingMonitor::StartTiming(L"GeometryLoaderFileProcess");
//
//	// create importer //
//	Importer importer;
//
//	// read a scene //
//	const aiScene* scene = importer.ReadFile(Convert::WstringToString(file),  aiProcess_CalcTangentSpace | aiProcess_Triangulate |
//		aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_LimitBoneWeights |
//		aiProcess_FindInvalidData | 
//		aiProcess_ConvertToLeftHanded );
//	// check for failure //
//	if(!scene){
//
//		Logger::Get()->Error(L"GeometryAdvancedLoader: ProcessFile: failed error: "+Convert::StringToWstring(importer.GetErrorString()));
//		TimingMonitor::StopTiming(L"GeometryLoaderFileProcess", false);
//		return false;
//	}
//
//	// temporary dirty hack to see how it works //
//
//	vector<GameObject::Model*> temp = Engine::GetEngine()->GetObjectLoader()->LoadModelAssIScene(scene);
//	for(unsigned int i = 0; i < temp.size(); i++){
//		if(temp[i] != NULL)
//			Engine::GetEngine()->AddObject(dynamic_cast<BaseObject *>(temp[i]));
//	}
//
//	// scan scene object //
//
//
//	//// generate texture data //
//	//wstring texturedata = L"";
//	//for(unsigned int i = 0; i < scene->mNumMaterials; i++){
//	//	aiMaterial* currentmat = scene->mMaterials[i];
//
//	//	wstring texturetype = L"NormalMap";
//	//	// put type //
//	//	texturedata += L"			<t> "+texturetype+L";\n";
//	//	// file name + ext
//	//	texturedata += L"			<t> "+currentmat->+L";\n";
//	//	currentmat->
//	//}
//
//
//	// and now the impossible part, write proper data to a levmd file //
//
//	wstring out = FileSystem::ChangeExtension(file, L"levmd");
//
//	wofstream writer;
//	writer.open(out);
//	if(!writer.good()){
//		// something went wrong //
//
//		Logger::Get()->Error(L"GeometryAdvancedLoader: ProcessFile: cant create compiled model FILE "+out+L" CANNOT BE OPENED FOR WRITING", 
//			GetLastError());
//		TimingMonitor::StopTiming(L"GeometryLoaderFileProcess", false);
//		return false;
//	}
//
//	writer << L"TODO: write proper writer for this, GeometryAdvancedLoader 100";
//
//	writer.close();
//
//	TimingMonitor::StopTiming(L"GeometryLoaderFileProcess");
//	return false;
}
// ---------------- ASSimpToLogBridge -------------------- //
//void Leviathan::ASSimpToLogBridge::write(const char* message){
//	// just calling log to write here //
//	Logger::Get()->Write(Convert::StringToWstring(message));
//}
//
//Leviathan::ASSimpToLogBridge::~ASSimpToLogBridge(){
//	// empty
//}
//
//Leviathan::ASSimpToLogBridge::ASSimpToLogBridge(){
//	// empty
//}
