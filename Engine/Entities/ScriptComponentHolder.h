// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include <map>

class asIScriptFunction;
class asIScriptObject;

namespace Leviathan {

//! \brief
//! \todo Allow defining if release should be called on the script object. and then calling it
//! now only the object is dereferenced so that it probably gets garbage collected soon
class ScriptComponentHolder : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    //! \note This expects that caller has increased factory refcount
    //! \param world Pointer to world that is passed to the script factory
    DLLEXPORT ScriptComponentHolder(
        const std::string& name, asIScriptFunction* factory, GameWorld* world);

public:
    DLLEXPORT ~ScriptComponentHolder();

    //! \brief Releases a single component
    //! \note If a script is holding a reference to the object it may not be released yet
    //! \returns True if object existed and was released
    DLLEXPORT bool ReleaseComponent(ObjectID entity);

    //! \brief Releases all components
    DLLEXPORT void ReleaseAllComponents();

    //! \brief Creates a new component of held type
    //! \note Increases refcount on returned object
    //! \warning Some script components need Init to be called on them afterwards, so you must
    //! cast this to the child type and call Init on it. See CustomScriptComponentTest.as file
    //! in the Scripts/tests directory
    DLLEXPORT asIScriptObject* Create(ObjectID entity);

    //! \brief Finds a component for entity
    //! \note Increases refcount on returned object
    DLLEXPORT asIScriptObject* Find(ObjectID entity);


    //! \brief Returns a reference to the vector of removed elements
    const auto& GetRemoved() const
    {
        return Removed;
    }

    //! \brief Returns a reference to the vector of added elements
    auto& GetAdded()
    {
        return Added;
    }

    //! \brief Clears the added list
    void ClearAdded()
    {
        Added.clear();
    }

    //! \brief Clears the removed list
    void ClearRemoved()
    {
        Removed.clear();
    }


    REFERENCE_COUNTED_PTR_TYPE(ScriptComponentHolder);

    const std::string ComponentType;

private:
    asIScriptFunction* Factory;
    GameWorld* World;

    std::map<ObjectID, asIScriptObject*> CreatedObjects;

    // Need to remember these for compatibility with caching
    std::vector<std::tuple<asIScriptObject*, ObjectID>> Added;
    std::vector<std::tuple<asIScriptObject*, ObjectID>> Removed;
};

} // namespace Leviathan
