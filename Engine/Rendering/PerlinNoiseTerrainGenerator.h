#ifndef LEVIATHAN_TERRAIN_PERLINNOISEGENERATOR
#define LEVIATHAN_TERRAIN_PERLINNOISEGENERATOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <Terrain/OgreTerrainPagedWorldSection.h>
#include "Utility/Random.h"


// no idea what this is but the sdk example uses this constant //
#define PerlinBSize 0x100
#define PerlinNoiseInternalSize PerlinBSize + PerlinBSize + 2

namespace Leviathan{

	// should work the same as the one in the sdk examples //

	class PerlinNoiseTerrainGenerator : public Ogre::TerrainPagedWorldSection::TerrainDefiner{
	public:
		// same parameters as the sdk example, used in the generation
		DLLEXPORT PerlinNoiseTerrainGenerator(const float &alpha = 3.3f, const float &beta = 2.2f, const int iterations = 10, const float &cycle = 128,
			const float &heightscale = 4);
		DLLEXPORT ~PerlinNoiseTerrainGenerator();


		// defines a terrain at location //
		DLLEXPORT virtual void define(Ogre::TerrainGroup* terraingroup, long x, long y);


		DLLEXPORT inline void RandomizeStartPosition(const int &difference){

			Origin = Float2(Random::Get()->GetNumber((float)-difference, (float)difference), 
				Random::Get()->GetNumber((float)-difference, (float)difference));

		}

	private:
		// produces noise according to PerlinNoise algorithm, same as the sdk example infinite world) //
		float ProduceSingleHeight(const Float2 &point);

		// 2d noise, also same as the sdk example //
		float Create2DNoise(const Float2 &point);

		inline float SCurce(const float &t) const {return t*t*(3-2*t);};
		inline float Lerp(const float &t, const float &a, const float &b) const {return a+t*(b-a);};
		void Setup(const float &target, int &b0, int &b1, float &r0, float &r1);

		// variables //
		float Alpha;
		float Beta;
		int Iterations;
		float Cycle;
		float HeightScale;

		Float2 Origin;

		// internal perlin noise variables (I think?) //
		int p[PerlinNoiseInternalSize];
		Float3 g3[PerlinNoiseInternalSize];
		Float2 g2[PerlinNoiseInternalSize];
		float g1[PerlinNoiseInternalSize];
	};

}
#endif