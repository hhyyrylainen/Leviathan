#ifndef LEVIATHAN_DATABLOCK
#define LEVIATHAN_DATABLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
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


	// static class used to convert DataBlocks from different types to other types //
	template<class FromDataBlockType, class TargetType>
	class DataBlockConverter{
	public:
		// conversion function that templates overload //
		static inline TargetType DoConvert(FromDataBlockType* block){
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
		static inline T DoConversionNonPtr(DataBlock<DBlockTDT>* block){
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
		}
		// functions used to check is conversion allowed //
		static inline bool IsConversionAllowedNonPtr(DataBlock<DBlockTDT>* block){
			// check types //
			if(DataBlockNameResolver<T>::TVal == block->Type){
				// same type, is allowed always //
				return true;
			}
			// use templates to see if there is a template that allows this //
			return DataBlockConverter<DataBlock<DBlockTDT>, T>::AllowedConversion;
		}
		static inline bool IsConversionAllowedPtr(DataBlock<DBlockTDT>* block){
			// check types //
			if(DataBlockNameResolver<T>::TVal == block->Type){
				// same type, is allowed always //
				return true;
			}
			// different types, cannot be returned as pointer //
			return false;
		}
	};

	// non template base class for pointers //
	class DataBlockAll{
	public:
		// virtual destructor for deleting through base pointers //
		DLLEXPORT inline virtual ~DataBlockAll(){


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
		DLLEXPORT virtual ~DataBlock(){
			// erase memory //
			SAFE_DELETE(Value);
		}
		// deep copy operator //
		DLLEXPORT DataBlock& operator =(const DataBlock& arg){
			// release existing value (if any) //
			if(Value){
				SAFE_DELETE(Value);
			}
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

		// value getting operators //
		template<class ConvertT>
		DLLEXPORT operator ConvertT(){

			return DataBlockConversionResolver<DBlockT, ConvertT>::DoConversionNonPtr(this);
		}
		template<class ConvertT>
		DLLEXPORT operator ConvertT*(){

			return DataBlockConversionResolver<DBlockT, ConvertT>::DoConversionPtr(this);
		}

		// conversion checkers //
		template<class ConvertT>
		DLLEXPORT bool IsConversionAllowedNonPtr(){
			// check it //
			return DataBlockConversionResolver<DBlockT, ConvertT>::IsConversionAllowedNonPtr(this);
		}

		template<class ConvertT>
		DLLEXPORT bool IsConversionAllowedPtr(){
			// check it //
			return DataBlockConversionResolver<DBlockT, ConvertT>::IsConversionAllowedPtr(this);
		}

	//private:

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

		static DataBlockAll* Conversion(DataBlockAll* bl){
			return bl;
		}
	};



	// DataBlock interface classes //
	class VariableBlock : public Object{
	public:
		// constructors that accept any type of DataBlock //
		template<class DBRType>
		DLLEXPORT VariableBlock(DataBlock<DBRType>* block){

			BlockData = (DataBlockAll*)block;
		}
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


		// templated operators //
		template<class ConvertT>
		DLLEXPORT inline operator ConvertT(){
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
			if(DataBlockNameResolver<T>::TVal == BlockData->Type){

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
		}


		// verifying functions //

		// conversion checkers //
		template<class ConvertT>
		DLLEXPORT inline bool IsConversionAllowedNonPtr(){
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
		DLLEXPORT inline bool IsConversionAllowedPtr(){
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



	// conversion template specifications //
#define CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCK(BlockTypeName, ToConvertTypeName, ConvertActionToDo) template<> class DataBlockConverter<BlockTypeName, ToConvertTypeName>{public: static inline ToConvertTypeName DoConvert(BlockTypeName* block){ return ConvertActionToDo;}; static const bool AllowedConversion = true;};
#define CONVERSIONTEMPLATESPECIFICATIONFORDATABLOCKDEFAULT(BlockTypeName, ToConvertTypeName) template<> class DataBlockConverter<BlockTypeName, ToConvertTypeName>{public: static inline ToConvertTypeName DoConvert(BlockTypeName* block){ return (ToConvertTypeName)(*block->Value);}; static const bool AllowedConversion = true;};

	// wstring and string conversions with templates //
	template<class FromDataBlockType> 
	class DataBlockConverter<FromDataBlockType, wstring>{
	public:
		static inline wstring DoConvert(FromDataBlockType* block){
			return Convert::ToWstring(*block->Value);
		}
		static const bool AllowedConversion = true;
	};
	template<class FromDataBlockType> 
	class DataBlockConverter<FromDataBlockType, string>{
	public:
		static inline string DoConvert(FromDataBlockType* block){
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


	// type resolver specifications //
#define TVALRESOLVERTYPE(BlockTypeT, DEFINEDValT) template<> struct TvalToTypeResolver<DEFINEDValT>{ static BlockTypeT* Conversion(DataBlockAll* bl){return static_cast<BlockTypeT*>(bl);}};

	TVALRESOLVERTYPE(IntBlock, DATABLOCK_TYPE_INT);
	TVALRESOLVERTYPE(FloatBlock, DATABLOCK_TYPE_FLOAT);
	TVALRESOLVERTYPE(BoolBlock, DATABLOCK_TYPE_BOOL);
	TVALRESOLVERTYPE(WstringBlock, DATABLOCK_TYPE_WSTRING);
	TVALRESOLVERTYPE(StringBlock, DATABLOCK_TYPE_STRING);
	TVALRESOLVERTYPE(CharBlock, DATABLOCK_TYPE_CHAR);
	TVALRESOLVERTYPE(DoubleBlock, DATABLOCK_TYPE_DOUBLE);
	TVALRESOLVERTYPE(LeviathanObjectBlock, DATABLOCK_TYPE_OBJECTL);
	TVALRESOLVERTYPE(VoidPtrBlock, DATABLOCK_TYPE_VOIDPTR);

}
#endif