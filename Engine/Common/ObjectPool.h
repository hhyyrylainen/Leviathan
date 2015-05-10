#pragma once
// ------------------------------------ //
#include "Include.h"

#include "ThreadSafe.h"
#include <unordered_map>
#include <functional>

#include "boost/pool/object_pool.hpp"

namespace Leviathan{

    

    template<class ElementType, typename KeyType>
	class ObjectPool : public ThreadSafe{
	public:

        DLLEXPORT ObjectPool() : Elements(100){

        }

        //! \brief Constructs a new component of the held type for entity
        //! \exception Exception when component has not been created
        template<typename... Args>
        DLLEXPORT ElementType* ConstructNew(KeyType forentity, Args... args){

            GUARD_LOCK();
            if(Find(guard, forentity))
                throw Exception("Entity with ID already has object in pool of this type");
            
            auto created = Elements.construct();

            try{

                if(!created->Init(args...)){

                    throw Exception("Failed to Init element");
                }
                
            } catch(...){

                Elements.destruct(created);
                throw;
            }

            // Add to index for finding later //
            Index.insert(std::make_pair(forentity, created));
            return created;
        }

        //! \return The found component or NULL
        DLLEXPORT ElementType* Find(Lock &guard, KeyType id) const{

            auto iter = Index.find(id);

            if(iter == Index.end())
                return nullptr;

            return iter->second;
        }

        DLLEXPORT inline ElementType* Find(KeyType id) const{

            GUARD_LOCK();
            return Find(guard, id);
        }

        //! \brief Destroys a component based on id
        DLLEXPORT void Destroy(Lock &guard, KeyType id){

            auto object = Find(guard, id);

            if(!object)
                throw InvalidArgument("ID is not in index");

            Elements.destroy(object);
            RemoveFromIndex(guard, id);
        }

        DLLEXPORT inline void Destroy(KeyType id){

            GUARD_LOCK();
            Destroy(guard, id);
        }

        //! \brief Calls an function on all the objects in the pool
        //! \note The order of the objects is not guaranteed and can change between runs
        //! \param function The function that is called with all of the components of this type
        //! the first parameter is the component, the second is the id of the entity owning the
        //! component and the last one is a lock to this ComponentHolder, the return value specifies
        //! if the component should be destroyed (true being yes and false being no)
        DLLEXPORT void Call(std::function<bool (ElementType&, KeyType, Lock&)> function){

            GUARD_LOCK();

            for(auto iter = Index.begin(); iter != Index.end(); ){

                if(function(*iter->second, iter->first, guard)){

                    Elements.destroy(iter->second);
                    iter = Index.erase(iter);
                    
                } else {
                    
                    ++iter;
                }
            }
        }

        //! \brief Clears the index and replkaces the pool with a new one
        //! \warning All objects after this call are invalid
        DLLEXPORT void Clear(){

            Index.clear();
            Elements = std::move(boost::object_pool<ElementType>());
        }

    protected:
        //! \brief Removes an component from the index but doesn't destruct it
        //! \note The component will only be deallocated once this object is destructed
        DLLEXPORT bool RemoveFromIndex(Lock &guard, KeyType id){

            auto end = Index.end();
            for(auto iter = Index.begin(); iter != end; ++iter){

                if(iter->first == id){

                    Index.erase(iter);
                    return true;
                }
            }

            return false;
        }

        inline bool RemoveFromIndex(KeyType id){

            GUARD_LOCK();
            return RemoveFromIndex(id);
        }

    protected:

        //! Used for looking up element belonging to id
        std::unordered_map<KeyType, ElementType*> Index;

        //! Pool for objects
        boost::object_pool<ElementType> Elements;
	};
}
