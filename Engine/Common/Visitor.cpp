// ------------------------------------ //
#include "Visitor.h"

#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Visitor::Visitor(){

}

DLLEXPORT Visitor::~Visitor(){

}
// ------------------------------------ //
DLLEXPORT void Visitor::Visit(Visitable* object){

    throw InvalidType("Base Visitor type is getting called by visitable");
}
// ------------------ EmptyVisitor ------------------ //
DLLEXPORT void EmptyVisitor::Visit(Visitable* object){

    // Emptyness...
}


