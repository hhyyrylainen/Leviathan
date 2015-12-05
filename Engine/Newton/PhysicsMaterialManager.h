#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "PhysicalMaterial.h"
#include "NewtonManager.h"

namespace Leviathan{

	class PhysicsMaterialManager{
	public:
		// Newton manager needs to be passed to ensure newton is instantiated //
		DLLEXPORT PhysicsMaterialManager(const NewtonManager* newtoninstanced);
		DLLEXPORT ~PhysicsMaterialManager();

		// This function should be called from the load physical materials callback
		// NOTE: do not delete the ptr after calling this //
		DLLEXPORT void LoadedMaterialAdd(PhysicalMaterial* ptrtotakeownership);

		// \todo file loading function //

		// ------------------ Material ID fetching functions ------------------ //

		// Gets the ID of a material based on name and NewtonWorld ptr //
		DLLEXPORT int GetMaterialIDForWorld(const std::string &name, NewtonWorld* WorldPtr);

		// Gets the pointer to physical material //
		DLLEXPORT std::shared_ptr<PhysicalMaterial> GetMaterial(const std::string &name);


		// This function builds the materials for a newton world (this is provided so that you can
        // use multiple threads to setup the game)
		DLLEXPORT void CreateActualMaterialsForWorld(NewtonWorld* newtonworld);

        //! \brief Clears world from materials
        DLLEXPORT void DestroyActualMaterialsForWorld(NewtonWorld* world);

		DLLEXPORT static PhysicsMaterialManager* Get();
	private:

		// Parts of data construction //
		void _CreatePrimitiveIDList(NewtonWorld* world);
		void _ApplyMaterialProperties(NewtonWorld* world);
		// ------------------------------------ //

		// map for fast finding //
		std::map<std::string, std::shared_ptr<PhysicalMaterial>> LoadedMaterials;

		static PhysicsMaterialManager* StaticInstance;
	};

}

