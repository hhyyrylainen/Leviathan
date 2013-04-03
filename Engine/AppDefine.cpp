#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATIONDEFINE
#include "AppDefine.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
AppDef::AppDef(){
	values = new NamedVars();



}

AppDef::AppDef(bool isdef){
	if(isdef){
		Defaultconf = this;
	}
	// default values //
	values = new NamedVars();
}
AppDef::~AppDef(){
	// release allocated stuff //
	SAFE_DELETE(values);

}
// ------------------------------------ //
AppDef* AppDef::Defaultconf = NULL;
AppDef* AppDef::GetDefault(){
	return Defaultconf;
}
	// default //
//AppDef DefApp(true);

NamedVars* Leviathan::AppDef::GetValues(){
	return values;
}
