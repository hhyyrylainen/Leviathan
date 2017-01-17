// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
//! \file This file contains some extra algorithms that make your life easier
#pragma once
// ------------------------------------ //

namespace Leviathan{



template<class InputIteratorT, class ResultInserterT>
    inline void FindRemovedElements(
        const InputIteratorT &oldbegin, const InputIteratorT &oldend,
        const InputIteratorT &newbegin, const InputIteratorT &newend,
        ResultInserterT &addedresult, ResultInserterT &removedresult)
{
    // Find removed items first //
    for(auto iter = oldbegin; iter != oldend; ++iter){

        bool found = false;

        for(auto iter2 = newbegin; iter2 != newend; ++iter2){

            if(*iter == *iter2){

                found = true;
                break;
            }
        }

        if(found)
            continue;

        // Not found in new //
        removedresult.push_back(*iter);
    }

    // Then find added items //
    for(auto iter = newbegin; iter != newend; ++iter){

        bool found = false;

        for(auto iter2 = oldbegin; iter2 != oldend; ++iter2){

            if(*iter == *iter2){

                found = true;
                break;
            }
        }

        if(found)
            continue;

        // Not found in old //
        addedresult.push_back(*iter);
    }    
}

// ------------------------------------ //
// Convenience functions
// These make calling the algorithms easier for some specific cases
template<class InputCollectionT, class ResultInserterT>
    inline void FindRemovedElements(const InputCollectionT &olditems,
        const InputCollectionT &newitems,
        ResultInserterT &addedresult, ResultInserterT &removedresult)
{
    FindRemovedElements(olditems.begin(), olditems.end(),
        newitems.begin(), newitems.end(), addedresult, removedresult);
}

}
