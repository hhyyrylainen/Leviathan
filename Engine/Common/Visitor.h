#pragma once
// ------------------------------------ //
#include "Include.h"

namespace Leviathan{

    class Visitor;

    //! \brief Base class for all classes that accept visitors
    class Visitable{
    public:

        DLLEXPORT virtual void Accept(Visitor &visitor) = 0;
    };

    //! \brief Base for all different visitors
	class Visitor{
	public:
		DLLEXPORT Visitor();

        //! Default virtual destructor
        DLLEXPORT virtual ~Visitor();

        DLLEXPORT virtual void Visit(Visitable* object);

        
	};

    //! \brief Example visitor that does nothing, but doesn't crash either
    class EmptyVisitor : public Visitor{
    public:

        
        DLLEXPORT void Visit(Visitable* object) override;
    };
}
