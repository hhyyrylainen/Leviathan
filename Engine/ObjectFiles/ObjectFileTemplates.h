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
	
	// These could be used in the future to streamline instances
	////! \brief Represents the data source for PotentiallyTemplatizedValue
	//template<class ValType, class DataSourceType>
	//class TemplateValueSource{
	//public:



	//protected:

	//	//! The master value that the template holds
	//	//! This will only be set when the value doesn't need specialization
	//	unique_ptr<ValType> ActualNonTemplateValue;

	//	//! The raw value where from ActualNonTemplateValue is generated
	//	unique_ptr<DataSourceType> RawValueString;



	//};

	////! \brief Represents a value that is potentially templatized
	//template<class ValType, class DataSourceType>
	//class PotentiallyTemplatizedValue{
	//public:



	//protected:


	//};


	//! \brief Class that represents an object created from a template
	class ObjectFileTemplateObject : public ObjectFileObjectProper{
	public:

		//! \brief Creates an ObjectFileTemplateObject as a wrapper around an ObjectFileObject
		//! \see ObjectFileObjectProper
		DLLEXPORT ObjectFileTemplateObject(const wstring &name, const wstring &typesname, vector<wstring*> prefix);
		


		DLLEXPORT virtual bool IsThisTemplated() const{

			return true;
		}

	};



	//! \brief Represents a template instantiation
	class ObjectFileTemplateInstance{
		friend ObjectFileTemplateDefinition;
	public:

		DLLEXPORT ObjectFileTemplateInstance(const string &mastertmplname, std::vector<unique_ptr<string>> &templateargs);


		DLLEXPORT inline const string& GetNameOfParentTemplate() const{

			return TemplatesName;
		}


	protected:

		//! The name of the master template from which the instances are generated
		string TemplatesName;

		//! Template arguments
		std::vector<unique_ptr<string>> Arguments;

		//! Converted template arguments to avoid having to convert multiple times
		std::vector<unique_ptr<wstring>> WArguments;

	};



	//! \brief Class that represents a template definition
	//! \todo Potentially allow changing the definition to update instantiations
	//! \todo Make this more robust and nice and reduce the bloat in the implementation
	class ObjectFileTemplateDefinition{
	public:

		//! \brief Creates a ObjectFileTemplateDefinition
		//! \warning CreateFromObject might change in the future so please don't use this function directly
		DLLEXPORT ObjectFileTemplateDefinition(const string &name, std::vector<unique_ptr<string>> &parameters, unique_ptr<ObjectFileObject> obj);

		//! \brief Gets the name of this template
		DLLEXPORT const string& GetName() const;


		//! \brief Creates a ObjectFileTemplateDefinition from an ObjectFileObject and a parameter list
		//! \param obj The object from which to construct the template, the pointer will be deleted by this
		DLLEXPORT static shared_ptr<ObjectFileTemplateDefinition> CreateFromObject(const string &name, ObjectFileObject* obj, 
			std::vector<unique_ptr<string>> &templateargs);

		//! \brief Creates an instance from this template
		//! \todo Refactor this function to be smaller
		//! \todo Allow objects to use the special defined values in the ObjectFileProcessor
		DLLEXPORT unique_ptr<ObjectFileTemplateObject> CreateInstanceFromThis(const ObjectFileTemplateInstance &instanceargs);



	protected:

		void ReplaceWstringWithTemplateArguments(wstring &target, const std::vector<unique_ptr<wstring>> &args);
		string ReplaceStringTemplateArguments(const string &target, const std::vector<unique_ptr<string>> &args);

		// ------------------------------------ //

		//! Name of this template
		string Name;

		//! Template parameter definitions
		std::vector<unique_ptr<string>> Parameters;


		//! Converted template parameter definitions
		std::vector<unique_ptr<wstring>> WParameters;


		//! The object from which the instances are created
		unique_ptr<ObjectFileObject> RepresentingObject;
	};


}
#endif