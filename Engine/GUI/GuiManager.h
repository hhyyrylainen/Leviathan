// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "../Application/AppDefine.h"
#include "BaseGuiObject.h"
#include "../Common/ThreadSafe.h"
#include "../TimeIncludes.h"

#include "OgreRenderQueueListener.h"

namespace Leviathan{
namespace GUI{

//! \brief A class for handling system clipboard interaction
class GuiClipboardHandler;

//! \brief Holds the state of some collections stored by name
struct GuiCollectionStates{
private:
    //! \brief Helper struct that allows keeping all the data in a single vector
    struct SingleCollectionEntry{
        //! \brief Constructs a new entry for a single GuiCollection
        SingleCollectionEntry(const std::string &name, bool enabled) :
            IsEnabled(enabled), Name(new std::string(name))
        {

        }

        bool IsEnabled;
        std::unique_ptr<std::string> Name;
    };

public:
    //! \brief Creates an empty list of GUI state
    GuiCollectionStates(size_t expectedcount = 0){
        CollectionNames.reserve(expectedcount);
    }

    //! \brief Adds a new entry to the list
    inline void AddNewEntry(const std::string &name, bool state){

        CollectionNames.push_back(std::unique_ptr<SingleCollectionEntry>(
                    new SingleCollectionEntry(name, state)));
    }


    std::vector<std::unique_ptr<SingleCollectionEntry>> CollectionNames;
};


//! \brief Main GUI controller
//! \todo Add GUI window objects to this which are associated with different windows
class GuiManager{
public:
    DLLEXPORT GuiManager();
    DLLEXPORT ~GuiManager();

    //! \param ismain Set to true for first created GuiManager
    DLLEXPORT bool Init(Graphics* graph, GraphicalInputEntity* window, bool ismain);
    DLLEXPORT void Release();

    //! \brief Sets up leviathan default GUI style and font.
    //!
    //! Call in LeviathanApplication::CustomizeEnginePostLoad if desired
    //! \note If this is to be called on any window. It must be called with
    //! the main window first. See: GuiManager::MainGuiManager
    DLLEXPORT void EnableStandardGUIThemes();

    //! \brief Called by Engine during Release to destroy static variables
    DLLEXPORT static void KillGlobalCache();

    DLLEXPORT void GuiTick(int mspassed);
    DLLEXPORT void Render();

    // key press receiving from listener //
    DLLEXPORT bool ProcessKeyDown(int32_t key, int specialmodifiers);

    //! \todo Actually pass this to Views
    DLLEXPORT inline void SetVisible(bool visible){
        Visible = visible;
    }
    //! \brief Notifies internal browsers
    //! \todo Make CEGUI allow multiple windows
    DLLEXPORT void OnResize();

    //! \brief Notifies contexts about the change to appropriately lose focus on fields
    DLLEXPORT void OnFocusChanged(bool focused);

    // Internal Gui element managing //
    DLLEXPORT bool AddGuiObject(BaseGuiObject* obj);
    DLLEXPORT void DeleteObject(int id);
    DLLEXPORT int GetObjectIndexFromId(int id);
    DLLEXPORT BaseGuiObject* GetObject(unsigned int index);


    //! \brief Returns the main GUI context
    DLLEXPORT CEGUI::GUIContext* GetMainContext();

    DLLEXPORT CEGUI::InputAggregator* GetContextInput(){
        return ContextInput;
    }

    // function split into peaces //
    DLLEXPORT bool LoadCollection(std::vector<std::shared_ptr<ObjectFileObject>> &data,
        ObjectFileObject &collectiondata);

    // file loading //

    //! \brief Loads a GUI file
    DLLEXPORT bool LoadGUIFile(const std::string &file, bool nochangelistener = false,
        int iteration = 0);

    //! \brief Unloads the currently loaded file
    DLLEXPORT void UnLoadGUIFile();

    //! \brief Creates an object representing the state of all GuiCollections
    DLLEXPORT std::unique_ptr<GuiCollectionStates> GetGuiStates() const;

    //! \brief Applies a stored set of states to the GUI
    //! \param states A pointer to an object holding the state,
    //! the object should be obtained by calling GetGuiStates
    //! \see GetGuiStates
    DLLEXPORT void ApplyGuiStates(const GuiCollectionStates* states);


    //! set to "none" to use default
    DLLEXPORT void SetMouseTheme(const std::string &tname);

    //! Sets the tooltip type to use
    DLLEXPORT void SetTooltipType(const std::string &type);

    // collection managing //
    DLLEXPORT void AddCollection(GuiCollection* add);
        
    DLLEXPORT GuiCollection* GetCollection(const int &id, const std::string &name = "");

    DLLEXPORT void SetCollectionState(const std::string &name, bool state);
    DLLEXPORT void SetCollectionAllowEnableState(const std::string &name, bool allow = true);
    DLLEXPORT inline void PossiblyGUIMouseDisable(){
        GuiMouseUseUpdated = true;
    }
        
    // called when mouse cannot be captured (should force at least one collection on) //
    DLLEXPORT void OnForceGUIOn();

    //! \brief Returns a single CEGUI::Window matching the name
    //! \todo Allow error reporting
    DLLEXPORT CEGUI::Window* GetWindowByStringName(const std::string &namepath);


    //! \brief Returns a string containing names of types that don't look good/break
    //! something when animated
    //!
    //! For use with PlayAnimationOnWindow the ignoretypenames parameter if you just want
    //! to avoid breaking some rendering
    DLLEXPORT FORCE_INLINE static std::string GetCEGUITypesWithBadAnimations(){
        return "";
    }


    //! \brief Creates and plays an animation on a CEGUI Window
    //! \param applyrecursively Applies the same animation to the child windows
    DLLEXPORT bool PlayAnimationOnWindow(const std::string &windowname,
        const std::string &animationname, bool applyrecursively = false,
        const std::string &ignoretypenames = "");

        
    //! \brief Proxy overload for PlayAnimationOnWindow
    DLLEXPORT bool PlayAnimationOnWindowProxy(const std::string &windowname,
        const std::string &animationname);


    //! \brief Stops the Gui from capturing the mouse when
    //! no Gui on-keeping collections are active
    DLLEXPORT void SetDisableMouseCapture(bool newvalue);
        
    //! \brief Tries to inject a paste request to CEGUI
    DLLEXPORT bool InjectPasteRequest();

    //! \brief Tries to inject a copy request to CEGUI
    DLLEXPORT bool InjectCopyRequest();

    //! \brief Tries to inject a cut request to CEGUI
    DLLEXPORT bool InjectCutRequest();

    //! \brief Loads a GUI scheme file
    //! \exception If can't load
    DLLEXPORT static void LoadGUITheme(const std::string &filename); 
    
protected:

    //! Is called by folder listeners to notify of Gui file changes
    void _FileChanged(const std::string &file, ResourceFolderListener &caller);
        

private:

    //! The implementation of PlayAnimationOnWindow
    void _PlayAnimationOnWindow(CEGUI::Window* targetwind,
        CEGUI::Animation* animdefinition, bool recurse,
        const std::string &ignoretypenames);

    // Static animation files //
    static bool IsAnimationFileLoaded(const std::string &file);

    //! \warning Won't check if the file is already in the vector,
    //! use IsAnimationFileLoaded
    static void SetAnimationFileLoaded(const std::string &file);
    
private:

    bool Visible = true;

    //! Used to determine when to scan collections for active ones
    bool GuiMouseUseUpdated = true;
        
    //! Set when containing window of the GUI shouldn't be allowed to capture mouse
    bool GuiDisallowMouseCapture = true;

    GraphicalInputEntity* ThisWindow = nullptr;
        
    //! Gui elements
    std::vector<BaseGuiObject*> Objects;

    //! The corresponding CEGUI context
    CEGUI::GUIContext* GuiContext = nullptr;

    //! The input handler for the context
    CEGUI::InputAggregator* ContextInput = nullptr;

    //! Used to keep track of elapsed time for keeping CEGUI posted on the current time
    WantedClockType::time_point LastTimePulseTime;

    //! The main file of the GUI from which it is loaded from
    std::string MainGUIFile;

    //! Set when this is the first created gui manager
    //! \detail Used for injecting time pulses into CEGUI
    bool MainGuiManager = false;

    int ID;

    //! Used to stop listening for file changes
    int FileChangeID = 0;

    //! When set to true will reload files on next tick
    bool ReloadQueued = false;

    // Collections //
    std::vector<GuiCollection*> Collections;


    //! The clipboard access object
    std::unique_ptr<GuiClipboardHandler> _GuiClipboardHandler;

    //! Disables the GUI trying to capture the mouse when no collection is active
    bool DisableGuiMouseCapture = false;

private:
    //! Holds the loaded animation files, used to prevent loading a single file
    //! multiple times
    static std::vector<std::string> LoadedAnimationFiles;
        
};

}}

