#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_WORLDTERRAIN
#include "WorldTerrain.h"
#endif
#include "Terrain/OgreTerrainMaterialGeneratorA.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::WorldTerrain::WorldTerrain(Ogre::Light* l, Ogre::SceneManager* scene, Ogre::Camera* terrainloadingcamera) : mTerrainGroup(0), 
	mTerrainPaging(0), mPageManager(0), mPagedWorld(0), mTerrainPagedWorldSection(0), mPerlinNoiseTerrainGenerator(0), TerrainPos(0, 0, 0)
{
	// this function is partially from the sdk sample EndlessTerrain //
	mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();


	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(scene, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
	mTerrainGroup->setFilenameConvention(TERRAINFILE_PREFIX, TERRAINFILE_SUFFIX);
	mTerrainGroup->setResourceGroup("Terrain");
	mTerrainGroup->setOrigin(TerrainPos);
	mTerrainGroup->setAutoUpdateLod(Ogre::TerrainAutoUpdateLodFactory::getAutoUpdateLod(Ogre::BY_DISTANCE));

	ConfigureTerrainDefaults(l, scene);

	// Paging setup
	mPageManager = OGRE_NEW Ogre::PageManager();
	// Since we're not loading any pages from .page files, we need a way just 
	// to say we've loaded them without them actually being loaded
	mPageManager->setPageProvider(&mDummyPageProvider);
	mPageManager->addCamera(terrainloadingcamera);
	mPageManager->setDebugDisplayLevel(0);
	mTerrainPaging = OGRE_NEW Ogre::TerrainPaging(mPageManager);
	mPagedWorld = mPageManager->createWorld();
	mTerrainPagedWorldSection = mTerrainPaging->createWorldSection(mPagedWorld, mTerrainGroup, 400, 500, 
		ENDLESS_PAGE_MIN_X, ENDLESS_PAGE_MIN_Y, 
		ENDLESS_PAGE_MAX_X, ENDLESS_PAGE_MAX_Y);

	mPerlinNoiseTerrainGenerator = new PerlinNoiseTerrainGenerator();
	mTerrainPagedWorldSection->setDefiner( mPerlinNoiseTerrainGenerator );
	//		mTerrainPagedWorldSection->setDefiner( OGRE_NEW SimpleTerrainDefiner );

	mTerrainGroup->freeTemporaryResources();
}

DLLEXPORT Leviathan::WorldTerrain::~WorldTerrain(){
	// release data //
	if(mTerrainPaging){

		OGRE_DELETE mTerrainPaging;
		mPageManager->destroyWorld( mPagedWorld );
		OGRE_DELETE mPageManager;
	}
	OGRE_DELETE mTerrainGlobals;
}
// ------------------------------------ //
void Leviathan::WorldTerrain::ConfigureTerrainDefaults(Ogre::Light* l, Ogre::SceneManager* scene){
	// Configure global
	mTerrainGlobals->setMaxPixelError(8);
	// testing composite map
	mTerrainGlobals->setCompositeMapDistance(32000);
	//mTerrainGlobals->setCastsDynamicShadows(false);
	mTerrainGlobals->setCastsDynamicShadows(true);
	//mTerrainGlobals->setCastsDynamicShadows(true);
	mTerrainGlobals->setSkirtSize(10);
	mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
	//mTerrainGlobals->setUseRayBoxDistanceCalculation(false);


	mTerrainGlobals->setCompositeMapAmbient(scene->getAmbientLight());
	mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
	mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());

	Ogre::TerrainMaterialGeneratorA::SM2Profile* mProfile = static_cast<Ogre::TerrainMaterialGeneratorA::SM2Profile*>
		(mTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());
	//mProfile->setLayerParallaxMappingEnabled(false);
	mProfile->setLayerParallaxMappingEnabled(true);
	mProfile->setLayerSpecularMappingEnabled(false);
	//mProfile->setLayerSpecularMappingEnabled(true);
	//mProfile->setLightmapEnabled(false);
	mProfile->setLightmapEnabled(true);
	//mProfile->setLayerNormalMappingEnabled(false);
	mProfile->setLayerNormalMappingEnabled(true);
	mTerrainGlobals->setLayerBlendMapSize(1024);
	mTerrainGlobals->setLightMapSize(512);

	//mProfile->setReceiveDynamicShadowsDepth(true);
	//mProfile->setReceiveDynamicShadowsPSSM();


	//m_pMainTerrain->setGlobalColourMapEnabled(true,2048);
	//TexturePtr ColourMap  = m_pMainTerrain->getGlobalColourMap();
	//Ogre::Image ColourMapImage;
	//ColourMapImage.load("DiffuseColurMap.jpg",m_pMainTerrain->getResourceGroup());
	//ColourMap->loadImage(ColourMapImage);

	// Configure default import settings for if we use imported image
	Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
	defaultimp.terrainSize = TERRAIN_SIZE;
	defaultimp.worldSize = TERRAIN_WORLD_SIZE;
	defaultimp.inputScale = 600;
	defaultimp.minBatchSize = 33;
	defaultimp.maxBatchSize = 65;
	// textures
	//defaultimp.layerList.resize(3);
	defaultimp.layerList.resize(1);
	defaultimp.layerList[0].worldSize = 100;
	defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
	defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
	//defaultimp.layerList[1].worldSize = 30;
	//defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
	//defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
	//defaultimp.layerList[2].worldSize = 200;
	//defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
	//defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
}
// ------------------------------------ //

// ------------------------------------ //
//void Leviathan::WorldTerrain::TERRAIN_DefineTerrainAt(long x, long y){
//	// first get the name of the file that could contain the terrain data//
//	Ogre::String filename = _TerrainGroup->generateFilename(x, y);
//
//	// check does the file exist //
//	if(Ogre::ResourceGroupManager::getSingleton().resourceExists(_TerrainGroup->getResourceGroup(), filename)){
//		// has cached results //
//		_TerrainGroup->defineTerrain(x, y);
//
//	} else {
//		// needs to generate new //
//		Ogre::Image img;
//		TERRAIN_GetTerrainImage(x % 2 != 0, y % 2 != 0, img);
//		_TerrainGroup->defineTerrain(x, y, &img);
//		_TerrainImported = true;
//	}
//}
//
//void Leviathan::WorldTerrain::TERRAIN_GetTerrainImage(bool flipx, bool flipy, Ogre::Image &img){
//	// load the terrain height map image and possibly flip it //
//	img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
//
//	if(flipx)
//		img.flipAroundY();
//	if(flipy)
//		img.flipAroundX();
//}
//
//void Leviathan::WorldTerrain::TERRAIN_InitBlendMaps(Ogre::Terrain* terrain){
//
//	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
//	Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
//	Ogre::Real minHeight0 = 70;
//	Ogre::Real fadeDist0 = 40;
//	Ogre::Real minHeight1 = 70;
//	Ogre::Real fadeDist1 = 15;
//	float* pBlend0 = blendMap0->getBlendPointer();
//	float* pBlend1 = blendMap1->getBlendPointer();
//	for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
//	{
//		for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
//		{
//			Ogre::Real tx, ty;
//
//			blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
//			Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
//			Ogre::Real val = (height - minHeight0) / fadeDist0;
//			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
//			*pBlend0++ = val;
//
//			val = (height - minHeight1) / fadeDist1;
//			val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
//			*pBlend1++ = val;
//		}
//	}
//	blendMap0->dirty();
//	blendMap1->dirty();
//	blendMap0->update();
//	blendMap1->update();
//}
// ------------------------------------ //






