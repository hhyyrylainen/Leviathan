#pragma once
#ifndef LEVIATHAN_BASEENTITYSERIALIZER
#define LEVIATHAN_BASEENTITYSERIALIZER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

    //! \brief The possible inbuilt types of BaseEntitySerializer
    enum ENTITYSERIALIZEDTYPE{
        //! The generic type which should be able to be decoded by the generic handler
        ENTITYSERIALIZEDTYPE_COMPONENT_CONTAINER = 1,
        
        //! This has to be the last value, use this to specify custom types
        ENTITYSERIALIZEDTYPE_LAST
    };
    
    //! \brief Base class for all entity serializer classes
    //! \note All possible types should be defined in this file here
	class BaseEntitySerializer{
	public:
        //! Type that should be large enough to hold everything in ENTITYSERIALIZEDTYPE and all custom types.
        typedef int TypeIDSize;

        //! \brief Creates a serializer which is guaranteed to be able to serialize the type
		DLLEXPORT BaseEntitySerializer(ENTITYSERIALIZEDTYPE type);
        DLLEXPORT virtual ~BaseEntitySerializer();

        //! Returns true when this serializer can work with the specified type
        DLLEXPORT bool CanSerializeType(TypeIDSize typetocheck) const;
        
        
    protected:
        
        //! The type this object can serialize
        //! This is actually of type ENTITYSERIALIZEDTYPE
        const TypeIDSize Type;

    private:
        
        // Disallow copy and assign //
        BaseEntitySerializer(const BaseEntitySerializer &other) = delete;
        BaseEntitySerializer& operator =(const BaseEntitySerializer &other) = delete;

	};
    
}
#endif
