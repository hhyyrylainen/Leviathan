#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
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
		DLLEXPORT ObjectFileTemplateObject(const std::string &name, const std::string &typesname,
            std::vector<std::string*> prefix);
		


		DLLEXPORT virtual bool IsThisTemplated() const{

			return true;
		}

	};



	//! \brief Represents a template instantiation
	class ObjectFileTemplateInstance{
		friend ObjectFileTemplateDefinition;
	public:

		DLLEXPORT ObjectFileTemplateInstance(const std::string &mastertmplname,
            std::vector<std::unique_ptr<std::string>> &templateargs);


		DLLEXPORT inline const std::string& GetNameOfParentTemplate() const{

			return TemplatesName;
		}


	protected:

		//! The name of the master template from which the instances are generated
        std::string TemplatesName;

		//! Template arguments
		std::vector<std::unique_ptr<std::string>> Arguments;
	};



	//! \brief Class that represents a template definition
	//! \todo Potentially allow changing the definition to update instantiations
	//! \todo Make this more robust and nice and reduce the bloat in the implementation
	class ObjectFileTemplateDefinition{
	public:

		//! \brief Creates a ObjectFileTemplateDefinition
		//! \warning CreateFromObject might change in the future so please don't use
        //! this function directly
		DLLEXPORT ObjectFileTemplateDefinition(const std::string &name,
            std::vector<std::unique_ptr<std::string>> &parameters,
            std::shared_ptr<ObjectFileObject> obj);

		//! \brief Gets the name of this template
		DLLEXPORT const std::string& GetName() const;


		//! \brief Creates a ObjectFileTemplateDefinition from an ObjectFileObject and a parameter list
		//! \param obj The object from which to construct the template, the pointer will be deleted by this
		DLLEXPORT static std::shared_ptr<ObjectFileTemplateDefinition> CreateFromObject(
            const std::string &name, std::shared_ptr<ObjectFileObject> obj, 
			std::vector<std::unique_ptr<std::string>> &templateargs);

		//! \brief Creates an instance from this template
		//! \todo Refactor this function to be smaller
		//! \todo Allow objects to use the special defined values in the ObjectFileProcessor
		DLLEXPORT std::unique_ptr<ObjectFileTemplateObject> CreateInstanceFromThis(
            const ObjectFileTemplateInstance &instanceargs);



	protected:

		void ReplaceStringWithTemplateArguments(std::string &target,
            const std::vector<std::unique_ptr<std::string>> &args);
        
        std::string ReplaceStringTemplateArguments(const std::string &target,
            const std::vector<std::unique_ptr<std::string>> &args);

		// ------------------------------------ //

		//! Name of this template
        std::string Name;

		//! Template parameter definitions
		std::vector<std::unique_ptr<std::string>> Parameters;

		//! The object from which the instances are created
        std::shared_ptr<ObjectFileObject> RepresentingObject;
	};


}

