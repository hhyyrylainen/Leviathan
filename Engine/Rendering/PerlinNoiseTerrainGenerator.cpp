#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TERRAIN_PERLINNOISEGENERATOR
#include "PerlinNoiseTerrainGenerator.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

#define BM 0xff
#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

DLLEXPORT Leviathan::PerlinNoiseTerrainGenerator::PerlinNoiseTerrainGenerator(const float &alpha /*= 3.3f*/, const float &beta /*= 2.2f*/, 
	const int iterations /*= 10*/, const float &cycle /*= 128*/, const float &heightscale /*= 4*/) : Alpha(alpha), Beta(beta), Iterations(iterations),
	Cycle(cycle), HeightScale(heightscale)
{
	// Random ptr to reduce code //
	Random* rnd = Random::Get();

	// setup some internal data //
	for(int i = 0; i < PerlinBSize; i++){
		p[i] = i;
		g1[i] = rnd->SymmetricRandom();

		g2[i] = Float2(rnd->SymmetricRandom(), rnd->SymmetricRandom());
		g2[i].Normalize();

		g3[i] = Float3(rnd->SymmetricRandom(), rnd->SymmetricRandom(), rnd->SymmetricRandom());
		g3[i].Normalize();
	}

	for(int i = 0; i < PerlinBSize; i++){
		int j = rnd->GetNumber(0, PerlinBSize);

		int k = p[i];
		p[i] = p[j];
		p[j] = k;
	}

	for(int i = 0; i < PerlinBSize + 2; i++){
		p[PerlinBSize + i] = p[i];
		g1[PerlinBSize + i] = g1[i];
		g2[PerlinBSize + i] = g2[i];
		g3[PerlinBSize + i] = g3[i];
	}
}

DLLEXPORT Leviathan::PerlinNoiseTerrainGenerator::~PerlinNoiseTerrainGenerator(){
	// should be nothing to cleanup //
}

// ------------------------------------ //
// function(s, following three too)  courtesy of sdk sample EndlessWorld (MIT) //
DLLEXPORT void Leviathan::PerlinNoiseTerrainGenerator::define(Ogre::TerrainGroup* terraingroup, long x, long y){

	UINT16 terrainSize = terraingroup->getTerrainSize();
	float* heightMap = OGRE_ALLOC_T(float, terrainSize*terrainSize, Ogre::MEMCATEGORY_GEOMETRY);

	Float2 worldOffset(float(x*(terrainSize-1)), float(y*(terrainSize-1)));
	worldOffset += Origin;

	Float2 revisedValuePoint;
	for(UINT16 i=0; i<terrainSize; i++){
		for(UINT16 j=0; j<terrainSize; j++){

			revisedValuePoint = (worldOffset+Float2(j, i))/Cycle;
			heightMap[i*terrainSize + j] = ProduceSingleHeight(revisedValuePoint) * HeightScale;
		}
	}

	terraingroup->defineTerrain(x, y, heightMap);
	OGRE_FREE(heightMap, Ogre::MEMCATEGORY_GEOMETRY);
}
// ------------------------------------ //
float Leviathan::PerlinNoiseTerrainGenerator::ProduceSingleHeight(const Float2 &point){

	Float2 temppoint(point);

	float sum = 0;
	float scale = 1;

	for(int i = 0; i < Iterations; i++){

		sum += Create2DNoise(temppoint)/scale;
		scale *= Alpha;
		temppoint *= Beta;
	}
	return sum;
}

float Leviathan::PerlinNoiseTerrainGenerator::Create2DNoise(const Float2 &point){
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, sx, sy, a, b, u, v;

	Setup(point.X, bx0,bx1, rx0,rx1);
	Setup(point.Y, by0,by1, ry0,ry1);

	int i = p[ bx0 ];
	int j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = SCurce(rx0);
	sy = SCurce(ry0);

	u = g2[b00].Dot(Float2(rx0, ry0));
	v = g2[b10].Dot(Float2(rx1, ry0));
	a = Lerp(sx, u, v);

	u = g2[b01].Dot(Float2(rx0, ry1));
	v = g2[b11].Dot(Float2(rx1, ry1));
	b = Lerp(sx, u, v);

	return Lerp(sy, a, b);
}
// ------------------------------------ //
void Leviathan::PerlinNoiseTerrainGenerator::Setup(const float &target, int &b0, int &b1, float &r0, float &r1){
	float t = target + N;
	b0 = ((int)t) & BM;
	b1 = (b0+1) & BM;
	r0 = t - (int)t;
	r1 = r0 - 1;
}
// ------------------------------------ //





