#ifndef LEVIATHAN_ANIMATIONBLOCK
#define LEVIATHAN_ANIMATIONBLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class AnimationManager;
	class LoadedAnimation;

	class AnimationBlock : public Object{
	friend AnimationManager;
	friend LoadedAnimation;
	public:
		DLLEXPORT AnimationBlock::~AnimationBlock();


	private:
		DLLEXPORT AnimationBlock::AnimationBlock();
	};

}
#endif