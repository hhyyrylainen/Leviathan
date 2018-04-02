#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
// ---- includes ---- //
#include "../../Common/ReferenceCounted.h"
#include "../../Utility/Convert.h"
#include <map>
#include <memory>

#ifdef SFML_PACKETS
#include "../../Common/SFMLPackets.h"
#endif

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
#include "../../Exceptions.h"
#endif


static_assert(sizeof(short) == 2, "Short must be 2 bytes for datablocks to work accross the network");

#define DATABLOCK_TYPE_INT		3
#define DATABLOCK_TYPE_FLOAT	4
#define DATABLOCK_TYPE_BOOL		5
#define DATABLOCK_TYPE_WSTRING	6
#define DATABLOCK_TYPE_STRING	7
#define DATABLOCK_TYPE_CHAR		8
#define DATABLOCK_TYPE_DOUBLE	9
#define DATABLOCK_TYPE_VOIDPTR	11

#define DATABLOCK_TYPE_ERROR	9000
#define DATABLOCK_TYPE_UNINITIALIZED 10000

namespace Leviathan{

    // forward declaration //
    template<class DBlockT> class DataBlock;

    // template struct for getting right type for right type...
    template<class T>
    struct DataBlockNameResolver{

        static const short TVal = DATABLOCK_TYPE_ERROR;
    };
    // specification values //

#define NAMERESOLVERTEMPLATEINSTANTIATION(DType, TVALDEFINE) template<> struct DataBlockNameResolver<DType>{ static const short TVal = TVALDEFINE;};

    NAMERESOLVERTEMPLATEINSTANTIATION(int, DATABLOCK_TYPE_INT);
    NAMERESOLVERTEMPLATEINSTANTIATION(float, DATABLOCK_TYPE_FLOAT);
    NAMERESOLVERTEMPLATEINSTANTIATION(bool, DATABLOCK_TYPE_BOOL);
    NAMERESOLVERTEMPLATEINSTANTIATION(std::wstring, DATABLOCK_TYPE_WSTRING);
    NAMERESOLVERTEMPLATEINSTANTIATION(std::string, DATABLOCK_TYPE_STRING);
    NAMERESOLVERTEMPLATEINSTANTIATION(char, DATABLOCK_TYPE_CHAR);
    NAMERESOLVERTEMPLATEINSTANTIATION(double, DATABLOCK_TYPE_DOUBLE);
    NAMERESOLVERTEMPLATEINSTANTIATION(void*, DATABLOCK_TYPE_VOIDPTR);
    // for conversion check to work //
    NAMERESOLVERTEMPLATEINSTANTIATION(void, DATABLOCK_TYPE_VOIDPTR);

    // static class used to convert DataBlocks from different types to other types //
    template<class FromDataBlockType, class TargetType>
    class DataBlockConverter{
    public:
        // conversion function that templates overload //
        static inline TargetType DoConvert(const FromDataBlockType* block){
            LEVIATHAN_ASSERT(0, "conversion not possible");
            return TargetType();
        }
        static const bool AllowedConversion = false;
    };

    // conversion templates //
    template<class DBlockTDT, class T>
    struct DataBlockConversionResolver{
        // handles operator class() functions
        static inline T DoConversionNonPtr(const DataBlock<DBlockTDT>* block){
            // direct conversion check //
            //if(DataBlockNameResolver<T>::TVal == block->Type){
            //	// just return //
            //	return *block->Value;
            //}
            return DataBlockConverter<DataBlock<DBlockTDT>, T>::DoConvert(block);
        }
        // handles operator class*() functions
        static inline T* DoConversionPtr(DataBlock<DBlockTDT>* block){
            // direct conversion check //
            if(DataBlockNameResolver<T>::TVal == block->Type){
                // just return //
                // Already type checked so force the pointer type //
                return reinterpret_cast<T*>(block->Value);
            }
            // cannot return converted value //
//#pragma message ("cannot return pointer from converted type")
            LEVIATHAN_ASSERT(0, "return pointer of converted value not possible");
            return NULL;
        }
        // functions used to check is conversion allowed //
        static inline bool IsConversionAllowedNonPtr(const DataBlock<DBlockTDT>* block){
            // check types //
            if(DataBlockNameResolver<T>::TVal == block->Type){
                // same type, is allowed always //
                return true;
            }
            // use templates to see if there is a template that allows this //
            return DataBlockConverter<DataBlock<DBlockTDT>, T>::AllowedConversion;
        }
        static inline bool IsConversionAllowedPtr(const DataBlock<DBlockTDT>* block){
            // check types //
            if(DataBlockNameResolver<T>::TVal == block->Type){
                // same type, is allowed always //
                return true;
            }
            // different types, cannot be returned as pointer //
            return false;
        }
    };

    // templates for getting AngelScript type id from template //
    template<class TypeToFetchID>
    struct TypeToAngelScriptIDConverter{

        static inline int GetTypeIDFromTemplated(){
            return -1;
        }
    };

    // non template base class for pointers //
    class DataBlockAll{
    public:
        // virtual destructor for deleting through base pointers //
        inline virtual ~DataBlockAll(){


        }

        // function used in deep copy //
        DLLEXPORT virtual DataBlockAll* AllocateNewFromThis() const = 0;


        // comparison operator //
        inline bool operator ==(const DataBlockAll &other){

            // just compare types here //
            return Type == other.Type;
        }

        short Type;

    protected:
        // private constructor to make sure that no instances of this class exist //
        DataBlockAll() : Type(-1){

        }

    };


    //! \brief Main DataBlock class
    //! \todo Add move constructor and move assignment operators
    template<class DBlockT>
    class DataBlock : public DataBlockAll{
        struct _U{};
    public:
        //! \brief Creates an uninitialized block of type non
        //! \warning Calling this will create an invalid block that cannot be casted or used
        explicit DataBlock(const _U &forceempty) : Value(NULL){

            Type = DATABLOCK_TYPE_UNINITIALIZED;
        }
        
        DataBlock(const DBlockT &val) : Value(new DBlockT(val)){

            // use templates to get type //
            Type = DataBlockNameResolver<DBlockT>::TVal;
        }
        DataBlock(DBlockT* val) : Value(val){

            // use templates to get type //
            Type = DataBlockNameResolver<DBlockT>::TVal;
        }

    #ifdef SFML_PACKETS

        DLLEXPORT DataBlock(sf::Packet &packet);

        DLLEXPORT void AddDataToPacket(sf::Packet &packet);
    #endif //SFML_PACKETS

        DataBlock(const DataBlock &otherdeepcopy) : Value(NULL){
            // allocate new pointer from the other instance //
            Value = new DBlockT(*otherdeepcopy.Value);

            // copy type //
            Type = otherdeepcopy.Type;
        }


        virtual ~DataBlock(){
            // erase memory //
            SAFE_DELETE(Value);
        }
        // deep copy operator //
        DataBlock& operator =(const DataBlock& arg){
            // release existing value (if any) //
            SAFE_DELETE(Value);

            // copy type //
            Type = arg.Type;
            // skip if other is null value //
            if(arg.Value == NULL)
                return *this;
            // deep copy //
            Value = new DBlockT(*arg.Value);

            // avoid performance issues //
            return *this;
        }

        // function used in deep copy //
        virtual DataBlockAll* AllocateNewFromThis() const{

            return static_cast<DataBlockAll*>((new DataBlock<DBlockT>(
                        const_cast<const DataBlock<DBlockT>&>(*this))));
        }

        // shallow copy operator //
        // copies just the pointer over (fast for copies that don't need both copies //
        static inline DataBlock* CopyConstructor(DataBlock* arg){

            std::unique_ptr<DataBlock> block(new DataBlock());

            block.get()->Type = arg->Type;
            // copy pointer //
            block.get()->Value = arg->Value;

            // destroy original //
            arg->Value = NULL;

            return block.release();
        }

        // comparison operator //
        inline bool operator ==(const DataBlock<DBlockT> &other){

            // compare values with default operator //
            return *Value == *other.Value;
        }


        // value getting operators //
        template<class ConvertT>
        operator ConvertT() const{

            return DataBlockConversionResolver<DBlockT, ConvertT>::DoConversionNonPtr(this);
        }
        // explicit so that this doesn't get called all the time with invalid values and such //
        template<class ConvertT>
        /*explicit*/ operator ConvertT*(){

            return DataBlockConversionResolver<DBlockT, ConvertT>::DoConversionPtr(this);
        }

        // conversion checkers //
        template<class ConvertT>
        bool IsConversionAllowedNonPtr() const{
            // check it //
            return DataBlockConversionResolver<DBlockT, ConvertT>::IsConversionAllowedNonPtr(this);
        }

        template<class ConvertT>
        bool IsConversionAllowedPtr() const{
            // check it //
            return DataBlockConversionResolver<DBlockT, ConvertT>::IsConversionAllowedPtr(this);
        }

    //private:

        DBlockT* Value;
    };

    //! A pointer specialized version of DataBlock
    //! \todo Add support for move operators to this too
    template<class DBlockT>
    class DataBlock<DBlockT*> : public DataBlockAll{
    public:
        DataBlock() : Value(NULL){

            Type = DATABLOCK_TYPE_ERROR;
        }
        DataBlock(const DBlockT* val) : Value(val){

            // use templates to get type //
            Type = DataBlockNameResolver<DBlockT>::TVal;
        }
        DataBlock(DBlockT* val) : Value(val){

            // use templates to get type //
            Type = DataBlockNameResolver<DBlockT>::TVal;
        }
        // not actually working on ptr types //
        DataBlock(const DataBlock &otherdeepcopy) : Value(otherdeepcopy.Value){

            // copy type //
            Type = otherdeepcopy.Type;
        }

        virtual ~DataBlock(){
        }

        // deep copy operator //
        DataBlock& operator =(const DataBlock& arg){
            // copy type //
            Type = arg.Type;
            Value = arg.Value;
            // avoid performance issues //
            return *this;
        }

        // function used in deep copy //
        virtual DataBlockAll* AllocateNewFromThis() const{

            return static_cast<DataBlockAll*>((new DataBlock<DBlockT*>(const_cast<const DataBlock<DBlockT*>&>(*this))));
        }

        // shallow copy operator //
        // copies just the pointer over (fast for copies that don't need both copies //
        static inline DataBlock* CopyConstructor(DataBlock* arg){

            std::unique_ptr<DataBlock> block(new DataBlock());

            block.get()->Type = arg->Type;
            // copy pointer //
            block.get()->Value = arg->Value;

            // destroy original //
            arg->Value = NULL;

            return block.release();
        }

        // comparison operator //
        inline bool operator ==(const DataBlock<DBlockT> &other){

            // compare values with default operator //
            return Value == other.Value;
        }

        // value getting operators //
        template<class ConvertT>
        operator ConvertT() const{
            //LEVIATHAN_ASSERT(false, "data block pointer cannot be made into value");
            //return ConvertT();
            return reinterpret_cast<ConvertT>(Value);
        }
        // explicit so that this doesn't get called all the time with invalid values and such //
        template<class ConvertT>
        explicit operator ConvertT*(){

            return DataBlockConversionResolver<DBlockT*, ConvertT>::DoConversionPtr(this);
        }

        // conversion checkers //
        template<class ConvertT>
        bool IsConversionAllowedNonPtr() const{
            // check it //
            return false;
        }

        template<class ConvertT>
        bool IsConversionAllowedPtr() const{
            // check it //
            return DataBlockConversionResolver<DBlockT*, ConvertT>::IsConversionAllowedPtr(this);
        }

        DBlockT* Value;
    };



    // define specific types //

    typedef DataBlock<int> IntBlock;
    typedef DataBlock<float> FloatBlock;
    typedef DataBlock<bool> BoolBlock;
    typedef DataBlock<std::wstring> WstringBlock;
    typedef DataBlock<std::string> StringBlock;
    typedef DataBlock<char> CharBlock;
    typedef DataBlock<double> DoubleBlock;
    typedef DataBlock<void*> VoidPtrBlock;

    // template class for converting Type values to data types //
    template<int TValue>
    struct TvalToTypeResolver{

        static const DataBlockAll* Conversion(const DataBlockAll* bl){
            return bl;
        }
        static DataBlockAll* Conversion(DataBlockAll* bl){
            return bl;
        }
    };


    // type resolver specifications //
#define TVALRESOLVERTYPE(BlockTypeT, DEFINEDValT) template<> struct TvalToTypeResolver<DEFINEDValT>{\
        static const BlockTypeT* Conversion(const DataBlockAll* bl){\
            return static_cast<const BlockTypeT*>(bl);}; \
        static BlockTypeT* Conversion(DataBlockAll* bl){return static_cast<BlockTypeT*>(bl);}};

    TVALRESOLVERTYPE(IntBlock, DATABLOCK_TYPE_INT);
    TVALRESOLVERTYPE(FloatBlock, DATABLOCK_TYPE_FLOAT);
    TVALRESOLVERTYPE(BoolBlock, DATABLOCK_TYPE_BOOL);
    TVALRESOLVERTYPE(WstringBlock, DATABLOCK_TYPE_WSTRING);
    TVALRESOLVERTYPE(StringBlock, DATABLOCK_TYPE_STRING);
    TVALRESOLVERTYPE(CharBlock, DATABLOCK_TYPE_CHAR);
    TVALRESOLVERTYPE(DoubleBlock, DATABLOCK_TYPE_DOUBLE);
    TVALRESOLVERTYPE(VoidPtrBlock, DATABLOCK_TYPE_VOIDPTR);
    

    //! \brief Non-template class for working with all types of DataBlocks
    //!
    //! If you want a VariableBlock with no value use:
    //! VariableBlock(static_cast<DataBlockAll*>(nullptr)) and a cast to void if you want
    //! a VoidPtrBlock with no value
    class VariableBlock{
    public:

        //! \brief Default empty constructor, block has no value of any kind
        VariableBlock() : BlockData(NULL){

        }

        // constructors that accept any type of DataBlock //
        template<class DBRType>
        VariableBlock(DataBlock<DBRType>* block){

            BlockData = static_cast<DataBlockAll*>(block);
        }
        // constructors that accept basic types //
        VariableBlock(const int &var){
            BlockData = static_cast<DataBlockAll*>(new IntBlock(var));
        }
        VariableBlock(const bool &var, bool isactuallybooltype){
            BlockData = static_cast<DataBlockAll*>(new BoolBlock(var));
        }
        VariableBlock(const std::string &var){
            BlockData = static_cast<DataBlockAll*>(new StringBlock(var));
        }
        VariableBlock(const std::wstring &var){
            BlockData = static_cast<DataBlockAll*>(new WstringBlock(var));
        }
        VariableBlock(const double &var){
            BlockData = static_cast<DataBlockAll*>(new DoubleBlock(var));
        }
        VariableBlock(const float &var){
            BlockData = static_cast<DataBlockAll*>(new FloatBlock(var));
        }
        VariableBlock(const char &var){
            BlockData = static_cast<DataBlockAll*>(new CharBlock(var));
        }
        VariableBlock(void* var){
            BlockData = static_cast<DataBlockAll*>(new VoidPtrBlock(var));
        }
        

    #ifdef SFML_PACKETS

        //! \brief Constructs from a packet
        DLLEXPORT VariableBlock(sf::Packet &packet);


        //! \brief Stores data to a packet
        DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

    #endif //SFML_PACKETS

        // deep copy constructor //
        VariableBlock(const VariableBlock &arg){
            // copy data //
            BlockData = arg.BlockData->AllocateNewFromThis();
        }

        // constructor for creating this from std::wstring //
        DLLEXPORT VariableBlock(const std::string &valuetoparse, std::map<std::string,
            std::shared_ptr<VariableBlock>>* predefined);

        // non template constructor //
        VariableBlock(DataBlockAll* block){

            BlockData = block;
        }
        // destructor that releases data //
        virtual ~VariableBlock(){

            SAFE_DELETE(BlockData);
        }

        // getting function //
        inline DataBlockAll* GetBlock(){

            return BlockData;
        }

        inline const DataBlockAll* GetBlockConst() const{

            return static_cast<const DataBlockAll*>(BlockData);
        }

        // operators //
        // Checks is this valid //
        bool IsValid() const {

            if (!BlockData)
                return false;

            return true;
        }

        // copy operators //
        // shallow copy (when both instances aren't wanted //
        VariableBlock& operator =(VariableBlock* arg){
            // release existing value (if any) //
            if(BlockData){
                SAFE_DELETE(BlockData);
            }

            // copy pointer //
            BlockData = arg->BlockData;

            // destroy original //
            arg->BlockData = NULL;

            // avoid performance issues //
            return *this;
        }
        
        template<class DBlockTP>
        VariableBlock& operator =(DataBlock<DBlockTP>* arg){
            // release existing value (if any) //
            if(BlockData){
                SAFE_DELETE(BlockData);
            }

            // copy pointer //
            BlockData = static_cast<DataBlockAll*>(arg);

            // avoid performance issues //
            return *this;
        }

        // deep copy //
        VariableBlock& operator =(const VariableBlock &arg){
            // release existing value (if any) //
            if(BlockData){
                SAFE_DELETE(BlockData);
            }

            // copy data //
            BlockData = arg.BlockData->AllocateNewFromThis();

            // avoid performance issues //
            return *this;
        }

        //! \brief comparison operator
        inline bool operator ==(const VariableBlock &other) const{
            // returns false if either block is NULL //
            if(BlockData == NULL || other.BlockData == NULL)
                return false;
            // if different types cannot match //
            if(BlockData->Type != other.BlockData->Type)
                return false;
            // need to check if values match //
            if(BlockData->Type == DATABLOCK_TYPE_INT)
                return *TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(other.BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
                return *TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(other.BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
                return *TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(other.BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
                return *TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(other.BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_STRING)
                return *TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(other.BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
                return *TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(other.BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
                return *TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData) ==
                    *TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(other.BlockData);

            // type that shouldn't be used is used //
            LEVIATHAN_ASSERT(0, "unallowed datatype in datablock");
            return false;
        }

        //! \brief Opposite of the comparison operator
        inline bool operator !=(const VariableBlock &other) const{

            return !(*this == other);
        }


        // templated operators //
        template<class ConvertT>
        inline operator ConvertT() const{
            // cast DataBlock to derived type //
            if(BlockData->Type == DATABLOCK_TYPE_INT)
                return *TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
                return *TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
                return *TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
                return *TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_STRING)
                return *TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
                return *TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData);
            else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
                return *TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData);

            // type that shouldn't be used is used //
            LEVIATHAN_ASSERT(0, "unallowed datatype in datablock");
            return ConvertT();
        }
        template<class ConvertT>
        inline operator ConvertT*(){
            // check does types match //
            if(DataBlockNameResolver<ConvertT>::TVal == BlockData->Type){

                if(BlockData->Type == DATABLOCK_TYPE_INT)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_STRING)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData);
                else if(BlockData->Type == DATABLOCK_TYPE_VOIDPTR)
                    return *TvalToTypeResolver<DATABLOCK_TYPE_VOIDPTR>::Conversion(BlockData);
            }
            // non matching types //
            LEVIATHAN_ASSERT(0, "unallowed cast from type to another with return pointer");
            return NULL;
        }


        // verifying functions //

        // conversion checkers //
        template<class ConvertT>
        inline bool IsConversionAllowedNonPtr() const{
            // check it //
            if(BlockData->Type == DATABLOCK_TYPE_INT)
                return TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
                return TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
                return TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
                return TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_STRING)
                return TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
                return TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
                return TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_VOIDPTR)
                return TvalToTypeResolver<DATABLOCK_TYPE_VOIDPTR>::Conversion(BlockData)->
                    IsConversionAllowedNonPtr<ConvertT>();

            LEVIATHAN_ASSERT(0, "invalid datablock type");
            return false;
        }

        template<class ConvertT>
        inline bool IsConversionAllowedPtr() const{
            // check it //
            if(BlockData->Type == DATABLOCK_TYPE_INT)
                return TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
                return TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
                return TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
                return TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_STRING)
                return TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
                return TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
                return TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();
            else if(BlockData->Type == DATABLOCK_TYPE_VOIDPTR)
                return TvalToTypeResolver<DATABLOCK_TYPE_VOIDPTR>::Conversion(BlockData)->
                    IsConversionAllowedPtr<ConvertT>();

            LEVIATHAN_ASSERT(0, "invalid datablock type");
            return false;
        }

        // templated assignment conversion operator //
        template<class ConvertT>
        inline bool ConvertAndAssingToVariable(ConvertT &var) const{
            // return if not allowed conversion //
            if(!IsConversionAllowedNonPtr<ConvertT>()){
                //Logger::Get()->Warning(L"VariableBlock: conversion not allowed");
                return false;
            }
            // assign directly to the wanted value, should be faster than converting returning and then assigning //
            var = this->operator ConvertT();

            // assignment succeeded //
            return true;
        }

        //! \todo Throw an exception if unallowed
        template<class ConvertT>
        inline ConvertT ConvertAndReturnVariable() const{
            // return if not allowed conversion //
            if(!IsConversionAllowedNonPtr<ConvertT>()){
                //Logger::Get()->Warning(L"VariableBlock: conversion not allowed");
                return ConvertT(0);
            }
            
            // return conversion result //
            return this->operator ConvertT();
        }

    protected:
        // data storing //
        DataBlockAll* BlockData = nullptr;

    };


    //! \brief DataBlock variant with name
    class NamedVariableBlock : public VariableBlock{
    public:
        // constructors that accept any type of DataBlock //
        template<class DBRType>
        NamedVariableBlock(DataBlock<DBRType>* block, const std::string &name) :
            VariableBlock(block), Name(name)
        {

        }
        
        // non template constructor //
        NamedVariableBlock(DataBlockAll* block, const std::string &name) :
            VariableBlock(block), Name(name)
        {

        }

        // constructors that accept basic types //
        NamedVariableBlock(const int &var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }
        NamedVariableBlock(const bool &var, bool isactuallybooltype, const std::string &name) :
            VariableBlock(var, isactuallybooltype), Name(name)
        {
        }
        NamedVariableBlock(const std::string &var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }
        NamedVariableBlock(const std::wstring &var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }
        NamedVariableBlock(const double &var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }
        NamedVariableBlock(const float &var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }
        NamedVariableBlock(const char &var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }
        NamedVariableBlock(void* var, const std::string &name) :
            VariableBlock(var), Name(name)
        {
        }        

        inline std::string GetName() const{
            return Name;
        }

        inline bool CompareName(const std::string &str) const{

            return Name == str;
        }

        inline std::string& GetNameChangeable(){

            return Name;
        }

    protected:

        std::string Name;
    };

#ifdef LEVIATHAN_USING_ANGELSCRIPT
    //! \brief Reference counted version for scripts of VariableBlock
    //!
    //! Also stores the AngelScript ID of the type
    //! \note Do NOT use smart pointers with this class
    //! \todo Unify multiple values containing things and naming
    //! \todo This whole thing should be made again and the types should be integrated to
    //! a single VariableBlock class that has a void* and an angelscript type
    class ScriptSafeVariableBlock : public NamedVariableBlock, public ReferenceCounted{
    public:

        template<class BlockBaseType>
        ScriptSafeVariableBlock(DataBlock<BlockBaseType>* block,
            const std::string &name) :
            NamedVariableBlock(block, name)
        {
            // getting typeid //
            ASTypeID = TypeToAngelScriptIDConverter<BlockBaseType>::GetTypeIDFromTemplated();
        }

        DLLEXPORT ScriptSafeVariableBlock(VariableBlock* copyfrom, const std::string &name);


        bool IsValidType(){
            return ASTypeID > 0 ? true: false;
        }

        // script proxy functions //
        int ConvertAndReturnProxyInt(){
            return ConvertAndReturnVariable<int>();
        }
        std::string ConvertAndReturnProxyString(){

            return ConvertAndReturnVariable<std::string>();
        }

    protected:

        int ASTypeID;
    };
#endif //LEVIATHAN_USING_ANGELSCRIPT

    // Stream operators //
    DLLEXPORT std::ostream& operator <<(std::ostream &stream,
        const Leviathan::VariableBlock &value);


    // conversion template specifications //
#define CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(BlockTypeName, ToConvertTypeName, ConvertActionToDo) template<> \
    class DataBlockConverter<BlockTypeName, ToConvertTypeName>{public: \
        static inline ToConvertTypeName DoConvert(const BlockTypeName* block){ \
            return ConvertActionToDo;}; \
        static const bool AllowedConversion = true;};
#define CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(BlockTypeName, ToConvertTypeName) \
    template<> class DataBlockConverter<BlockTypeName, ToConvertTypeName>{public: \
        static inline ToConvertTypeName DoConvert(const BlockTypeName* block){ \
            return (ToConvertTypeName)(*block->Value);}; \
        static const bool AllowedConversion = true;};

    // std::wstring and std::string conversions with templates //
    template<class FromDataBlockType>
    class DataBlockConverter<FromDataBlockType, std::wstring>{
    public:
        static inline std::wstring DoConvert(const FromDataBlockType* block){
            return std::to_wstring(*block->Value);
        }
        static const bool AllowedConversion = true;
    };
    template<class FromDataBlockType>
    class DataBlockConverter<FromDataBlockType, std::string>{
    public:
        static inline std::string DoConvert(const FromDataBlockType* block){
            return Convert::ToString(*block->Value);
        }
        static const bool AllowedConversion = true;
    };




    // ------------------ IntBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(IntBlock, int, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(IntBlock, bool, (*block->Value) != 0);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(IntBlock, float);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(IntBlock, double);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(IntBlock, char);

    // ------------------ FloatBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(FloatBlock, float, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(FloatBlock, bool, (*block->Value) != 0);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(FloatBlock, int);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(FloatBlock, double);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(FloatBlock, char);
    // ------------------ BoolBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(BoolBlock, bool, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(BoolBlock, int);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(BoolBlock, double);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(BoolBlock, char);

    // ------------------ CharBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(CharBlock, char, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(CharBlock, bool, (*block->Value) != 0);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(CharBlock, int);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(CharBlock, double);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(CharBlock, float);
    // ------------------ DoubleBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(DoubleBlock, double, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(DoubleBlock, bool, (*block->Value) != 0);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(DoubleBlock, int);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(DoubleBlock, char);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(DoubleBlock, float);

    // little different std::string block definitions //
    // ------------------ WstringBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, std::wstring, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, std::string,
        Convert::Utf16ToUtf8(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, bool,
        Convert::WstringFromBoolToInt(*block->Value) != 0);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, float,
        Convert::WstringTo<float>(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, double,
        Convert::WstringTo<double>(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, char,
        (char)Convert::WstringTo<wchar_t>(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, int,
        Convert::WstringTo<int>(*block->Value));
    //// ------------------ StringBlock conversions ------------------ //
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, std::string, (*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, std::wstring,
        Convert::Utf8ToUtf16(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, bool,
        Convert::StringFromBoolToInt(*block->Value) != 0);
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, float,
        Convert::StringTo<float>(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, double,
        Convert::StringTo<double>(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, char,
        Convert::StringTo<char>(*block->Value));
    CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, int,
        Convert::StringTo<int>(*block->Value));

}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::VariableBlock;
#endif

