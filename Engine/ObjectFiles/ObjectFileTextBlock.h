#ifndef LEVIATHAN_OBJECTFILE_TEXTBLOCK
#define LEVIATHAN_OBJECTFILE_TEXTBLOCK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{


	//! \brief Interface for object file text blocks to implement
	//! \see ObjectFileListProper
	class ObjectFileTextBlock : public Object{
	public:
		
		DLLEXPORT virtual ~ObjectFileTextBlock();

		//! \brief Adds an UTF8 encoded line
		//! \note The line will be converted into wstring (utf16)
		//! \return True when added, false if the UTF8 encoding is invalid
		DLLEXPORT virtual bool AddTextLine(const string &line) = 0;


		//! \brief Returns the number of text lines
		DLLEXPORT virtual size_t GetLineCount() const = 0;

		//! \brief Gets a line from index
		//! \except ExceptionInvalidArgument when the index is out of bounds
		//! \see GetLineCount
		DLLEXPORT virtual const wstring& GetLine(size_t index) const THROWS = 0;

		//! \brief Gets the name of this text block
		DLLEXPORT virtual const wstring& GetName() const = 0;


	protected:

		ObjectFileTextBlock(){};


	};


	//! \brief Implementation of ObjectFileTextBlock
	//! \see ObjectFileTextBlock
	class ObjectFileTextBlockProper : public ObjectFileTextBlock{
	public:

		DLLEXPORT ObjectFileTextBlockProper(const wstring &name);
		DLLEXPORT ~ObjectFileTextBlockProper();

		DLLEXPORT virtual bool AddTextLine(const string &line);

		DLLEXPORT virtual size_t GetLineCount() const;

		DLLEXPORT virtual const wstring& GetLine(size_t index) const THROWS;

		DLLEXPORT virtual const wstring& GetName() const;





	protected:


		wstring Name;
		std::vector<wstring*> Lines;
	};


}
#endif
