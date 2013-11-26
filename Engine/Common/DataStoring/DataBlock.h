#ifndef LEVIATHAN_DATABLOCK
#define LEVIATHAN_DATABLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
#include "Common\ReferenceCounted.h"
// ------------------------------------ //
// ---- includes ---- //

#define DATABLOCK_TYPE_INT		3
#define DATABLOCK_TYPE_FLOAT	4
#define DATABLOCK_TYPE_BOOL		5
#define DATABLOCK_TYPE_WSTRING	6
#define DATABLOCK_TYPE_STRING	7
#define DATABLOCK_TYPE_CHAR		8
#define DATABLOCK_TYPE_DOUBLE	9
#define DATABLOCK_TYPE_OBJECTL	10 // uses Leviathan::Object* as type
#define DATABLOCK_TYPE_VOIDPTR	11

#define DATABLOCK_TYPE_ERROR	9000

namespace Leviathan{

	// testing function (for some purpose) //
	DLLEXPORT bool DataBlockTestVerifier(const int &tests);

	// forward declaration //
	template<class DBlockT> class DataBlock;

	// template struct for getting right type for right type...
	template<class T>
	struct DataBlockNameResolver{

		static const int TVal = DATABLOCK_TYPE_ERROR;
	};
	// specification values //

#define NAMERESOLVERTEMPLATEINSTANTIATION(DType, TVALDEFINE) template<> struct DataBlockNameResolver<DType>{ static const int TVal = TVALDEFINE;};

	NAMERESOLVERTEMPLATEINSTANTIATION(int, DATABLOCK_TYPE_INT);
	NAMERESOLVERTEMPLATEINSTANTIATION(float, DATABLOCK_TYPE_FLOAT);
	NAMERESOLVERTEMPLATEINSTANTIATION(bool, DATABLOCK_TYPE_BOOL);
	NAMERESOLVERTEMPLATEINSTANTIATION(wstring, DATABLOCK_TYPE_WSTRING);
	NAMERESOLVERTEMPLATEINSTANTIATION(string, DATABLOCK_TYPE_STRING);
	NAMERESOLVERTEMPLATEINSTANTIATION(char, DATABLOCK_TYPE_CHAR);
	NAMERESOLVERTEMPLATEINSTANTIATION(double, DATABLOCK_TYPE_DOUBLE);
	NAMERESOLVERTEMPLATEINSTANTIATION(Object*, DATABLOCK_TYPE_OBJECTL);
	NAMERESOLVERTEMPLATEINSTANTIATION(void*, DATABLOCK_TYPE_VOIDPTR);
	// for conversion check to work //
	NAMERESOLVERTEMPLATEINSTANTIATION(Object, DATABLOCK_TYPE_OBJECTL);
	NAMERESOLVERTEMPLATEINSTANTIATION(void, DATABLOCK_TYPE_VOIDPTR);

	// static class used to convert DataBlocks from different types to other types //
	template<class FromDataBlockType, class TargetType>
	class DataBlockConverter{
	public:
		// conversion function that templates overload //
		static inline TargetType DoConvert(const FromDataBlockType* block){
//#pragma message ("non valid conversion from types")
			assert(0 && "conversion not possible");
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
				return block->Value;
			}
			// cannot return converted value //
//#pragma message ("cannot return pointer from converted type")
			assert(0 && "return pointer of converted value not possible");
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
		DLLEXPORT inline virtual ~DataBlockAll(){


		}

		// function used in deep copy //
		DLLEXPORT virtual DataBlockAll* AllocateNewFromThis() const = 0;


		// comparison operator //
		DLLEXPORT inline bool operator ==(const DataBlockAll &other){

			// just compare types here //
			return Type == other.Type;
		}

		int Type;
	protected:
		// private constructor to make sure that no instances of this class exist //
		DataBlockAll(){

		}

	};


	// main DataBlock class //
	template<class DBlockT>
	class DataBlock : public DataBlockAll{
	public:
		DLLEXPORT DataBlock() : Value(NULL){

			Type = DATABLOCK_TYPE_ERROR;
		}
		DLLEXPORT DataBlock(const DBlockT &val) : Value(new DBlockT(val)){

			// use templates to get type //
			Type = DataBlockNameResolver<DBlockT>::TVal;
		}
		DLLEXPORT DataBlock(DBlockT* val) : Value(val){

			// use templates to get type //
			Type = DataBlockNameResolver<DBlockT>::TVal;
		}

		DLLEXPORT DataBlock(const DataBlock &otherdeepcopy) : Value(NULL){
			// allocate new pointer from the other instance //
			Value = new DBlockT(*otherdeepcopy.Value);

			// copy type //
			Type = otherdeepcopy.Type;
		}
		

		DLLEXPORT virtual ~DataBlock(){
			// erase memory //
			SAFE_DELETE(Value);
		}
		// deep copy operator //
		DLLEXPORT DataBlock& operator =(const DataBlock& arg){
			// release existing value (if any) //
			SAFE_DELETE(Value);

			// copy type //
			Type = arg.Type;
			// skip if other is null value //
			if(arg.Value == NULL)
				return;
			// deep copy //
			Value = new DBlockT(*arg.Value);

			// avoid performance issues //
			return *this;
		}

		// function used in deep copy //
		DLLEXPORT virtual DataBlockAll* AllocateNewFromThis() const{

			return (DataBlockAll*)(new DataBlock<DBlockT>(const_cast<const DataBlock<DBlockT>&>(*this)));
		}

		// shallow copy operator //
		// copies just the pointer over (fast for copies that don't need both copies //
		DLLEXPORT static inline DataBlock* CopyConstructor(DataBlock* arg){

			unique_ptr<DataBlock> block(new DataBlock());

			block.get()->Type = arg->Type;
			// copy pointer //
			block.get()->Value = arg->Value;

			// destroy original //
			arg->Value = NULL;

			return block.release();
		}

		// comparison operator //
		DLLEXPORT inline bool operator ==(const DataBlock<DBlockT> &other){

			// compare values with default operator //
			return *Value == *other.Value;
		}


		// value getting operators //
		template<class ConvertT>
		DLLEXPORT operator ConvertT() const{

			return DataBlockConversionResolver<DBlockT, ConvertT>::DoConversionNonPtr(this);
		}
		// explicit so that this doesn't get called all the time with invalid values and such //
		template<class ConvertT>
		DLLEXPORT /*explicit*/ operator ConvertT*(){

			return DataBlockConversionResolver<DBlockT, ConvertT>::DoConversionPtr(this);
		}

		// conversion checkers //
		template<class ConvertT>
		DLLEXPORT bool IsConversionAllowedNonPtr() const{
			// check it //
			return DataBlockConversionResolver<DBlockT, ConvertT>::IsConversionAllowedNonPtr(this);
		}

		template<class ConvertT>
		DLLEXPORT bool IsConversionAllowedPtr() const{
			// check it //
			return DataBlockConversionResolver<DBlockT, ConvertT>::IsConversionAllowedPtr(this);
		}

	//private:

		DBlockT* Value;
	};

	// pointer specialized version //
	template<class DBlockT>
	class DataBlock<DBlockT*> : public DataBlockAll{
	public:
		DLLEXPORT DataBlock() : Value(NULL){

			Type = DATABLOCK_TYPE_ERROR;
		}
		DLLEXPORT DataBlock(const DBlockT* val) : Value(val){

			// use templates to get type //
			Type = DataBlockNameResolver<DBlockT>::TVal;
		}
		DLLEXPORT DataBlock(DBlockT* val) : Value(val){

			// use templates to get type //
			Type = DataBlockNameResolver<DBlockT>::TVal;
		}
		// not actually working on ptr types //
		DLLEXPORT DataBlock(const DataBlock &otherdeepcopy) : Value(otherdeepcopy.Value){

			// copy type //
			Type = otherdeepcopy.Type;
		}

		DLLEXPORT virtual ~DataBlock(){
		}

		// deep copy operator //
		DLLEXPORT DataBlock& operator =(const DataBlock& arg){
			// copy type //
			Type = arg.Type;
			Value = arg.Value;
			// avoid performance issues //
			return *this;
		}

		// function used in deep copy //
		DLLEXPORT virtual DataBlockAll* AllocateNewFromThis() const{

			return (DataBlockAll*)(new DataBlock<DBlockT*>(const_cast<const DataBlock<DBlockT*>&>(*this)));
		}

		// shallow copy operator //
		// copies just the pointer over (fast for copies that don't need both copies //
		DLLEXPORT static inline DataBlock* CopyConstructor(DataBlock* arg){

			unique_ptr<DataBlock> block(new DataBlock());

			block.get()->Type = arg->Type;
			// copy pointer //
			block.get()->Value = arg->Value;

			// destroy original //
			arg->Value = NULL;

			return block.release();
		}

		// comparison operator //
		DLLEXPORT inline bool operator ==(const DataBlock<DBlockT> &other){

			// compare values with default operator //
			return Value == other.Value;
		}

		// value getting operators //
		template<class ConvertT>
		DLLEXPORT operator ConvertT() const{
			assert(true && "datablock pointer cannot be made into value");
			return ConvertT();
		}
		// explicit so that this doesn't get called all the time with invalid values and such //
		template<class ConvertT>
		DLLEXPORT /*explicit*/ operator ConvertT*(){

			return DataBlockConversionResolver<DBlockT*, ConvertT>::DoConversionPtr(this);
		}

		// conversion checkers //
		template<class ConvertT>
		DLLEXPORT bool IsConversionAllowedNonPtr() const{
			// check it //
			return false;
		}

		template<class ConvertT>
		DLLEXPORT bool IsConversionAllowedPtr() const{
			// check it //
			return DataBlockConversionResolver<DBlockT*, ConvertT>::IsConversionAllowedPtr(this);
		}

		DBlockT* Value;
	};
	


	// define specific types //

	typedef DataBlock<int>			IntBlock;
	typedef DataBlock<float>		FloatBlock;
	typedef DataBlock<bool>			BoolBlock;
	typedef DataBlock<wstring>		WstringBlock;
	typedef DataBlock<string>		StringBlock;
	typedef DataBlock<char>			CharBlock;
	typedef DataBlock<double>		DoubleBlock;
	typedef DataBlock<Object*>		LeviathanObjectBlock;
	typedef DataBlock<void*>		VoidPtrBlock;

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
#define TVALRESOLVERTYPE(BlockTypeT, DEFINEDValT) template<> struct TvalToTypeResolver<DEFINEDValT>{ static const BlockTypeT* Conversion(const DataBlockAll* bl){return static_cast<const BlockTypeT*>(bl);}; static BlockTypeT* Conversion(DataBlockAll* bl){return static_cast<BlockTypeT*>(bl);}};

	TVALRESOLVERTYPE(IntBlock, DATABLOCK_TYPE_INT);
	TVALRESOLVERTYPE(FloatBlock, DATABLOCK_TYPE_FLOAT);
	TVALRESOLVERTYPE(BoolBlock, DATABLOCK_TYPE_BOOL);
	TVALRESOLVERTYPE(WstringBlock, DATABLOCK_TYPE_WSTRING);
	TVALRESOLVERTYPE(StringBlock, DATABLOCK_TYPE_STRING);
	TVALRESOLVERTYPE(CharBlock, DATABLOCK_TYPE_CHAR);
	TVALRESOLVERTYPE(DoubleBlock, DATABLOCK_TYPE_DOUBLE);
	TVALRESOLVERTYPE(LeviathanObjectBlock, DATABLOCK_TYPE_OBJECTL);
	TVALRESOLVERTYPE(VoidPtrBlock, DATABLOCK_TYPE_VOIDPTR);

	// forward declaration to make one of the constructors work //
	class NamedVariableBlock;


	// DataBlock interface classes //
	class VariableBlock : public Object{
	public:
		// constructors that accept any type of DataBlock //
		template<class DBRType>
		DLLEXPORT VariableBlock(DataBlock<DBRType>* block){

			BlockData = (DataBlockAll*)block;
		}
		// constructors that accept basic types //
		DLLEXPORT VariableBlock(const int &var){
			BlockData = (DataBlockAll*)new IntBlock(var);
		}
		DLLEXPORT VariableBlock(const bool &var){
			BlockData = (DataBlockAll*)new BoolBlock(var);
		}
		DLLEXPORT VariableBlock(const string &var){
			BlockData = (DataBlockAll*)new StringBlock(var);
		}
		DLLEXPORT VariableBlock(const wstring &var){
			BlockData = (DataBlockAll*)new WstringBlock(var);
		}
		DLLEXPORT VariableBlock(const double &var){
			BlockData = (DataBlockAll*)new DoubleBlock(var);
		}
		DLLEXPORT VariableBlock(const float &var){
			BlockData = (DataBlockAll*)new FloatBlock(var);
		}
		DLLEXPORT VariableBlock(const char &var){
			BlockData = (DataBlockAll*)new CharBlock(var);
		}
		




		// deep copy constructor //
		DLLEXPORT VariableBlock(const VariableBlock &arg){
			// copy data //
			BlockData = arg.BlockData->AllocateNewFromThis();
		}

		// constructor for creating this from wstring //
		DLLEXPORT VariableBlock(wstring &valuetoparse, map<wstring, shared_ptr<VariableBlock>>* predefined = NULL) throw(...);

		// non template constructor //
		DLLEXPORT VariableBlock(DataBlockAll* block){

			BlockData = block;
		}
		// destructor that releases data //
		DLLEXPORT virtual ~VariableBlock(){

			SAFE_DELETE(BlockData);
		}

		// getting function //
		DLLEXPORT inline DataBlockAll* GetBlock(){

			return BlockData;
		}

		DLLEXPORT inline const DataBlockAll* GetBlockConst() const{

			return static_cast<const DataBlockAll*>(BlockData);
		}

		// operators //
		// copy operators //
		// shallow copy (when both instances aren't wanted //
		DLLEXPORT VariableBlock& operator =(VariableBlock* arg){
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
		DLLEXPORT VariableBlock& operator =(DataBlock<DBlockTP>* arg){
			// release existing value (if any) //
			if(BlockData){
				SAFE_DELETE(BlockData);
			}

			// copy pointer //
			BlockData = (DataBlockAll*)arg;

			// avoid performance issues //
			return *this;
		}

		// deep copy //
		DLLEXPORT VariableBlock& operator =(const VariableBlock &arg){
			// release existing value (if any) //
			if(BlockData){
				SAFE_DELETE(BlockData);
			}

			// copy data //
			BlockData = arg.BlockData->AllocateNewFromThis();

			// avoid performance issues //
			return *this;
		}

		// comparison operator //
		DLLEXPORT inline bool operator ==(const VariableBlock &other){
			// returns false if either block is NULL //
			if(BlockData == NULL || other.BlockData == NULL)
				return false;
			// if different types cannot match //
			if(BlockData->Type != other.BlockData->Type)
				return false;
			// need to check if values match //
			if(BlockData->Type == DATABLOCK_TYPE_INT)
				return *TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(other.BlockData);
			else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
				return *TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(other.BlockData);
			else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
				return *TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(other.BlockData);
			else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
				return *TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(other.BlockData);
			else if(BlockData->Type == DATABLOCK_TYPE_STRING)
				return *TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(other.BlockData);
			else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
				return *TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(other.BlockData);
			else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
				return *TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData) == *TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(other.BlockData);

			// type that shouldn't be used is used //
			assert(0 && "unallowed datatype in datablock");
			return false;
		}


		// templated operators //
		template<class ConvertT>
		DLLEXPORT inline operator ConvertT() const{
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
			assert(0 && "unallowed datatype in datablock");
			return ConvertT();
		}
		template<class ConvertT>
		DLLEXPORT inline operator ConvertT*(){
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
				else if(BlockData->Type == DATABLOCK_TYPE_OBJECTL)
					return *TvalToTypeResolver<DATABLOCK_TYPE_OBJECTL>::Conversion(BlockData);
				else if(BlockData->Type == DATABLOCK_TYPE_VOIDPTR)
					return *TvalToTypeResolver<DATABLOCK_TYPE_VOIDPTR>::Conversion(BlockData);
			}
			// non matching types //
			assert(0 && "unallowed cast from type to another with return pointer");
			return NULL;
		}


		// verifying functions //

		// conversion checkers //
		template<class ConvertT>
		DLLEXPORT inline bool IsConversionAllowedNonPtr() const{
			// return if null //
			if(this == NULL)
				return false;
			// check it //
			if(BlockData->Type == DATABLOCK_TYPE_INT)
				return TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
				return TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
				return TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
				return TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_STRING)
				return TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
				return TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
				return TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_OBJECTL)
				return TvalToTypeResolver<DATABLOCK_TYPE_OBJECTL>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_VOIDPTR)
				return TvalToTypeResolver<DATABLOCK_TYPE_VOIDPTR>::Conversion(BlockData)->IsConversionAllowedNonPtr<ConvertT>();

			assert(0 && "invalid datablock type");
			return false;
		}

		template<class ConvertT>
		DLLEXPORT inline bool IsConversionAllowedPtr() const{
			// check it //
			if(BlockData->Type == DATABLOCK_TYPE_INT)
				return TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
				return TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
				return TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
				return TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_STRING)
				return TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
				return TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
				return TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_OBJECTL)
				return TvalToTypeResolver<DATABLOCK_TYPE_OBJECTL>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();
			else if(BlockData->Type == DATABLOCK_TYPE_VOIDPTR)
				return TvalToTypeResolver<DATABLOCK_TYPE_VOIDPTR>::Conversion(BlockData)->IsConversionAllowedPtr<ConvertT>();

			assert(0 && "invalid datablock type");
			return false;
		}

		// templated assignment conversion operator //
		template<class ConvertT>
		DLLEXPORT inline bool ConvertAndAssingToVariable(ConvertT &var) const{
			// return if not allowed conversion //
			if(!IsConversionAllowedNonPtr<ConvertT>()){
				//Logger::Get()->Warning(L"VariableBlock: conversion not allowed");
				return false;
			}
			// assign directly to the wanted value, should be faster than converting returning and then assigning //
			var = (ConvertT)*this;
			// assignment succeeded //
			return true;
		}

		template<class ConvertT>
		DLLEXPORT inline ConvertT ConvertAndReturnVariable() const{
			// return if not allowed conversion //
			if(!IsConversionAllowedNonPtr<ConvertT>()){
				//Logger::Get()->Warning(L"VariableBlock: conversion not allowed");
				return ConvertT(0);
			}
			// return conversion result //
			return (ConvertT)*this;
		}

	protected:
		// data storing //
		DataBlockAll* BlockData;

	};


	// variant with name //
	class NamedVariableBlock : public VariableBlock{
	public:
		// constructors that accept any type of DataBlock //
		template<class DBRType>
		DLLEXPORT NamedVariableBlock(DataBlock<DBRType>* block, const wstring &name): VariableBlock(block), Name(name){

		}
		// non template constructor //
		DLLEXPORT NamedVariableBlock(DataBlockAll* block, const wstring &name) : VariableBlock(block), Name(name){

		}

		DLLEXPORT inline wstring GetName() const{
			return Name;
		}

		DLLEXPORT inline bool CompareName(const wstring &str) const{

			return Name == str;
		}

		DLLEXPORT inline wstring& GetNameChangeable(){

			return Name;
		}

	protected:

		wstring Name;
	};

	// NOTE: Do NOT use smart pointers with this class //
	// reference counted version for scripts //
	class ScriptSafeVariableBlock : public NamedVariableBlock, public ReferenceCounted{
	public:

		template<class BlockBaseType>
		DLLEXPORT ScriptSafeVariableBlock(DataBlock<BlockBaseType>* block, const wstring &name) : NamedVariableBlock(block, name){
			// getting typeid //
			ASTypeID = TypeToAngelScriptIDConverter<BlockBaseType>::GetTypeIDFromTemplated();
		}

		DLLEXPORT ScriptSafeVariableBlock(VariableBlock* copyfrom, const wstring &name);


		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(ScriptSafeVariableBlock);


		DLLEXPORT bool IsValidType(){
			return ASTypeID > 0 ? true: false;
		}

		// script proxy functions //
		int ConvertAndReturnProxyInt(){
			return ConvertAndReturnVariable<int>();
		}
		string ConvertAndReturnProxyString(){

			return ConvertAndReturnVariable<string>();
		}

		// More script proxies //
		ScriptSafeVariableBlock* CreateNewWstringProxy(){

			// Try to convert our block //
			wstring wstrval;

			if(ConvertAndAssingToVariable(wstrval)){

				return new ScriptSafeVariableBlock(new WstringBlock(wstrval), Name);
			}
			// Conversion failed //
			return NULL;
		}

	protected:


		int ASTypeID;
	};



	// conversion template specifications //
#define CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(BlockTypeName, ToConvertTypeName, ConvertActionToDo) template<> class DataBlockConverter<BlockTypeName, ToConvertTypeName>{public: static inline ToConvertTypeName DoConvert(const BlockTypeName* block){ return ConvertActionToDo;}; static const bool AllowedConversion = true;};
#define CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(BlockTypeName, ToConvertTypeName) template<> class DataBlockConverter<BlockTypeName, ToConvertTypeName>{public: static inline ToConvertTypeName DoConvert(const BlockTypeName* block){ return (ToConvertTypeName)(*block->Value);}; static const bool AllowedConversion = true;};

	// wstring and string conversions with templates //
	template<class FromDataBlockType> 
	class DataBlockConverter<FromDataBlockType, wstring>{
	public:
		static inline wstring DoConvert(const FromDataBlockType* block){
			return Convert::ToWstring(*block->Value);
		}
		static const bool AllowedConversion = true;
	};
	template<class FromDataBlockType> 
	class DataBlockConverter<FromDataBlockType, string>{
	public:
		static inline string DoConvert(const FromDataBlockType* block){
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

	// little different string block definitions //
	// ------------------ WstringBlock conversions ------------------ //
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, wstring, (*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, string, Convert::WstringToString(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, bool, Convert::WstringFromBoolToInt(*block->Value) != 0);
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, float, Convert::WstringTo<float>(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, double, Convert::WstringTo<double>(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, char, (char)Convert::WstringTo<wchar_t>(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(WstringBlock, int, Convert::WstringTo<int>(*block->Value));
	//// ------------------ StringBlock conversions ------------------ //
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, string, (*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, wstring, Convert::StringToWstring(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, bool, Convert::StringFromBoolToInt(*block->Value) != 0);
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, float, Convert::StringTo<float>(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, double, Convert::StringTo<double>(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, char, Convert::StringTo<char>(*block->Value));
	CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(StringBlock, int, Convert::StringTo<int>(*block->Value));




}
#endif