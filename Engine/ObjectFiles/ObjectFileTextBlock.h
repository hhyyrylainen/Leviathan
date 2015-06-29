#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <vector>

namespace Leviathan{

	//! \brief Interface for object file text blocks to implement
	//! \see ObjectFileListProper
	class ObjectFileTextBlock{
	public:
		
		DLLEXPORT virtual ~ObjectFileTextBlock();

		//! \brief Adds an UTF8 encoded line
		DLLEXPORT virtual void AddTextLine(const std::string &line) = 0;

		//! \brief Returns the number of text lines
		DLLEXPORT virtual size_t GetLineCount() const = 0;

		//! \brief Gets a line from index
		//! \except ExceptionInvalidArgument when the index is out of bounds
		//! \see GetLineCount
		DLLEXPORT virtual const std::string& GetLine(size_t index) const = 0;

		//! \brief Gets the name of this text block
		DLLEXPORT virtual const std::string& GetName() const = 0;


	protected:

		ObjectFileTextBlock(){};
	};

	//! \brief Implementation of ObjectFileTextBlock
	//! \see ObjectFileTextBlock
	class ObjectFileTextBlockProper : public ObjectFileTextBlock{
	public:

		DLLEXPORT ObjectFileTextBlockProper(const std::string &name);
		DLLEXPORT ~ObjectFileTextBlockProper();

		DLLEXPORT virtual void AddTextLine(const std::string &line);

		DLLEXPORT virtual size_t GetLineCount() const;

		DLLEXPORT virtual const std::string& GetLine(size_t index) const;

		DLLEXPORT virtual const std::string& GetName() const;

	protected:

		std::string Name;
		std::vector<std::string*> Lines;
	};


}

