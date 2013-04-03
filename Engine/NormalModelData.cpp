#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NORMALMODELDATA
#include "NormalModelData.h"
#endif
using namespace Leviathan;
using namespace Leviathan::GameObject;
// ------------------------------------ //
#include "ObjectFileProcessor.h"
#include "TriangulateDelaunay.h"

DLLEXPORT Leviathan::GameObject::NormalModelData::NormalModelData() : pRenderModel(), ModelFaceData(){
	Type = MODELOBJECT_MODEL_TYPE_NONLOADED;
	IndexBuffer = NULL;
	VertexBuffer = NULL;
	Skeleton = NULL;
	IsVertexGroupsUsed = false;
}

DLLEXPORT Leviathan::GameObject::NormalModelData::~NormalModelData(){
	ReleaseModel();

}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::NormalModelData::ReleaseModel(){
	SAFE_DELETE_VECTOR(pRenderModel);
	SAFE_DELETE_VECTOR(ModelFaceData);
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(VertexBuffer);
	SAFE_RELEASEDEL(Skeleton);
	Type = MODELOBJECT_MODEL_TYPE_NONLOADED;
}

DLLEXPORT bool Leviathan::GameObject::NormalModelData::InitBuffers(ID3D11Device* device){
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT hr = S_OK;

	// create definition //
	D3D11_BUFFER_DESC VertexBufferdesc;
	VertexBufferdesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferdesc.CPUAccessFlags = 0;
	VertexBufferdesc.MiscFlags = 0;
	VertexBufferdesc.StructureByteStride = 0;

	ModelVertexType* basicvertices = NULL;
	ModelGroupedVertex* groupedvertices = NULL;

	// create index array //
	unsigned long* indices;
	indices = new unsigned long[IndexCount];

	// create vertex array //
	if(!IsVertexGroupsUsed){
		// allocate //
		basicvertices = new ModelVertexType[VertexCount];

		// load arrays with data //
		for(int i = 0; i < VertexCount; i++){

			basicvertices[i].position = D3DXVECTOR3(pRenderModel[i]->x, pRenderModel[i]->y, pRenderModel[i]->z);
			basicvertices[i].texture = D3DXVECTOR2(pRenderModel[i]->tu, pRenderModel[i]->tv);
			basicvertices[i].normal = D3DXVECTOR3(pRenderModel[i]->nx, pRenderModel[i]->ny, pRenderModel[i]->nz);

			indices[i] = i;

		}
		// set buffer descs //
		VertexBufferdesc.ByteWidth = sizeof(ModelVertexType) * VertexCount;
	} else {

		// allocate memory //
		groupedvertices = new ModelGroupedVertex[VertexCount];
		

		//FileSystem::WriteToFile(L"", L"Tmp.txt");

		// load arrays with data //
		for(int i = 0; i < VertexCount; i++){

			groupedvertices[i].position = D3DXVECTOR3(pRenderModel[i]->x, pRenderModel[i]->y, pRenderModel[i]->z);
			float tu = pRenderModel[i]->tu;
			float tv = pRenderModel[i]->tv;
			groupedvertices[i].texture = D3DXVECTOR2(tu, tv);
			groupedvertices[i].normal = D3DXVECTOR3(pRenderModel[i]->nx, pRenderModel[i]->ny, pRenderModel[i]->nz);

			// set groups //
			NormalModelVertexTypeVertexGroups* tmp = dynamic_cast<NormalModelVertexTypeVertexGroups*>(pRenderModel[i]);
			// copy data over //
			groupedvertices[i].VertexGroups = tmp->VertexGroups;
			groupedvertices[i].VertexGroupWeights = tmp->VertexGroupWeights;
			//groupedvertices[i].VertexGroupWeights = Float4(0,0,0,0);
			
			//groupedvertices[i].VertexGroupWeights = D3DXVECTOR4(tmp->VertexGroupWeights[0], tmp->VertexGroupWeights[1], tmp->VertexGroupWeights[2], tmp->VertexGroupWeights[3]);
			//groupedvertices[i].VertexGroupWeights = D3DXVECTOR3(tmp->VertexGroupWeights[0], tmp->VertexGroupWeights[1], tmp->VertexGroupWeights[2]);

			indices[i] = i;
			//// output it //
			//wstringstream streamer;
			//streamer << groupedvertices[i].position.x << L" " << groupedvertices[i].position.y << L" " << groupedvertices[i].position.z << L" " << 
			//	groupedvertices[i].texture.x << L" " << groupedvertices[i].texture.y << L" " <<
			//	groupedvertices[i].normal.x << L" " << groupedvertices[i].normal.y << L" " << groupedvertices[i].normal.z << endl;
			//FileSystem::AppendToFile(streamer.str(), L"Tmp.txt");
		}
		// set buffer descs //
		VertexBufferdesc.ByteWidth = sizeof(ModelGroupedVertex) * VertexCount;
	}
	

	// Give the sub resource structure a pointer to the vertex data.
	if(!IsVertexGroupsUsed){
		vertexData.pSysMem = basicvertices;
	} else {
		vertexData.pSysMem = groupedvertices;
	}
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// create vertex buffer //
	hr = device->CreateBuffer(&VertexBufferdesc, &vertexData, &VertexBuffer);
	if(FAILED(hr)){

		Logger::Get()->Error(L"Failed to init RenderModelhandler buffers, create vertex buffer failed",0);
		return false;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC IndexBufferdesc;
	IndexBufferdesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferdesc.ByteWidth = sizeof(unsigned long) * IndexCount;
	IndexBufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferdesc.CPUAccessFlags = 0;
	IndexBufferdesc.MiscFlags = 0;
	IndexBufferdesc.StructureByteStride = 0;

	// Give the sub resource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// create index buff //
	hr = device->CreateBuffer(&IndexBufferdesc, &indexData, &IndexBuffer);
	if(FAILED(hr))
	{
		Logger::Get()->Error(L"Failed to init RenderModelhandler buffers, create index buffer failed",0);
		return false;
	}

	SAFE_DELETE_ARRAY(basicvertices);
	SAFE_DELETE_ARRAY(indices);

	IsBufferDone = true;
	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameObject::NormalModelData::RenderBuffers(ID3D11DeviceContext* devcont){

	// important set right stride size //
	unsigned int stride;
	// Set vertex buffer stride and offset.
	if(IsVertexGroupsUsed){
		stride = sizeof(ModelGroupedVertex);
	} else {
		stride = sizeof(ModelVertexType);
	}
	
	unsigned int offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devcont->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	devcont->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	devcont->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameObject::NormalModelData::LoadRenderModel(wstring* file){

#ifdef _DEBUG
	wstring timername = L"NormalModelData: loading model data "+*file;
	TimingMonitor::StartTiming(timername);
#endif // _DEBUG
	// file type check //
	wstring filetype = FileSystem::GetExtension(*file);
	if(filetype == L"obj"){
		bool result = LoadFromOBJ(file);
#ifdef _DEBUG
		TimingMonitor::StopTiming(timername, true);
#endif // _DEBUG
		return result;
	}
	if(filetype == L"levmo"){
		bool result = LoadFromLEVMO(file);
#ifdef _DEBUG
		TimingMonitor::StopTiming(timername, true);
#endif // _DEBUG
		return result;
	}
	Logger::Get()->Info(L"Error Typeless model: "+*file);
#ifdef _DEBUG
	TimingMonitor::StopTiming(timername, true);
#endif // _DEBUG
	return false;
}

DLLEXPORT bool Leviathan::GameObject::NormalModelData::LoadFromLEVMO(wstring* file){
	vector<shared_ptr<NamedVar>> HeadVars;

	vector<ObjectFileObject*> FileStructure = ObjectFileProcessor::ProcessObjectFile(*file, HeadVars);

	int head_int = 0;
	wstring head_str = L"";

	bool NeedToChangeCoordinateSystem = false;
	bool IsUnCompiled = false;

	int ExpectedVertexCount = -1;
	int ExpectedFaceCount = -1;

	// check some values in header //
	for(unsigned int i = 0; i < HeadVars.size(); i++){
		if(HeadVars[i]->GetName() == L"FileType"){
			// get values //
			HeadVars[i]->GetValue(head_int, head_str);
			// checks/storing //
			if(head_str != L"ModelData"){
				Logger::Get()->Error(L"NormalModelData: load from levmo: invalid FileType, ModelData expected got: "+head_str, true);
				return false;
			}
			continue;
		}
		if(HeadVars[i]->GetName() == L"Model-Name"){
			// get values //
			HeadVars[i]->GetValue(head_int, head_str);
			// checks/storing //
			ThisName = head_str;
			continue;
		}
		if(HeadVars[i]->GetName() == L"CoordinateSystem"){
			// get values //
			HeadVars[i]->GetValue(head_int, head_str);
			// checks/storing //
			if(head_str != L"LEVIATHAN"){
				// needs to transform //
				NeedToChangeCoordinateSystem = true;
			}
			continue;
		}
		if(HeadVars[i]->GetName() == L"Model-UnCompiled"){
			// get values //
			HeadVars[i]->GetValue(head_int, head_str);
			// checks/storing //
			if(head_int != 0){
				// compilation needed //
				IsUnCompiled = true;
			}
			continue;
		}
		if(HeadVars[i]->GetName() == L"ExpectedVertexCount"){
			// get values //
			HeadVars[i]->GetValue(head_int, head_str);
			// checks/storing //
			ExpectedVertexCount = head_int;
			continue;
		}
		if(HeadVars[i]->GetName() == L"ExpectedFaceCount"){
			// get values //
			HeadVars[i]->GetValue(head_int, head_str);
			// checks/storing //
			ExpectedFaceCount = head_int;
			continue;
		}
	}


	// lets start some processing //
	for(unsigned int i = 0; i < FileStructure.size(); i++){
		if(FileStructure[i]->TName == L"ModelData"){
			// data //
			vector<Float3*> Vertices;

			// vertice weights //
			int LinesChecked = 0;
			bool IsVertexGroupsPresent = false;
			vector<vector<CombinedClass<Int1, Float1>*>> VerticeWeights;

			// armature loading //


			//vector<Float3*> VertexNormals;



			vector<LoadingFace*> Faces;

			// reserve space for data, if known //
			if(ExpectedFaceCount > 0){
				// reserving //
				Faces.reserve((unsigned int)ExpectedFaceCount);
			}
			if(ExpectedVertexCount > 0){
				// reserving //
				Vertices.reserve((unsigned int)ExpectedVertexCount);
			}
			
			bool VertexGroupDataReserved = false;

			// loop through all structures //
			//for(unsigned int a = 0; a < FileStructure[i]->Contents.size(); a++){
			//	// handle based on type //
			//	if(FileStructure[i]->Contents[a]->Name == L"VertexCoordinates")
			//}
			for(unsigned int a = 0; a < FileStructure[i]->TextBlocks.size(); a++){
				// handle on name //
				if(FileStructure[i]->TextBlocks[a]->Name == L"VertexCoordinates"){
					// load vertices //
					for(unsigned int e = 0; e < FileStructure[i]->TextBlocks[a]->Lines.size(); e++){
						wstring line = *FileStructure[i]->TextBlocks[a]->Lines[e];
						//// trim beginning and end //
						////ObjectFileProcessor::TrimLineSpaces(&line);

						// check does line contain group data //
						if(!(!IsVertexGroupsPresent && LinesChecked > 5)){
							if(Misc::CountOccuranceWstring(line, L"groups")){
								IsVertexGroupsPresent = true;

								if(!VertexGroupDataReserved){
									// reserve data in huge block //
									VertexGroupDataReserved = true;

								}

								// this needs to be tokenized for parsing //
								vector<wstring*> Tokenizedline;
								LineTokeNizer::TokeNizeLine(line, Tokenizedline);

								// check token count //
								if(Tokenizedline.size() != 4){
									// invalid line //
									Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: while parsing VertexCoordinates invalid line found: \""+line
										+L"\" token parser returned invalid number of tokens");
									continue;
								}
								// get x,y,z values //
								float x(Convert::WstringToFloat(*Tokenizedline[0])),y(Convert::WstringToFloat(*Tokenizedline[1])),z(Convert::WstringToFloat(*Tokenizedline[2]));

								// parse group data //
								vector<Token*> RealTokens;
								LineTokeNizer::SplitTokenToRTokens(*Tokenizedline[3], RealTokens);

								VerticeWeights.push_back(vector<CombinedClass<Int1, Float1>*>());

								// parse vertex groups //
								if(RealTokens.size() > 0){
									// reserve space //
									VerticeWeights.back().reserve(RealTokens.size()-2);
									for(unsigned int splitindex = 0; splitindex < RealTokens.size(); splitindex++){
										Token* toprocess = RealTokens[splitindex];

										// check for data //
										if((std::string::npos != toprocess->GetData().find(L',')) 
											&& (Misc::WstringContainsNumbers(toprocess->GetData())))
										{
											int vertexgroup(0);
											float weight(0);

											wstringstream streamer(toprocess->GetData());
											streamer >> vertexgroup;
											// skip a ', ' char //
											streamer.seekg(streamer.tellg()+std::streamoff(2));
											streamer >> weight;

											// set data //
											CombinedClass<Int1, Float1>* data = new CombinedClass<Int1, Float1>();
											data->SetIntValue(vertexgroup);
											data->SetFloatValue(weight);
											// push value //
											VerticeWeights.back().push_back(data);
										}
									}
								} else {
									// complain once //
									//if(DataStore::Get()->AddValueIfDoesntExist(L"Dont_report_missing_groups", 1)){
									//	// value wasn't there, complain //
									//	Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: split tokens returned nothing, MISSING GROUPS!", true);
									//}
									ComplainOnce::PrintWarningOnce(L"Dont_report_missing_groups_for"+*file, 
										L"NormalModelData: LoadFromLEVMO: split tokens returned nothing, MISSING GROUPS! File: "+*file);
								}

								// store //
								if(NeedToChangeCoordinateSystem){
									// swap y and z to convert from blender coordinates to work with  //
									swap(y,z);
								}
								Vertices.push_back(new Float3(x, y, z));

								// remember to release memory //
								SAFE_DELETE_VECTOR(Tokenizedline);
								SAFE_DELETE_VECTOR(RealTokens);
								continue;
							}
						}
						// normal line, no groups( thing //
						LinesChecked++;
						// use stringstream to get floats out of this line //
						wstringstream streamer(line);
						float x,y,z = 0;
						streamer >> x >> y >> z;
						// store //
						if(NeedToChangeCoordinateSystem){
							// swap y and z to convert from blender coordinates to work with  //

							swap(y,z);
						}
						Vertices.push_back(new Float3(x, y, z));

						if(IsVertexGroupsPresent){
							// needs to "fake" vertex groups //
							VerticeWeights.push_back(vector<CombinedClass<Int1, Float1>*>());
							// set fake data //
							CombinedClass<Int1, Float1>* data = new CombinedClass<Int1, Float1>();
							data->SetIntValue(0);
							data->SetFloatValue(0.f);
							VerticeWeights.back().push_back(data);
						}
					}
					continue;
				}
				//if(FileStructure[i]->TextBlocks[a]->Name == L"VertexNormals"){
				//	// load vertices //
				//	for(unsigned int e = 0; e < FileStructure[i]->TextBlocks[a]->Lines.size(); e++){
				//		wstring line = *FileStructure[i]->TextBlocks[a]->Lines[e];
				//		//// trim beginning and end //
				//		////ObjectFileProcessor::TrimLineSpaces(&line);

				//		// use stringstream to get floats out of this line //
				//		wstringstream streamer(line);
				//		float x,y,z = 0;
				//		streamer >> x >> y >> z;
				//		// store //
				//		if(NeedToChangeCoordinateSystem){
				//			// flip z to change handedness //
				//			//z *= -1;
				//			// UMMMMMM !!!!
				//		}
				//		VertexNormals.push_back(new Float3(x, y, z));
				//		//if(needtochangecoordinatesystem){
				//		//	// inverse normal //
				//		//	//vertexnormals.back
				//		//	d3dxvector3 tempmult = (*vertexnormals.back())*2;
				//		//	d3dxvec3subtract(vertexnormals.back(), vertexnormals.back(), &tempmult);

				//		//}
				//	}
				//}
				if(FileStructure[i]->TextBlocks[a]->Name == L"Faces"){
					// load vertices //
					for(unsigned int e = 0; e < FileStructure[i]->TextBlocks[a]->Lines.size(); e++){
						// add a face //
						LoadingFace* CurrentFace = new LoadingFace();

						// tokenize the string //
						vector<wstring*> Tokens;
						int error = LineTokeNizer::TokeNizeLine(*FileStructure[i]->TextBlocks[a]->Lines[e], Tokens);

						for(unsigned int ind = 0; ind < Tokens.size(); ind++){
							// check for uv coordinate //
							if(Misc::WstringStartsWith(*Tokens[ind], L"uv")){
								// uv coordinate //
								// split to values //
								// check for NULL //
								if(Misc::CountOccuranceWstring(*Tokens[ind], L"NULL") > 0){
									// no UVs
									continue;
								}

								vector<wstring> Parts;
								error = LineTokeNizer::SplitTokenToValues(*Tokens[ind], Parts);
								if(Parts.size() != 2){
									// invalid //
									QUICK_ERROR_MESSAGE;
									continue;
								}
								// create it //
								Float2 UV;
								if(!NeedToChangeCoordinateSystem){
									UV.Val[0] = Convert::WstringToFloat(Parts[0]);
									UV.Val[1] = Convert::WstringToFloat(Parts[1]);
								} else {
								//	UV.Val[0] = Convert::WstringToFloat(Parts[0]);
								//	UV.Val[1] = 1.f-Convert::WstringToFloat(Parts[1]);
									UV.Val[0] = Convert::WstringToFloat(Parts[0]);
									UV.Val[1] = Convert::WstringToFloat(Parts[1]);
									bool Adjusted = false;
									// clamp indexes into 0-1 range //
									if(UV.GetY() < 0){
										// don't allow negative Y's //
										Adjusted = true;
										UV.SetY(-1.f*UV.GetY());
									}
									if(UV.GetY() > 1){
										float remainder = fmodf(UV.GetY(), 1.f);
										if(remainder == 0.f){
											// even //
											Adjusted = true;
											UV.SetY(1.f);
										} else {
											// needs to do more advanced adjustment //
											Adjusted = true;
											// the remainder might work as the new value //
											UV.SetY(remainder);
										}

										if(Adjusted){
											// report //
											DEBUG_BREAK;
											ComplainOnce::PrintWarningOnce(*file+L"_report_uv_adjust", 
												L"NormalModelData: LoadFromLEVMO: File has UVs that are not in range 0-1 adjusting done, file: "+*file);

										}
									}

									UV.Val[1] = 1.f-UV.Val[1];
								}

								// push //
								CurrentFace->UVs.push_back(UV);
								continue;
							}
							// is a number //
							int VertIndex = Convert::WstringToInt(*Tokens[ind]);
							CurrentFace->VertexIDS.push_back(VertIndex);
							continue;
						}
						// delete tokens //
						SAFE_DELETE_VECTOR(Tokens);

						// add final //
						Faces.push_back(CurrentFace);
					}
					continue;
				}
				if(FileStructure[i]->TextBlocks[a]->Name == L"Bones"){
					// load bone stucture from file //
					Skeleton = SkeletonRig::LoadRigFromFileStructure(FileStructure[i]->TextBlocks[a], NeedToChangeCoordinateSystem);
					if(!Skeleton){
						// is still NULL //

						Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: Invalid structure for bones, not using bones");
						continue;
					}

					// model is using bones //
					IsVertexGroupsUsed = true;
					continue;
				}
			}

			// check for data validness and convert from coordinate system if required //

			// check faces and triangulate if needed //
			if(IsUnCompiled){
				for(unsigned int a = 0; a < Faces.size(); a++){
					// check vertice count //
					if(Faces[a]->VertexIDS.size() > 3){
						// needs triangulation //
						throw exception("this shit needs a rewrite");



					}
					if(NeedToChangeCoordinateSystem){
						// flip first and second point to flip the face from counter clockwise to clockwise //
						//int tempid;
						//tempid = Faces[a]->VertexIDS[2];
						//Faces[a]->VertexIDS[2] = Faces[a]->VertexIDS[1];
						//Faces[a]->VertexIDS[1] = tempid;

						swap(Faces[a]->VertexIDS[2], Faces[a]->VertexIDS[1]);

						if(Faces[a]->UVs.size() == 3){
							//Float2 tmpuv = Faces[a]->UVs[2];
							//Faces[a]->UVs[2] = Faces[a]->UVs[1];
							//Faces[a]->UVs[1] = tmpuv;
							swap(Faces[a]->UVs[2], Faces[a]->UVs[1]);
						}
					}
				}
			}


			// create render model //
			VertexCount = Faces.size()*3;
			IndexCount = VertexCount;

			//IsVertexGroupsUsed = false;

			// Create the model using the vertex count that was read in.
			pRenderModel.reserve(VertexCount);
			// fill it //
			for(int ind = 0; ind < VertexCount; ind++){
				// create new //
				if(IsVertexGroupsUsed){
					// needs to have space for vertex group //
					pRenderModel.push_back(new NormalModelVertexTypeVertexGroups());
				} else {
					pRenderModel.push_back(new NormalModelVertexType());
				}
				
				if(!pRenderModel[ind]){
					// failed to get memory //
					QUICK_MEMORY_ERROR_MESSAGE;
					// cleanup //
					SAFE_DELETE_VECTOR(pRenderModel);
					SAFE_DELETE_VECTOR(Vertices);
					SAFE_DELETE_VECTOR(Faces);
					for(unsigned int delind = 0; delind < VerticeWeights.size(); delind++){
						SAFE_DELETE_VECTOR(VerticeWeights[delind]);
					}
					break;
				}
			}

			// Load in vertex data
			int fileverticeindex = 0;
			int facecount = Faces.size();
			//unsigned int vertexgroupindex = 0;
			for(int ind = 0; ind < facecount; ind++){

				// re calculate normals //
				LoadingFace* CurrentFace = Faces[ind];
				//if(IsUnCompiled){
				Float3 facenormal = Float3(((*Vertices[CurrentFace->VertexIDS[1]])-(*Vertices[CurrentFace->VertexIDS[0]]))
					.Cross((*Vertices[CurrentFace->VertexIDS[2]])-(*Vertices[CurrentFace->VertexIDS[0]])));

				// set normals //
				pRenderModel[fileverticeindex]->nx = facenormal.X();
				pRenderModel[fileverticeindex]->ny = facenormal.Y();
				pRenderModel[fileverticeindex]->nz = facenormal.Z();

				pRenderModel[fileverticeindex+1]->nx = facenormal.X();
				pRenderModel[fileverticeindex+1]->ny = facenormal.Y();
				pRenderModel[fileverticeindex+1]->nz = facenormal.Z();

				pRenderModel[fileverticeindex+2]->nx = facenormal.X();
				pRenderModel[fileverticeindex+2]->ny = facenormal.Y();
				pRenderModel[fileverticeindex+2]->nz = facenormal.Z();

				// loop through vertices in face //
				for(unsigned int VerticeIndexNumber = 0; VerticeIndexNumber < 3; VerticeIndexNumber++){
					// just loop 3 times

					// get vertice index //
					int VertIndex = CurrentFace->VertexIDS[VerticeIndexNumber];


					if(IsVertexGroupsUsed){
						// needs to add data about vertex groups //
						// should have same index as vertices //
						vector<CombinedClass<Int1, Float1>*> &curweights = VerticeWeights[VertIndex];


						NormalModelVertexTypeVertexGroups* tmp = dynamic_cast<NormalModelVertexTypeVertexGroups*>(pRenderModel[fileverticeindex]);
						if(tmp == NULL){

							Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: File is inconsistent, doesn't contain vertex groups, but requires them");
							DEBUG_BREAK;
							return false;
						}

						if(curweights.size() > 0){
							tmp->VertexGroups[0] = curweights[0]->iVal;
							tmp->VertexGroupWeights[0] = curweights[0]->fVal;
						} else {
							// empty vertex groups //
							tmp->VertexGroups[0] = 0;
							tmp->VertexGroupWeights[0] = 0.f;
						}
						if(curweights.size() > 1){
							tmp->VertexGroups[1] = curweights[1]->iVal;
							tmp->VertexGroupWeights[1] = curweights[1]->fVal;
						} else {
							// empty vertex groups //
							tmp->VertexGroups[1] = 0;
							tmp->VertexGroupWeights[1] = 0.f;
						}
						if(curweights.size() > 2){
							tmp->VertexGroups[2] = curweights[2]->iVal;
							tmp->VertexGroupWeights[2] = curweights[2]->fVal;
						} else {
							// empty vertex groups //
							tmp->VertexGroups[2] = 0;
							tmp->VertexGroupWeights[2] = 0.f;
						}
						if(curweights.size() > 3){
							tmp->VertexGroups[3] = curweights[3]->iVal;
							tmp->VertexGroupWeights[3] = curweights[3]->fVal;
						} else {
							// empty vertex groups //
							tmp->VertexGroups[3] = 0;
							tmp->VertexGroupWeights[3] = 0.f;
						}
					}
					// set vertice (1-3) //


					pRenderModel[fileverticeindex]->x = Vertices[VertIndex]->X();
					pRenderModel[fileverticeindex]->y = Vertices[VertIndex]->Y();
					pRenderModel[fileverticeindex]->z = Vertices[VertIndex]->Z();
					// texture coordinates //
					if(CurrentFace->UVs.size() > VerticeIndexNumber){
						pRenderModel[fileverticeindex]->tu = CurrentFace->UVs[VerticeIndexNumber].GetX();
						pRenderModel[fileverticeindex]->tv = CurrentFace->UVs[VerticeIndexNumber].GetY();
					} else {
						pRenderModel[fileverticeindex]->tu = 0;
						pRenderModel[fileverticeindex]->tv = 0;
					}
					// change index to next address //
					fileverticeindex++;
				}
				//// increment group here //
				//vertexgroupindex++;
			}


			// this function doesn't work, needs to be done here //
			//if(IsUnCompiled){
			//	// save on top of the uncompiled file //
			//	//if(!WriteToFile(*file, false)){
			//	if(!WriteToFile(FileSystem::RemoveExtension(*file, false)+L"_test.levmo", false)){
			//		Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: failed to save file on top of the original");
			//	}
			//}
			if(IsUnCompiled){
				vector<shared_ptr<NamedVar>> SavedHeadVars;

				// header data //
				SavedHeadVars.push_back(shared_ptr<NamedVar>(new NamedVar(L"FileType", L"ModelData")));
				SavedHeadVars.push_back(shared_ptr<NamedVar>(new NamedVar(L"Model-Name", L'\"'+ThisName+L'\"')));
				SavedHeadVars.push_back(shared_ptr<NamedVar>(new NamedVar(L"CoordinateSystem", L"LEVIATHAN")));
				// set data //
				ExpectedVertexCount = Vertices.size();
				ExpectedFaceCount = Faces.size();

				SavedHeadVars.push_back(shared_ptr<NamedVar>(new NamedVar(L"ExpectedVertexCount", ExpectedVertexCount)));
				SavedHeadVars.push_back(shared_ptr<NamedVar>(new NamedVar(L"ExpectedFaceCount", ExpectedFaceCount)));

				// write to blocks //
				wstringstream converter;

				ObjectFileTextBlock* vertexblock = NULL;
				// search for vertex block //
				for(unsigned int ind = 0; ind < FileStructure[i]->TextBlocks.size(); ind++){
					if(FileStructure[i]->TextBlocks[ind]->Name == L"VertexCoordinates"){
						vertexblock = FileStructure[i]->TextBlocks[ind];
						break;
					}
				}

				// search for faces //
				ObjectFileTextBlock* faceblock = NULL;
				for(unsigned int ind = 0; ind < FileStructure[i]->TextBlocks.size(); ind++){
					if(FileStructure[i]->TextBlocks[ind]->Name == L"Faces"){
						faceblock = FileStructure[i]->TextBlocks[ind];
						break;
					}
				}

				// search for bones //
				ObjectFileTextBlock* boneblock = NULL;
				for(unsigned int ind = 0; ind < FileStructure[i]->TextBlocks.size(); ind++){
					if(FileStructure[i]->TextBlocks[ind]->Name == L"Bones"){
						boneblock = FileStructure[i]->TextBlocks[ind];
						break;
					}
				}

				// replace old vertices //
				if(vertexblock == NULL){
					// something is wrong //
					Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: file saving error, could not locate VertexCoordinates block");

				} else {
					for(unsigned int ind = 0; ind < Vertices.size(); ind++){
						// write //
						converter.str(L"");
						converter << Vertices[ind]->X() << L" " << Vertices[ind]->Y() << L" " << Vertices[ind]->Z();
						// group data //
						if((ind < VerticeWeights.size()) && (VerticeWeights.size() != 0)){
							// loop and add //
							converter << L" groups(";
							for(unsigned int addindex = 0; addindex < VerticeWeights[ind].size(); addindex++){
								converter << L"[" << VerticeWeights[ind][addindex]->iVal << L", " << VerticeWeights[ind][addindex]->fVal << L"]";
							}
							converter << L")";
						}

						// set line //
						(*vertexblock->Lines[ind]) = converter.str();
					}
				}
				// replace old faces //
				if(faceblock == NULL){
					// something is wrong //
					Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: file saving error, could not locate Faces block");

				} else {
					for(unsigned int ind = 0; ind < Faces.size(); ind++){
						// write //
						converter.str(L"");
						bool First = true;
						for(unsigned int addindex = 0; addindex < Faces[ind]->VertexIDS.size(); addindex++){
							if(!First){
								converter << L" ";
							}
							First = false;

							converter << Faces[ind]->VertexIDS[addindex];
							// if uv exists, put it //
							if(addindex < Faces[ind]->UVs.size())
								converter << L" uv(" << Faces[ind]->UVs[addindex].GetX() << L", " << Faces[ind]->UVs[addindex].GetY() << L")";
						}

						(*faceblock->Lines[ind]) = converter.str();
					}
				}
				if(boneblock){
					// replace old bones with new ones //
					if(!Skeleton){
						// invalid bone definition //
						Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: Trying to save invalid bone structure");
						// erase data //
						boneblock->Lines.clear();
						boneblock->Name.clear();
					} else {

					}
				}

				////for(unsigned int i = 0; i < VertexNormals.size(); i++){
				////	// write //
				////	converter.str(L"");
				////	converter << VertexNormals[i]->X() << L" " << VertexNormals[i]->Y() << L" " << VertexNormals[i]->Z();

				////	normals->Lines.push_back(new wstring(converter.str()));
				////}

				bool Succeeded = true;

				// call save //
				//if(ObjectFileProcessor::WriteObjectFile(SaveStructure, *file, SavedHeadVars, false) != 1){
				if(ObjectFileProcessor::WriteObjectFile(FileStructure, *file, SavedHeadVars, false) != 1){
					// error //
					Succeeded = false;
					// don't error on this one //
					//Type = MODELOBJECT_MODEL_TYPE_ERROR;
					Logger::Get()->Error(L"NormalModelData: LoadFromLEVMO: failed to resave the file");
				}

				// clear save structure //
				//SAFE_DELETE_VECTOR(SavedHeadVars);
				SavedHeadVars.clear();
			}

			// cleanup //
			SAFE_DELETE_VECTOR(Vertices);
			//SAFE_DELETE_VECTOR(VertexNormals);

			SAFE_DELETE_VECTOR(Faces);
			for(unsigned int delind = 0; delind < VerticeWeights.size(); delind++){
				SAFE_DELETE_VECTOR(VerticeWeights[delind]);
			}
			
			IsLoaded = true;
			Type = MODELOBJECT_MODEL_TYPE_NORMAL;
			//return true;
			continue;
		}
		// invalid object //
		Logger::Get()->Error(L"NormalModelData: load levmo: file contains unknown structure type: "+FileStructure[i]->TName);
	}
	// cleanup //
	SAFE_DELETE_VECTOR(FileStructure);
	//SAFE_DELETE_VECTOR(HeadVars);

	if(pRenderModel.size() == 0){
		// failed //
		Logger::Get()->Error(L"NormalModelData: load levmo: file has invalid structure, no objects found");
		Type = MODELOBJECT_MODEL_TYPE_ERROR;
		return false;
	}
	IsLoaded = true;
	Type = MODELOBJECT_MODEL_TYPE_NORMAL;
	return true;
}

DLLEXPORT bool Leviathan::GameObject::NormalModelData::LoadFromOBJ(wstring* file){
	// struct that only this function requires //
	struct FaceType{
	public:
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	};
	// -------------------------------------- //



	// autodesk object file, hopefully from Blender //
	wifstream reader;
	wchar_t readchar;


	// Initialize the counts.
	VertexCount = 0;
	int texturecount = 0;
	int normalcount = 0;
	int facecount = 0;

	// Open the file.
	reader.open(*file);

	// Check if it was successful in opening the file.
	if(!reader.good()){

		return false;
	}

	// Read from the file and continue to read until the end of the file is reached.
	reader.get(readchar);
	while(reader.good()){

		// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
		if(readchar == L'v'){
			reader.get(readchar);
			if(readchar == L' '){ VertexCount++; }
			if(readchar == L't'){ texturecount++; }
			if(readchar == L'n'){ normalcount++; }
		}

		// If the line starts with 'f' then increment the face count.
		if(readchar == 'f'){
			reader.get(readchar);
			if(readchar == L' '){ facecount++; }
		}

		// Otherwise read in the remainder of the line.
		while(readchar != L'\n'){
			reader.get(readchar);
		}

		// Start reading the beginning of the next line.
		reader.get(readchar);
	}

	// go back to beginning
	reader.clear();
	reader.seekg(0, 0);


	ModelVertexFormat *vertices, *texcoords, *normals;
	FaceType *faces;
	int vertexindex, texcoordindex, normalindex, faceindex; //vindex, tindex, nindex;

	// Initialize the four data structures.
	vertices = new ModelVertexFormat[VertexCount];
	if(!vertices){
		return false;
	}

	texcoords = new ModelVertexFormat[texturecount];
	if(!texcoords){
		return false;
	}

	normals = new ModelVertexFormat[normalcount];
	if(!normals){
		return false;
	}

	faces = new FaceType[facecount];
	if(!faces){
		return false;
	}

	// Initialize the indexes.
	vertexindex = 0;
	texcoordindex = 0;
	normalindex = 0;
	faceindex = 0;

	if(!reader.good()){
		return false;
	}

	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	reader.get(readchar);
	while(reader.good()){
		if(readchar == L'v'){
			reader.get(readchar);

			// Read in the vertices.
			if(readchar == L' '){
				reader >> vertices[vertexindex].x >> vertices[vertexindex].y >> vertices[vertexindex].z;

				// Invert the Z vertex to change to left hand system.
				vertices[vertexindex].z = vertices[vertexindex].z * -1.0f;
				vertexindex++; 
			}

			// Read in the texture uv coordinates.
			if(readchar == L't'){ 
				reader >> texcoords[texcoordindex].x >> texcoords[texcoordindex].y;

				// Invert the V texture coordinates to left hand system.
				texcoords[texcoordindex].y = 1.0f - texcoords[texcoordindex].y;
				texcoordindex++; 
			}

			// Read in the normals.
			if(readchar == L'n'){ 
				reader >> normals[normalindex].x >> normals[normalindex].y >> normals[normalindex].z;

				// Invert the Z normal to change to left hand system.
				normals[normalindex].z = normals[normalindex].z * -1.0f;
				normalindex++; 
			}
		}

		// Read in the faces.
		if(readchar == L'f'){
			reader.get(readchar);
			if(readchar == L' '){

				// Read the face data in backwards to convert it to a left hand system from right hand system.
				reader >> faces[faceindex].vIndex3;
				// check if coord index is defined //
				if(reader.peek() == L'/'){
					// check for value
					reader.get(readchar);
					if(reader.peek() != L'/'){
						reader >> faces[faceindex].tIndex3;
						if(reader.peek() == L'/'){
							reader.get(readchar);
							reader >> faces[faceindex].nIndex3;
						}
					} else {
						faces[faceindex].tIndex3 = 0;
						// skip 1 '/' to get to normal index //
						reader.get(readchar);
						//reader.get(readchar);
						reader >> faces[faceindex].nIndex3;
					}
				} else {
					faces[faceindex].tIndex3 = 0;
					faces[faceindex].nIndex3 = 0;
				}
				// second //
				reader >> faces[faceindex].vIndex2;
				// check if coord index is defined //
				if(reader.peek() == L'/'){
					// check for value
					reader.get(readchar);
					if(reader.peek() != L'/'){
						reader >> faces[faceindex].tIndex2;
						if(reader.peek() == L'/'){
							reader.get(readchar);
							reader >> faces[faceindex].nIndex2;
						}
					} else {
						faces[faceindex].tIndex2 = 0;
						// skip 1 '/' to get to normal index //
						reader.get(readchar);
						//reader.get(readchar);
						reader >> faces[faceindex].nIndex2;
					}
				} else {
					faces[faceindex].tIndex2 = 0;
					faces[faceindex].nIndex2 = 0;
				}

				// first which is actually last in file //
				reader >> faces[faceindex].vIndex1;
				// check if coord index is defined //
				if(reader.peek() == L'/'){
					// check for value
					reader.get(readchar);
					if(reader.peek() != L'/'){
						reader >> faces[faceindex].tIndex1;
						if(reader.peek() == L'/'){
							reader.get(readchar);
							reader >> faces[faceindex].nIndex1;
						}
					} else {
						faces[faceindex].tIndex1 = 0;
						// skip 1 '/' to get to normal index //
						reader.get(readchar);
						//reader.get(readchar);
						reader >> faces[faceindex].nIndex1;
					}
				} else {
					faces[faceindex].tIndex1 = 0;
					faces[faceindex].nIndex1 = 0;
				}
				faceindex++;
			}
		}

		// Read in the remainder of the line.
		while(readchar != L'\n'){
			reader.get(readchar);
		}

		// Start reading the beginning of the next line.
		reader.get(readchar);
	}

	// Close the file.
	reader.close();


	vertexindex = 0;

	int vIndex = 0;
	int tIndex = 0;
	int nIndex = 0;


	// loading model //

	VertexCount = facecount*3;

	// Set the number of indices to be the same as the vertex count.
	IndexCount = VertexCount;

	// Create the model using the vertex count that was read in.
	pRenderModel.resize(VertexCount);
	// fill it //
	for(int ind = 0; ind < VertexCount; ind++){
		// create new //
		pRenderModel[ind] = new NormalModelVertexType();
		if(!pRenderModel[ind]){
			// failed to get memory //
			QUICK_MEMORY_ERROR_MESSAGE;
			return false;
		}
	}

	// Load in vertex data
	int fileverticeindex = 0;

	for(int i = 0; i < facecount; i++){


		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

		pRenderModel[fileverticeindex]->x = vertices[vIndex].x;
		pRenderModel[fileverticeindex]->y = vertices[vIndex].y;
		pRenderModel[fileverticeindex]->z = vertices[vIndex].z;
		if(tIndex > -1){
			pRenderModel[fileverticeindex]->tu = texcoords[tIndex].x;
			pRenderModel[fileverticeindex]->tv = texcoords[tIndex].y;
		} else {
			pRenderModel[fileverticeindex]->tu = 0;
			pRenderModel[fileverticeindex]->tv = 0;
		}
		if(nIndex > -1){	
			pRenderModel[fileverticeindex]->nx = normals[nIndex].x;
			pRenderModel[fileverticeindex]->ny = normals[nIndex].y;
			pRenderModel[fileverticeindex]->nz = normals[nIndex].z;
		} else {
			pRenderModel[fileverticeindex]->nx = 0;
			pRenderModel[fileverticeindex]->ny = 0;
			pRenderModel[fileverticeindex]->nz = 0;
		}

		fileverticeindex++;
		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;

		pRenderModel[fileverticeindex]->x = vertices[vIndex].x;
		pRenderModel[fileverticeindex]->y = vertices[vIndex].y;
		pRenderModel[fileverticeindex]->z = vertices[vIndex].z;
		if(tIndex > -1){
			pRenderModel[fileverticeindex]->tu = texcoords[tIndex].x;
			pRenderModel[fileverticeindex]->tv = texcoords[tIndex].y;
		} else {
			pRenderModel[fileverticeindex]->tu = 0;
			pRenderModel[fileverticeindex]->tv = 0;
		}
		if(nIndex > -1){	
			pRenderModel[fileverticeindex]->nx = normals[nIndex].x;
			pRenderModel[fileverticeindex]->ny = normals[nIndex].y;
			pRenderModel[fileverticeindex]->nz = normals[nIndex].z;
		} else {
			pRenderModel[fileverticeindex]->nx = 0;
			pRenderModel[fileverticeindex]->ny = 0;
			pRenderModel[fileverticeindex]->nz = 0;
		}

		fileverticeindex++;
		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

		pRenderModel[fileverticeindex]->x = vertices[vIndex].x;
		pRenderModel[fileverticeindex]->y = vertices[vIndex].y;
		pRenderModel[fileverticeindex]->z = vertices[vIndex].z;
		if(tIndex > -1){
			pRenderModel[fileverticeindex]->tu = texcoords[tIndex].x;
			pRenderModel[fileverticeindex]->tv = texcoords[tIndex].y;
		} else {
			pRenderModel[fileverticeindex]->tu = 0;
			pRenderModel[fileverticeindex]->tv = 0;
		}
		if(nIndex > -1){	
			pRenderModel[fileverticeindex]->nx = normals[nIndex].x;
			pRenderModel[fileverticeindex]->ny = normals[nIndex].y;
			pRenderModel[fileverticeindex]->nz = normals[nIndex].z;
		} else {
			pRenderModel[fileverticeindex]->nx = 0;
			pRenderModel[fileverticeindex]->ny = 0;
			pRenderModel[fileverticeindex]->nz = 0;
		}

		fileverticeindex++;
	}
	// write a file //
	WriteToFile(FileSystem::ChangeExtension(*file, L".levmo"), false);

	// release data structures //
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(texcoords);
	SAFE_DELETE_ARRAY(normals);
	SAFE_DELETE_ARRAY(faces);

	IsLoaded = true;
	return true;
}

DLLEXPORT bool Leviathan::GameObject::NormalModelData::WriteToFile(const wstring& file, bool InBinary /*= false*/){
	throw exception("Not done, doesn't work");
	//// save file structured into a file //
	//vector<ObjectFileObject*> SaveStructure;
	//vector<NamedVar> SavedHeadVars;
	//SaveStructure.push_back(new ObjectFileObject(this->ThisName, -1, L"ModelData"));

	//// header data //
	//SavedHeadVars.push_back(NamedVar(L"FileType", L"ModelData"));
	//SavedHeadVars.push_back(NamedVar(L"Model-Name", L'\"'+ThisName+L'\"'));
	//SavedHeadVars.push_back(NamedVar(L"CoordinateSystem", L"LEVIATHAN"));

	//// add all the other data //
	//ObjectFileObject* main = SaveStructure[0];

	//// fill up structures //
	//ObjectFileTextBlock* verts = new ObjectFileTextBlock(L"VertexCoordinates");
	//ObjectFileTextBlock* normals = new ObjectFileTextBlock(L"VertexNormals");
	//ObjectFileTextBlock* faces = new ObjectFileTextBlock(L"Faces");

	//// create temporary structures for collecting data //
	//vector<Float3*> Vertices;
	//vector<Float3*> VertexNormals;

	//vector<LoadingFace*> Faces;

	////int FaceCount = /3;

	//// reverse data //
	//for(int i = 0; i < VertexCount; i+=3){
	//	RenderModelType* current = &pRenderModel[i];
	//	RenderModelType* next = &pRenderModel[i+1];
	//	RenderModelType* nextnext = &pRenderModel[i+2];

	//	// create face //
	//	LoadingFace* face = new LoadingFace();

	//	// add stuff //
	//	// uvs //
	//	face->UVs.push_back(Float2(current->tu, current->tv));
	//	face->UVs.push_back(Float2(next->tu, next->tv));
	//	face->UVs.push_back(Float2(nextnext->tu, nextnext->tv));

	//	// add vertices //	
	//	Vertices.push_back(new Float3(current->x, current->y, current->z));
	//	face->VertexIDS.push_back(Vertices.size()-1);
	//	Vertices.push_back(new Float3(next->x, current->y, next->z));
	//	face->VertexIDS.push_back(Vertices.size()-1);
	//	Vertices.push_back(new Float3(nextnext->x, nextnext->y, nextnext->z));
	//	face->VertexIDS.push_back(Vertices.size()-1);


	//	// add normals //
	//	VertexNormals.push_back(new Float3(current->nx, current->ny, current->nz));
	//	VertexNormals.push_back(new Float3(next->nx, next->ny, next->nz));
	//	VertexNormals.push_back(new Float3(nextnext->nx, nextnext->ny, nextnext->nz));

	//	// add face //
	//	Faces.push_back(face);
	//}
	//// check for repeats //
	//int MaxIndex = Vertices.size();

	//for(unsigned int i = 0; i < Vertices.size(); i++){
	//	// check if same is defined elsewhere //
	//	Float3* cur = Vertices[i];
	//	// start check higher //
	//	for(unsigned int a = 0; a < Vertices.size(); a++){
	//		Float3* check = Vertices[a];

	//		if(cur == check)
	//			// is same //
	//			continue;

	//		// don't forget to un-pointerize or it won't work //
	//		if(*check == *cur){
	//			// needs to be collapsed //

	//			// replace IDS in faces //
	//			for(unsigned int ind = 0; ind < Faces.size(); ind++){
	//				Misc::ReplaceValuesInVector<int>(Faces[ind]->VertexIDS, a, i);
	//			}
	//			// all higher values need to be reduced by 1 //
	//			for(int intval = a+1; intval < MaxIndex; intval++){
	//				// replace IDS in faces //
	//				for(unsigned int ind = 0; ind < Faces.size(); ind++){
	//					Misc::ReplaceValuesInVector<int>(Faces[ind]->VertexIDS, intval, intval-1);
	//				}
	//			}
	//			MaxIndex--;

	//			// remove index from normals //
	//			SAFE_DELETE(VertexNormals[a]);
	//			VertexNormals.erase(VertexNormals.begin()+a);
	//			SAFE_DELETE(Vertices[a]);
	//			Vertices.erase(Vertices.begin()+a);
	//			a--;
	//			continue;
	//		}
	//	}

	//}

	//// write to blocks //
	//wstringstream converter;

	//for(unsigned int i = 0; i < Vertices.size(); i++){
	//	// write //
	//	converter.str(L"");
	//	converter << Vertices[i]->X() << L" " << Vertices[i]->Y() << L" " << Vertices[i]->Z();

	//	verts->Lines.push_back(new wstring(converter.str()));
	//}
	//for(unsigned int i = 0; i < VertexNormals.size(); i++){
	//	// write //
	//	converter.str(L"");
	//	converter << VertexNormals[i]->X() << L" " << VertexNormals[i]->Y() << L" " << VertexNormals[i]->Z();

	//	normals->Lines.push_back(new wstring(converter.str()));
	//}
	//for(unsigned int i = 0; i < Faces.size(); i++){
	//	// write //
	//	converter.str(L"");

	//	converter << Faces[i]->VertexIDS[0];
	//	if(Faces[i]->UVs.size() > 0){
	//		converter << L" uv(" << Faces[i]->UVs[0].GetX() << L", " << Faces[i]->UVs[0].GetY() << L")";
	//	}
	//	converter << L" " << Faces[i]->VertexIDS[1];
	//	if(Faces[i]->UVs.size() > 1){
	//		converter << L" uv(" << Faces[i]->UVs[1].GetX() << L", " << Faces[i]->UVs[1].GetY() << L")";
	//	}
	//	converter << L" " << Faces[i]->VertexIDS[2];
	//	if(Faces[i]->UVs.size() > 2){
	//		converter << L" uv(" << Faces[i]->UVs[2].GetX() << L", " << Faces[i]->UVs[2].GetY() << L")";
	//	}

	//	faces->Lines.push_back(new wstring(converter.str()));
	//}


	//// add data //
	//main->TextBlocks.push_back(verts);
	//main->TextBlocks.push_back(normals);
	//main->TextBlocks.push_back(faces);

	//bool Succeeded = true;

	//// call save //
	//if(ObjectFileProcessor::WriteObjectFile(SaveStructure, file, SavedHeadVars, InBinary) != 1){
	//	// error //
	//	Succeeded = false;
	//	// don't error on this one //
	//	//Type = MODELOBJECT_MODEL_TYPE_ERROR;
	//}

	//// clear save structure //
	//// deleting the structure deletes all sub structures //
	//SAFE_DELETE_VECTOR(SaveStructure);
	//SAFE_DELETE_VECTOR(Vertices);
	//SAFE_DELETE_VECTOR(VertexNormals);
	//SAFE_DELETE_VECTOR(Faces);
	////Type = MODELOBJECT_MODEL_TYPE_NORMAL;
	//return Succeeded;
}

DLLEXPORT wstring Leviathan::GameObject::NormalModelData::GetModelTypeName(){
	return L"NormalModel";
}

DLLEXPORT SkeletonRig* Leviathan::GameObject::NormalModelData::GetSkeleton(){
	return Skeleton;
}

// ------------------------------------ //

Leviathan::GameObject::NormalModelData::LoadingFace::LoadingFace(){
	// expects 3 elements to each //
	VertexIDS.reserve(3);
	UVs.reserve(3);
}
