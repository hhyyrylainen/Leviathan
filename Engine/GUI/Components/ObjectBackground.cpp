#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIOBJECTBACKGROUND
#include "ObjectBackground.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "ObjectFiles\ObjectFileProcessor.h"

// initialize all different background pointers to NULL //
DLLEXPORT Leviathan::Gui::ObjectBackground::ObjectBackground(int slot, int zorder) : BaseGraphicalComponent(slot, zorder), BData(NULL){
	// set type //
	CType = GUI_GRAPHICALCOMPONENT_TYPE_BACKGROUND;
	// not initialized //
	WhichType = BACKGROUNDTYPE_NONE;

	NeedsToUnAllocateRBData = false;
}

DLLEXPORT Leviathan::Gui::ObjectBackground::~ObjectBackground(){
	// release data if any (could have been released, but release isn't actually required to be called on graphical components) //
	_UnAllocateAllBackgroundData();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::ObjectBackground::Release(RenderBridge* bridge){
	// remove from render bridge //
	size_t i = bridge->GetSlotIndex(Slot);

	SAFE_DELETE(bridge->DrawActions[i]);
	bridge->DrawActions.erase(bridge->DrawActions.begin()+i);

	// unallocate //
	_UnAllocateAllBackgroundData();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::ObjectBackground::Init(shared_ptr<NamedVariableList> variables){
	// we can just call the update function and be done with it //
	Update(variables.get());

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::ObjectBackground::Render(RenderBridge* bridge, Graphics* graph){
	if(!RUpdated){
		// nothing to update //
		return;
	}
	// verify that type matches what is actually stored //
	if(WhichType != BData->GetType())
		DEBUG_BREAK;


	// ensure that right thing and parameters are in the render bridge //
	size_t i = bridge->GetSlotIndex(Slot);

	// delete data if type has changed //
	if(NeedsToUnAllocateRBData){
		SAFE_DELETE(bridge->DrawActions[i]);
		NeedsToUnAllocateRBData = false;
	}


	// we need to ensure that the object is actually created //
	if(bridge->DrawActions[i] == NULL){
		// allocate new //

		// we'll take advantage of background data base class virtual functions to create new blob //
		bridge->DrawActions[i] = BData->CreateBlobForThis(graph, this);
	}

	// if it is still null we need to error //
	assert(bridge->DrawActions[i] != NULL && "cannot create rendering for object's background");

	// call overloaded rendering method of the data object //
	BData->RenderThis(bridge, graph, i, this);

	
	// set as non updated //
	RUpdated = false;
	PositionsUpdated = false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Gui::ObjectBackground::Update(NamedVariableList* variables){
	// first check first object for name of background type //
	
	// we could catch the possible exception here, but it might be better to let if climb higher //
	VariableBlock& basename = variables->GetValue(0);

	wstring backgroundtypename;

	if(!basename.ConvertAndAssingToVariable<wstring>(backgroundtypename)){

		Logger::Get()->Error(L"ObjectBackground: Update: invalid background name, not a string");
		return;
	}

	if(backgroundtypename == L"gradient"){
		// type is now gradient, we just need to match the shape //

		VariableBlock& shape = variables->GetValue(1);
		// recycle the old variable //
		if(!shape.ConvertAndAssingToVariable<wstring>(backgroundtypename)){

			Logger::Get()->Error(L"ObjectBackground: Update: invalid gradient shape, not a string");
			return;
		}

		if(backgroundtypename == L"box"){
			// we have found the right type //
			
			// now we make sure that we have pointer of right type and call the update function on the base class //
			_EnsureBackgroundDataType<BACKGROUNDTYPE_GRADIENT>();

			// now we'll just call the update function, next index to look at is 2 here //
			try{
				BData->Update(variables, 2);
			}
			catch(...){
				// we catch here because we have almost initialized, but if the rest of parameters are wrong we might as well be type of none //
				_EnsureBackgroundDataType<BACKGROUNDTYPE_NONE>();
				Logger::Get()->Error(L"ObjectBackground: Update: failed to update gradient, transforming into empty background");
			}
			return;
		}


		return;
	}

	// non matched type //
	Logger::Get()->Error(L"ObjectBackground: Update: invalid background name, "+backgroundtypename+L" didn't match anything");
}

DLLEXPORT void Leviathan::Gui::ObjectBackground::Update(const Float4 &colour1, const Float4 &colour2, const int &gradient){
	// ensure that data object is right //
	_EnsureBackgroundDataType<BACKGROUNDTYPE_GRADIENT>();

	// set parameters //
	GradientBackgroundData* tmpptr = static_cast<GradientBackgroundData*>(BData);

	// set data //
	tmpptr->Set(colour1, colour2, gradient);

	// ensure that updated is set //
	RUpdated = true;
}
// ------------------------------------ //
void Leviathan::Gui::ObjectBackground::_OnLocationOrSizeChange(){
	// set as updated //
	RUpdated = true;
}

void Leviathan::Gui::ObjectBackground::_UnAllocateAllBackgroundData(){
	// delete data pointer and set type accordingly //
	SAFE_DELETE(BData);
	WhichType = BACKGROUNDTYPE_NONE;
	// we also need to delete the rendering bridge object //
	NeedsToUnAllocateRBData = true;
}
// ------------------ GradientBackgroundData ------------------ //
Leviathan::Gui::GradientBackgroundData::GradientBackgroundData(): BaseBackgroundData(), Colour1(0), Colour2(0), GradientType(0){
	// set right type //
	ThisTYPE = BACKGROUNDTYPE_GRADIENT;
}

Leviathan::Gui::GradientBackgroundData::~GradientBackgroundData(){

}

void Leviathan::Gui::GradientBackgroundData::Set(const Float4 &colour1, const Float4 &colour2, int type){
	Colour1 = colour1;
	Colour2 = colour2;
	GradientType = type;
}

RenderingGBlob* Leviathan::Gui::GradientBackgroundData::CreateBlobForThis(Graphics* graph, ObjectBackground* vars){
	return new ColorQuadRendBlob(graph, vars->ZOrder, vars->Slot, false);
}

void Leviathan::Gui::GradientBackgroundData::RenderThis(RenderBridge* bridge, Graphics* graph, const size_t index, ObjectBackground* vars){
	// update rendering blob //
	ColorQuadRendBlob* tmppr = static_cast<ColorQuadRendBlob*>(bridge->DrawActions[index]);

	tmppr->Update(graph, vars->ZOrder, vars->Position, Colour1, Colour2, vars->Size, GradientType, vars->CoordType);
}

void Leviathan::Gui::GradientBackgroundData::Update(NamedVariableList* variables, int currentindex){
	// get gradient type //
	VariableBlock& gradienttype = variables->GetValue(currentindex++);

	if(!gradienttype.ConvertAndAssingToVariable<int>(GradientType)){
		// report invalid type, but we can continue //
		Logger::Get()->Error(L"GradientBackgroundData: Update: invalid gradient type, variable is not int, (specific int value registration might be broken");
	}
	
	// now there should be two sets of 4 floats //

	if(!ObjectFileProcessor::LoadMultiPartValueFromNamedVariableList<Float4, float, 4>(variables, currentindex, Colour1, Float4::ColourBlack, true, 
		L"GradientBackgroundData: Update:"))
	{
		goto gradientbackgroundtellhowitshouldbelabel;
	}
	//// increment index //
	// incremented automatically by the function
	//currentindex += 4;


	if(!ObjectFileProcessor::LoadMultiPartValueFromNamedVariableList<Float4, float, 4>(variables, currentindex, Colour2, Float4::ColourBlack, true, 
		L"GradientBackgroundData: Update:"))
	{
		goto gradientbackgroundtellhowitshouldbelabel;
	}


	return;

gradientbackgroundtellhowitshouldbelabel:

	Logger::Get()->Error(L"GradientBackgroundData: Update: invalid last arguments, should be like: \"[0.1][0.4][0.7][1.][0.01][0.2][1.0][1.0]\"");
	return;
}

// ------------------ BaseBackgroundData ------------------ //
Leviathan::Gui::BaseBackgroundData::BaseBackgroundData() : ThisTYPE(BACKGROUNDTYPE_NONE){
	// on construct bridge won't be ready //
	RBridgeObjectCreated = false;
}

Leviathan::Gui::BaseBackgroundData::~BaseBackgroundData(){

}
