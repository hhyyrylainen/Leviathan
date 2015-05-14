#pragma once
// ------------------------------------ //
#include "Include.h"

#include "ThreadSafe.h"
#include <unordered_map>
#include <functional>
#include <tuple>
#include <vector>

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
        DLLEXPORT ElementType* ConstructNew(Lock &guard, KeyType forentity, Args&&... args){

            if(Find(guard, forentity))
                throw Exception("Entity with ID already has object in pool of this type");

            try{

                auto created = Elements.construct(args...);

                // Add to index for finding later //
                Index.insert(std::make_pair(forentity, created));

                Added.push_back(std::make_tuple(created, forentity));
            
                return created;

                
            } catch(const Exception){

                throw Exception("Failed to construct element");
            }
        }

        template<typename... Args>
        DLLEXPORT inline ElementType* ConstructNew(KeyType forentity, Args&&... args){

            GUARD_LOCK();
            return ConstructNew(guard, forentity, args...);
        }

        //! \brief Returns true if there are objects in Removed
        DLLEXPORT bool HasElementsInRemoved() const{

            GUARD_LOCK();
            return !Removed.empty();
        }

        //! \brief Returns true if there are objects in Added
        DLLEXPORT bool HasElementsInAdded() const{

            GUARD_LOCK();
            return !Added.empty();
        }

        //! \brief Calls Release with the specified arguments on the released types
        template<typename... Args>
        DLLEXPORT void ReleaseRemoved(Args&&... args){

            GUARD_LOCK();

            for(auto iter = Removed.begin(); iter != Removed.end(); ++iter){

                auto object = std::get<0>(*iter);
                const auto id = std::get<1>(*iter);

                object->Release(args...);

                Elements.destroy(object);
                RemoveFromIndex(guard, id);
            }

            Removed.clear();
        }


        //! \brief Clears removed entities without calling Release
        DLLEXPORT void ClearRemoved(){

            GUARD_LOCK();

            for(auto iter = Removed.begin(); iter != Removed.end(); ++iter){

                auto object = std::get<0>(*iter);
                const auto id = std::get<1>(*iter);

                Elements.destroy(object);
                RemoveFromIndex(guard, id);
            }

            Removed.clear();
        }

        //! \brief Returns a reference to the vector of removed elements
        //! \note This object needs to be locked and kept locked while using the result
        //! of this call
        DLLEXPORT const auto& GetRemoved(Lock &locked) const{

            return Removed;
        }
        
        

        //! \brief Returns a reference to the vector of added elements
        //! \note This object needs to be locked and kept locked while using the result
        //! of this call
        DLLEXPORT auto& GetAdded(Lock &locked){

            return Added;
        }

        //! \brief Clears the added list
        DLLEXPORT void ClearAdded(){

            GUARD_LOCK();
            Added.clear();
        }

        //! \brief QueueDestroys elements based on ids in vector
        template<typename Any>
        DLLEXPORT void RemoveBasedOnKeyTupleList(
            const std::vector<std::tuple<Any, KeyType>> &values)
        {

            GUARD_LOCK();

            for(auto iter = values.begin(); iter != values.end(); ++iter){

                auto todelete = Index.find(std::get<1>(*iter));

                if(todelete == Index.end())
                    continue;
                
                Removed.push_back(std::make_tuple(todelete->second, todelete->first));

                RemoveFromAdded(guard, todelete->first);
                    
                Index.erase(iter);
            }
        }

        //! \brief Removes a specific id from the added list
        //!
        //! Used to remove entries that have been deleted before clearing the added ones
        //! \todo Check if this gives better performance than noticing missing elements
        //! during node construction
        DLLEXPORT void RemoveFromAdded(Lock &guard, KeyType id){

            for(auto iter = Added.begin(); iter != Added.end(); ++iter){

                if(std::get<1>(*iter) == id){

                    Added.erase(iter);
                    return;
                }
            }
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
            RemoveFromAdded(guard, id);
        }

        DLLEXPORT inline void Destroy(KeyType id){

            GUARD_LOCK();
            Destroy(guard, id);
        }

        //! \brief Queues destruction of an element
        //! \exception InvalidArgument when key is not found (is already deleted)
        //! \note This has to be used for objects that require calling Release
        DLLEXPORT void QueueDestroy(Lock &guard, KeyType id){

            auto end = Index.end();
            for(auto iter = Index.begin(); iter != end; ++iter){

                if(iter->first == id){

                    Removed.push_back(std::make_tuple(iter->second, id));

                    RemoveFromAdded(guard, id);
                    
                    Index.erase(iter);
                    return;
                }
            }

            throw InvalidArgument("ID is not in index");
        }

        DLLEXPORT inline void QueueDestroy(KeyType id){

            GUARD_LOCK();
            QueueDestroy(guard, id);
        }

        //! \brief Calls an function on all the objects in the pool
        //! \note The order of the objects is not guaranteed and can change between runs
        //! \param function The function that is called with all of the components of this type
        //! the first parameter is the component, the second is the id of the entity owning the
        //! component and the last one is a lock to this ComponentHolder,
        //! the return value specifies
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

        //! \brief Clears the index and replaces the pool with a new one
        //! \warning All objects after this call are invalid
        DLLEXPORT void Clear(){

            Index.clear();
            Elements = std::move(boost::object_pool<ElementType>());

            Removed.clear();
            Added.clear();
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

        //! Used for delayed deleting elements
        std::vector<std::tuple<ElementType*, KeyType>> Removed;

        //! Used for detecting created elements
        std::vector<std::tuple<ElementType*, KeyType>> Added;

        //! Pool for objects
        boost::object_pool<ElementType> Elements;
	};
}
