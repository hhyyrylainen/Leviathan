// ------------------------------------ //
#include "ObjectFileTextBlock.h"

#include "utf8/checked.h"
#include "Exceptions.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileTextBlockProper::ObjectFileTextBlockProper(const string &name) :
    Name(name)
{

}

DLLEXPORT Leviathan::ObjectFileTextBlockProper::~ObjectFileTextBlockProper(){
	SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //
DLLEXPORT const string& Leviathan::ObjectFileTextBlockProper::GetName() const{
	return Name;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ObjectFileTextBlockProper::GetLineCount() const{
	return Lines.size();
}

DLLEXPORT const string& Leviathan::ObjectFileTextBlockProper::GetLine(size_t index) const{
	// Check the index //
	if(index >= Lines.size()){

		throw InvalidArgument("index is out of range");
	}
	
	return *Lines[index];
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ObjectFileTextBlockProper::AddTextLine(const string &line){
	Lines.push_back(new string(line));
}

// ------------------ ObjectFileTextBlock ------------------ //
DLLEXPORT Leviathan::ObjectFileTextBlock::~ObjectFileTextBlock(){

}
