#pragma once
#ifndef LEVIATHAN_OBJECTFILETEMPLATES
#define LEVIATHAN_OBJECTFILETEMPLATES
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ObjectFileObject.h"



namespace Leviathan{

	//! \brief Class that represents a template definition
	class ObjectFileTemplateDefinition{
	public:


		//! \brief Creates a ObjectFileTemplateDefinition from an ObjectFileObject and a parameter list
		DLLEXPORT static shared_ptr<ObjectFileTemplateDefinition> CreateFromObject(const string &name, ObjectFileObject* obj, 
			std::vector<unique_ptr<string>> &templateargs);


	protected:


	};


	//! \brief Represents a template instantiation
	class ObjectFileTemplateInstance{
	public:

		DLLEXPORT ObjectFileTemplateInstance(const string &mastertmplname, std::vector<unique_ptr<string>> &templateargs);



	protected:


	};



	//! \brief Class that represents an object created from a template
	class ObjectFileTemplateObject : public ObjectFileObject{
	public:


		//! \brief Creates a ObjectFileTemplateDefinition from an ObjectFileObject and a parameter list
		DLLEXPORT shared_ptr<ObjectFileTemplateDefinition> CreateFromObject(ObjectFileObject* obj, std::vector<unique_ptr<string>> &templateargs);


	protected:


	};

}
#endif