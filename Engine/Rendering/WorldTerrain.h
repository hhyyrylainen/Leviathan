#ifndef LEVIATHAN_WORLDTERRAIN
#define LEVIATHAN_WORLDTERRAIN
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "PerlinNoiseTerrainGenerator.h"
#include <Terrain\OgreTerrain.h>
#include <Terrain\OgreTerrainGroup.h>
#include <Terrain\OgreTerrainPagedWorldSection.h>
#include <Terrain\OgreTerrainPaging.h>
#include <Paging\OgrePageManager.h>

// max ranges for terrain (max int16 values //
#define ENDLESS_PAGE_MIN_X (-0x7FFF)
#define ENDLESS_PAGE_MIN_Y (-0x7FFF)
#define ENDLESS_PAGE_MAX_X 0x7FFF
#define ENDLESS_PAGE_MAX_Y 0x7FFF

// terrain storage config //
#define TERRAINFILE_PREFIX Ogre::String("TerrainCache")
#define TERRAINFILE_SUFFIX Ogre::String("dat")


// size definitions //
#define TERRAIN_WORLD_SIZE		12000.0f
#define TERRAIN_SIZE			513
#define HOLD_LOD_DISTANCE		3000.0f

namespace Leviathan{

	class WorldTerrain : public Object{
		// these are too from the sdk sample EndlessTerraing (MIT) //
		/// This class just pretends to provide procedural page content to avoid page loading
		class DummyPageProvider : public Ogre::PageProvider
		{
		public:
			bool prepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
			bool loadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
			bool unloadProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
			bool unprepareProceduralPage(Ogre::Page* page, Ogre::PagedWorldSection* section) { return true; }
		};

		class SimpleTerrainDefiner : public Ogre::TerrainPagedWorldSection::TerrainDefiner
		{
		public:
			virtual void define(Ogre::TerrainGroup* terrainGroup, long x, long y)
			{
				Ogre::Image img;
				img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
				if (x % 2)
					img.flipAroundY();
				if (y % 2)
					img.flipAroundX();
				terrainGroup->defineTerrain(x, y, &img);
			}
		};

	public:
		DLLEXPORT WorldTerrain(Ogre::Light* l, Ogre::SceneManager* scene, Ogre::Camera* terrainloadingcamera);
		DLLEXPORT ~WorldTerrain();


	private:

		Ogre::TerrainGlobalOptions* mTerrainGlobals;
		Ogre::TerrainGroup* mTerrainGroup;
		Ogre::TerrainPaging* mTerrainPaging;
		Ogre::PageManager* mPageManager;
		Ogre::PagedWorld* mPagedWorld;
		Ogre::TerrainPagedWorldSection* mTerrainPagedWorldSection;

		PerlinNoiseTerrainGenerator* mPerlinNoiseTerrainGenerator;

		DummyPageProvider mDummyPageProvider;

		Ogre::Vector3 TerrainPos;



		void ConfigureTerrainDefaults(Ogre::Light* l, Ogre::SceneManager* scene);


		//void TERRAIN_DefineTerrainAt(long x, long y);
		//void TERRAIN_GetTerrainImage(bool flipx, bool flipy, Ogre::Image &img);
		//void TERRAIN_InitBlendMaps(Ogre::Terrain* terrain);
		//bool TERRAIN_FrameRenderingQueued(const Ogre::FrameEvent& evt);
	};

}
#endif