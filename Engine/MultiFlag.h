#ifndef LEVIATHAN_MULTIFLAG
#define LEVIATHAN_MULTIFLAG
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


// ALL possible flags //
#define FLAG_GOBJECT_CONTAINS_INIT				20000001
#define FLAG_GOBJECT_CONTAINS_TICK				20000002
#define FLAG_GOBJECT_CONTAINS_RENDER			20000003

// object definitions //
#define FLAG_GOBJECT_TYPE_MODEL					30000001


// object specific flags //
#define FLAG_GOBJECT_MODEL_TYPE_NORMAL			40000001
#define FLAG_GOBJECT_MODEL_TYPE_BUMP			40000002
#define FLAG_GOBJECT_MODEL_TEXTURETYPE_NORMAL	40000003
#define FLAG_GOBJECT_MODEL_TEXTURETYPE_BUMP		40000004
#define FLAG_GOBJECT_MODEL_TEXTURETYPE_LIGHT	40000005
#define FLAG_GOBJECT_MODEL_TEXTURETYPE_BLENDMAP	40000006
#define FLAG_GOBJECT_MODEL_TEXTURETYPE_UNKOWN	40000007
//#define FLAG_GOBJECT_MODEL_
//#define FLAG_GOBJECT_MODEL_

namespace Leviathan{

	struct Flag{
		Flag(int val){
			Value = val;
		}

		bool operator ==(const Flag& othr){
			if(othr.Value == this->Value)
				return true;
			return false;
		}

		int Value;
	};

	class MultiFlag : public Object{
	public:
		DLLEXPORT MultiFlag::MultiFlag();
		DLLEXPORT MultiFlag::MultiFlag(vector<shared_ptr<Flag>>& toset);
		DLLEXPORT MultiFlag::~MultiFlag();

		DLLEXPORT void SetFlag(const Flag &flag);
		DLLEXPORT void UnsetFlag(const Flag &flag);

		DLLEXPORT bool IsSet(int value);


		DLLEXPORT inline vector<shared_ptr<Flag>> GetFlags();
	private:
		vector<shared_ptr<Flag>> Flags;
		int CombinedVal;
	};

}
#endif