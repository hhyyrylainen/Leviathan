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
#define DATABLOCK_TYPE_WSTRING	6 // is actually wstring
#define DATABLOCK_TYPE_STRING	7
#define DATABLOCK_TYPE_CHAR		8
#define DATABLOCK_TYPE_DOUBLE	9
#define DATABLOCK_TYPE_OBJECTL	10 // uses Leviathan::Object* as type
#define DATABLOCK_TYPE_VOIDPTR	11

#define DATABLOCK_TYPE_ERROR	9000

namespace Leviathan{

	// static class used to convert DataBlocks from different types to other types //
	class DataBlockConverter{
	public:



	};

	class DataBlock : public Object{
	public:
		DLLEXPORT DataBlock::DataBlock();
		DLLEXPORT DataBlock::DataBlock(const int &type);
		//DLLEXPORT DataBlock::DataBlock(DataBlock* arg);
		DLLEXPORT virtual DataBlock::~DataBlock();

		DataBlock operator =(const DataBlock& arg);

		DLLEXPORT static DataBlock* CopyConstructor(DataBlock* arg);

		// value getting operators //
		DLLEXPORT operator int();
		DLLEXPORT operator int*();
		DLLEXPORT operator float();
		DLLEXPORT operator float*();
		DLLEXPORT operator bool();
		DLLEXPORT operator bool*();
		DLLEXPORT operator wstring();
		DLLEXPORT operator wstring*();
		DLLEXPORT operator void*();


		//int* Value;
		int Type;
	};

	class IntBlock : public DataBlock{
	public:
		DLLEXPORT IntBlock::IntBlock();
		DLLEXPORT IntBlock::IntBlock(int value/*,int type*/);
		DLLEXPORT IntBlock::IntBlock(IntBlock* arg);
		DLLEXPORT IntBlock::~IntBlock();

		IntBlock operator =(const IntBlock& arg);
		IntBlock operator=(int& arg);

		operator int();
		operator int*();

		int* Value;
		//int Type;
	};


	class FloatBlock : public DataBlock{
	public:
		DLLEXPORT FloatBlock::FloatBlock();
		DLLEXPORT FloatBlock::FloatBlock(float value/*,int type*/);
		DLLEXPORT FloatBlock::FloatBlock(FloatBlock* arg);
		DLLEXPORT FloatBlock::~FloatBlock();

		FloatBlock operator =(const FloatBlock& arg);
		FloatBlock operator=(float& arg);

		operator float();
		operator float*();

		float* Value;
		//int Type;
	};

	class BoolBlock : public DataBlock{
	public:
		DLLEXPORT BoolBlock::BoolBlock();
		DLLEXPORT BoolBlock::BoolBlock(bool value/*,int type*/);
		DLLEXPORT BoolBlock::BoolBlock(BoolBlock* arg);
		DLLEXPORT BoolBlock::~BoolBlock();

		BoolBlock operator =(const BoolBlock& arg);
		BoolBlock operator=(bool& arg);

		operator bool();
		operator bool*();

		bool* Value;
		//int Type;
	};

	class WstringBlock : public DataBlock{
	public:
		DLLEXPORT WstringBlock::WstringBlock();
		DLLEXPORT WstringBlock::WstringBlock(wstring value/*,int type*/);
		DLLEXPORT WstringBlock::WstringBlock(WstringBlock* arg);
		DLLEXPORT WstringBlock::~WstringBlock();

		WstringBlock operator =(const WstringBlock& arg);
		WstringBlock operator=(wstring& arg);

		operator wstring();
		operator wstring*();

		wstring* Value;
		//int Type;
	};


	class VoidBlock : public DataBlock{
	public:
		DLLEXPORT VoidBlock::VoidBlock();
		DLLEXPORT VoidBlock::VoidBlock(void* value/*,int type*/);
		DLLEXPORT VoidBlock::VoidBlock(VoidBlock* arg);
		DLLEXPORT VoidBlock::~VoidBlock();

		VoidBlock operator =(const VoidBlock& arg);
		VoidBlock operator=(void* arg);


		operator void*();

		void* Value;
		//int Type;
	};

}
#endif