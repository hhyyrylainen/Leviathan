
            
            DLLEXPORT Prop(bool hidden, GameWorld* world);
            DLLEXPORT virtual ~Prop();
            
            DLLEXPORT bool Init(Lock &guard, const std::string &modelfile);

            DLLEXPORT inline bool Init(const std::string &modelfile){

                GUARD_LOCK();
                return Init(guard, modelfile);
            }
            
            DLLEXPORT virtual void ReleaseData();


            DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

            //! \copydoc BaseSendableEntity::CaptureState
            DLLEXPORT std::shared_ptr<ObjectDeltaStateData> CaptureState(Lock &guard, int tick)
                override;

            DLLEXPORT int OnEvent(Event** pEvent) override;
            DLLEXPORT int OnGenericEvent(GenericEvent** pevent) override;

            //! \copydoc BaseSendableEntity::CreateStateFromPacket
            DLLEXPORT virtual std::shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
                sf::Packet &packet) const override;

            REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(Prop);
            
        protected:
            //! \brief Constructs a prop for receiving through the network
            Prop(bool hidden, GameWorld* world, int netid);

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(Lock &guard, sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(Lock &guard, sf::Packet &packet) override;

            //! \copydoc BaseConstraintable::_SendCreatedConstraint
            void _SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr) override;

            void _OnNewStateReceived() override;

            
            
            // for setting new values to graphical object and physical object //
            virtual void _UpdatePhysicsObjectLocation(Lock &guard) override;

            BaseConstraintable* BasePhysicsGetConstraintable() override;
            // ------------------------------------ //

            std::string ModelFile;

            bool ListeningForEvents;
        };

}}


