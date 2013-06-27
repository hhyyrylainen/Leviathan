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
	bool DataBlockTestVerifier(const int &tests);

	// forward declaration //
	template<class DBlockT> class DataBlock;

	// template struct for getting right type for right type...
	template<class T>
	struct DataBlockNameResolver{

		static const int TVal = DATABLOCK_TYPE_ERROR;
	};
	// specification values //
	template<> struct DataBlockNameResolver<int>{		static const int TVal = DATABLOCK_TYPE_INT;};
	template<> struct DataBlockNameResolver<float>{		static const int TVal = DATABLOCK_TYPE_FLOAT;};
	template<> struct DataBlockNameResolver<bool>{		static const int TVal = DATABLOCK_TYPE_BOOL;};
	template<> struct DataBlockNameResolver<wstring>{	static const int TVal = DATABLOCK_TYPE_WSTRING;};
	template<> struct DataBlockNameResolver<string>{	static const int TVal = DATABLOCK_TYPE_STRING;};
	template<> struct DataBlockNameResolver<char>{		static const int TVal = DATABLOCK_TYPE_CHAR;};
	template<> struct DataBlockNameResolver<double>{	static const int TVal = DATABLOCK_TYPE_DOUBLE;};
	template<> struct DataBlockNameResolver<Object*>{	static const int TVal = DATABLOCK_TYPE_OBJECTL;};
	template<> struct DataBlockNameResolver<void*>{		static const int TVal = DATABLOCK_TYPE_VOIDPTR;};


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
			if(DataBlockNameResolver<T>::TVal == block->Type){
				// just return //
				return *block->Value;
			}
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

		// pure virtual functions to prevent instantiation and force derived classes define these //
		// value getting operators //
		//template<class ConvertT>
		//DLLEXPORT operator ConvertT(){

		//	assert(0 && "base class access");
		//}
		//template<class ConvertT>
		//DLLEXPORT operator ConvertT*(){

		//	assert(0 && "base class access");
		//}


		int Type;
	};


	// main DataBlock class //
	template<class DBlockT>
	class DataBlock : public DataBlockAll{
	public:
		DLLEXPORT DataBlock() : Value(NULL){

			Type = DATABLOCK_TYPE_ERROR;
		}
		DLLEXPORT DataBlock(const DBlockT &blockvalue){

			// use templates to get type //
			Type = DataBlockNameResolver<DBlockT>::TVal;
		}

		DLLEXPORT virtual ~DataBlock(){
			// erase memory //
			SAFE_DELETE(Value);
		}
		// deep copy operator //
		DataBlock& operator =(const DataBlock& arg){
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

		//DLLEXPORT operator int(){

		//	return DataBlockConversionResolver<DBlockT, int>::DoConversionNonPtr(this);
		//}
		//DLLEXPORT operator int*(){

		//	return DataBlockConversionResolver<DBlockT, int>::DoConversionPtr(this);
		//}
		//DLLEXPORT operator float();
		//DLLEXPORT operator float*();
		//DLLEXPORT operator bool();
		//DLLEXPORT operator bool*();
		//DLLEXPORT operator wstring();
		//DLLEXPORT operator wstring*();
		//DLLEXPORT operator string();
		//DLLEXPORT operator string*();
		//DLLEXPORT operator char();
		//DLLEXPORT operator char*();
		//DLLEXPORT operator double();
		//DLLEXPORT operator double*();
		//// pointer only types //
		//DLLEXPORT operator Object*();
		//DLLEXPORT operator void*();

	//private:

		DBlockT* Value;
	};

	class NamedDataBlockAll /*: public DataBlockAll*/{

		// virtual destructor for deleting through base pointers
		DLLEXPORT inline virtual ~NamedDataBlockAll(){


		}

		wstring name;
	};

	// Datablock variant with space for name //
	template<class DBlockT>
	class NamedDataBlock : public DataBlock<DBlockT>, public NamedDataBlockAll{




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


	// conversion template specifications //
	template<>
	class DataBlockConverter<IntBlock, bool>{
	public:
		static inline bool DoConvert(IntBlock* block){
			return (*block->Value) != 0;
		}
		static const bool AllowedConversion = true;
	};

}
#endif