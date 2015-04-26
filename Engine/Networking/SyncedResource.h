#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/BaseNotifiable.h"
#include "Common/SFMLPackets.h"
#include "Exceptions.h"

namespace Leviathan{

	//! \brief Base class for all values that are to be automatically synced between clients
	//! \note The variable is not sent to other clients unless they allow new resources from the net
	//! \warning The variable must be registered for syncing after SyncedVariables is created
	//! \todo Possibly do a global static class that will automatically register all when single StartSync is called
	class SyncedResource : public BaseNotifiableAll{
		friend SyncedVariables;
	public:
		//! \brief Constructs a base class for synced variables that requires a unique name
		//! \todo Actually check if the name is actually unique
		DLLEXPORT SyncedResource(const wstring &uniquename);
		DLLEXPORT virtual ~SyncedResource();


		//! \brief Serializes the name to a packet
		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! \brief Gets a name from packet leaving only the variable data there
		DLLEXPORT static wstring GetSyncedResourceNameFromPacket(sf::Packet &packet);

		//! \brief Assigns data from a packet to this resource
		//! \return False when the actual implementation throws
		DLLEXPORT virtual bool UpdateDataFromPacket(sf::Packet &packet);

		//! \brief Registers this resource with the SyncedVariables instance
		//! \post The variable is now ready for use
		DLLEXPORT virtual void StartSync();

		//! \brief Notify that this is changed and we want a local message, too
		//!
		//! If you do not want a local message call UpdateOurNetworkValue directly
		DLLEXPORT void NotifyUpdatedValue();

	protected:

		//! \brief Should load the custom data from a packet
		virtual void UpdateCustomDataFromPacket(sf::Packet &packet) = 0;

		//! \brief Should be used to add custom data to packet
		//! \see UpdateCustomDataFromPacket
		virtual void SerializeCustomDataToPacket(sf::Packet &packet) = 0;


		//! \brief Notifies our SyncedVariables of an update
        //! \todo Proper locking
		DLLEXPORT virtual void UpdateOurNetworkValue();

		//! Update notifications are received through this
		//! 
		//! Called from UpdateDataFromPacket
		virtual void OnValueUpdated();


		// Disable copy and copy constructor usage //
		SyncedResource& operator=(const SyncedResource &other);
		SyncedResource(const SyncedResource &other);
		// ------------------------------------ //

		const wstring Name;
	};


	//! \brief Template class for syncing basic types
	//! \warning This will only work with primitive types like int, float, string etc.
    //! For other use you must inherit SyncedResource and create a custom class
	template<class DTypeName>
	class SyncedPrimitive : public SyncedResource{
	public:
		//! The callback type
		typedef void (*CallbackPtr)(SyncedPrimitive<DTypeName>* updated);

		//! \brief Constructs an instance with a initial value
		//! \warning The order of the initializer list is important since anytime after calling
        //! SyncedResource we can receive updates
		DLLEXPORT SyncedPrimitive(const wstring &uniquename, const DTypeName &initialvalue,
            CallbackPtr updatecallback = NULL) : OurValue(initialvalue), 
			ValueUpdateCallback(updatecallback), SyncedResource(uniquename)
		{
			// Now we are ready to be updated //
			IsValid = true;
		}
        
		DLLEXPORT ~SyncedPrimitive(){
			// Unhook already //
			ReleaseParentHooks();
			// Set us as invalid after locking //
			GUARD_LOCK();
			IsValid = false;

			// Destructors will take care of the rest //
		}

		//! \brief Updates the value and notifies SyncedVariables
		//! \note This does not call the callback or OnValueUpdated. They are only called when
        //! receiving updates through network
		DLLEXPORT inline void UpdateValue(const DTypeName &newvalue){
			{
				GUARD_LOCK();
				// Update our value //
				OurValue = newvalue;
			}
			UpdateOurNetworkValue();
		}

		//! \brief Gets the value with locking
		DLLEXPORT DTypeName GetValue() const{
			GUARD_LOCK();
			return OurValue;
		}

		//! \brief Directly accesses the variable, you will need to use your own locking with complex types
		//! \warning The returned value might be changed at any point and depending on this
        //! object's lifespan it may become invalid
		//! \see GetValue
		DLLEXPORT DTypeName* GetValueDirect(){
			return &OurValue;
		}

		//! \brief Assignment operator that acts like UpdateValue
		DLLEXPORT SyncedResource& operator =(const DTypeName &value){
			UpdateValue(value);
			return *this;
		}
		// ------------------ Overloaded operators for ease of use ------------------ //
		DLLEXPORT bool operator ==(const DTypeName &value){
			return OurValue == value;
		}
		DLLEXPORT bool operator !=(const DTypeName &value){
			return OurValue != value;
		}

		DLLEXPORT operator DTypeName(){
			GUARD_LOCK();
			return OurValue;
		}


	protected:

		virtual void OnValueUpdated(){
			// Report update //
			if(ValueUpdateCallback)
				ValueUpdateCallback(this);
		}

		virtual void UpdateCustomDataFromPacket(sf::Packet &packet){
			// The object is already locked at this point //

			// Try to get our variable //
			if(!(packet >> OurValue)){

				throw InvalidArgument("resource sync primitive packet has invalid format");
			}

		}

		virtual void SerializeCustomDataToPacket(sf::Packet &packet){
			packet << OurValue;
		}

		// ------------------------------------ //

		//! Little overhead but this is important to discard update requests after stopping
		bool IsValid;

		//! This is quite an important feature to store this function pointer
		CallbackPtr ValueUpdateCallback;

		//! The primitive object owned by this
		DTypeName OurValue;
	};


}

