Leviathan game engine repository

-- Potential bugs that should be fixed --
    - Many places that use StringIterator don't bother to check if they get null pointers

-- Content For games --
opengameart.org
game-icons.net

-- Doxygen notes --

Use these new doxygen commands:

\todo
\test
\bug
\deprecated
\warning
\pre
(\post)
\note
\param  (\param[out] dest The function's result destination.)
\return

"see also" \sa

Also remember these:

\details
\brief
\author
\version
\date
(\copyright)

Something for nice structure:

\par
\anchor (something for \ref)


What could be done:

    line 914 REFERENCED_BY_RELATION = NO to true
    
    line 1500 has SERVER_BASED_SEARCH which should be off for local viewing
    
    
Things to watch out for:

    "Please make sure answers to stateless/anonymous requests are no larger than the request package 
    to avoid providing an amplification platform for (D)DoS."
    
    