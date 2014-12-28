#ifndef LEVIATHAN_TRAILEMITTER
#define LEVIATHAN_TRAILEMITTER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "../Bases/BaseObject.h"
#include "../Bases/BaseParentable.h"
#include "../Bases/BaseRenderable.h"


namespace Leviathan{ namespace Entity{

	struct TrailElementProperties{

		DLLEXPORT TrailElementProperties(const Float4 &initialcolour, const Float4 &colourchange,
            const float &initialsize, const float &sizechange) : 
			InitialColour(initialcolour), ColourChange(colourchange), InitialSize(initialsize), SizeChange(sizechange)
		{

		}
        
		DLLEXPORT TrailElementProperties(const Float4 &initialcolour, const float &initialsize) : 
			InitialColour(initialcolour), ColourChange(0), InitialSize(initialsize), SizeChange(0)
		{

		}

		Float4 InitialColour;
		Float4 ColourChange;
		float InitialSize;
		float SizeChange;
	};

	struct TrailProperties{
	public:
		DLLEXPORT TrailProperties(size_t maxelements, float lenght, float maxdistance, bool castshadows = false) :
            ElementProperties(1), TrailLenght(lenght), MaxDistance(maxdistance), CastShadows(castshadows),
            MaxChainElements(maxelements)
		{
		}
        
		// Empty constructor //
		DLLEXPORT TrailProperties(){}

		DLLEXPORT ~TrailProperties(){
			// Delete memory //
			SAFE_DELETE_VECTOR(ElementProperties);
		}
        
		// Needs a custom assignment operator //
		DLLEXPORT TrailProperties& operator =(const TrailProperties &other);

		float TrailLenght;
		float MaxDistance;
		size_t MaxChainElements;
		bool CastShadows;

		std::vector<TrailElementProperties*> ElementProperties;
	};

	class TrailEmitter : virtual public BaseObject, public BaseParentable, public BaseRenderable{
	public:
		DLLEXPORT TrailEmitter(GameWorld* world, bool hidden = false);
		DLLEXPORT virtual ~TrailEmitter();

		//! \brief Creates the actual trail object
        //! \param allowupdate Set to true if you want to use SetTrailProperties later
		DLLEXPORT bool Init(const string &materialname, const TrailProperties &variables, bool allowupdate = true);
        
		DLLEXPORT virtual void ReleaseData();

		//! \brief Sets properties on the trail object
        //! \pre Init is called
        //! aram force If set to true all settings will be applied
		DLLEXPORT bool SetTrailProperties(const TrailProperties &variables, bool force = false);

		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

	private:
		// Used to reposition the node //
		virtual void PosUpdated();
		virtual void OrientationUpdated();

		virtual void _OnHiddenStateUpdated();
		// ------------------------------------ //

		// Ogre handles pretty much everything related to trails
		Ogre::RibbonTrail* TrailEntity;
        
		//! This node controls where the trail appears (as the actual entity is in the root node)
		Ogre::SceneNode* TrailLocation;

		//! Some settings need caching to avoid having to destroy the old trail object
		TrailProperties CachedSettings;

	};

}}
#endif
