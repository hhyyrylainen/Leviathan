#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILEPROCESSOR
#include "ObjectFileProcessor.h"
#endif
#define BOOST_SPIRIT_UNICODE

#include "FileSystem.h"
#include <boost/assign/list_of.hpp>
#include "Common/DataStoring/DataStore.h"
#include "Common/DataStoring/DataBlock.h"
#include "Common/StringOperations.h"
#include "Utility/Iterators/WstringIterator.h"
#include "Common/Misc.h"
#include "boost/spirit/home/qi.hpp"
#include "boost/fusion/adapted/struct/adapt_struct.hpp"
#include <boost/spirit/repository/include/qi_confix.hpp>
#include "boost/spirit/home/support/iterators/line_pos_iterator.hpp"
#include <boost/regex/pending/unicode_iterator.hpp>
#include "boost/spirit/home/karma.hpp"
#include <boost/fusion/adapted/adt/adapt_adt.hpp>
#include "utf8/core.h"
using namespace Leviathan;
// ------------------------------------ //
ObjectFileProcessor::ObjectFileProcessor(){}
Leviathan::ObjectFileProcessor::~ObjectFileProcessor(){}

// quick macro to make this shorter //
#ifdef _MSC_VER
#define ADDDATANAMEINTDEFINITION(x) (WIDEN(#x), new VariableBlock(new IntBlock(x)))
#else
#define ADDDATANAMEINTDEFINITION(x) (WIDEN(#x), shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(x))))
#endif

map<wstring, shared_ptr<VariableBlock>> Leviathan::ObjectFileProcessor::RegisteredValues = boost::assign::map_list_of
	ADDDATANAMEINTDEFINITION(DATAINDEX_TICKTIME)
	ADDDATANAMEINTDEFINITION(DATAINDEX_TICKCOUNT)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS)
	ADDDATANAMEINTDEFINITION(DATAINDEX_WIDTH)
	ADDDATANAMEINTDEFINITION(DATAINDEX_HEIGHT)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_MIN)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_MAX)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FRAMETIME_AVERAGE)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_AVERAGE)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_MIN)
	ADDDATANAMEINTDEFINITION(DATAINDEX_FPS_MAX)
	;

// ------------------------------------ //
void Leviathan::ObjectFileProcessor::Initialize(){
#ifdef _DEBUG
	// Just out of curiosity check this //
	auto iter = RegisteredValues.find(L"DATAINDEX_TICKTIME");

	if(iter == RegisteredValues.end()){

		Logger::Get()->Error(L"ObjectFileProcessor: RegisteredValues are messed up, DATAINDEX_TICKTIME is not defined, check the macros!");
		return;
	}
#endif // _DEBUG
}
void Leviathan::ObjectFileProcessor::Release(){
	// Release our allocated memory //
	RegisteredValues.clear();
}
// ------------------------------------ //
DLLEXPORT  void Leviathan::ObjectFileProcessor::RegisterValue(const wstring &name, VariableBlock* valuetokeep){
	RegisteredValues[name] = shared_ptr<VariableBlock>(valuetokeep);
}
// ------------------ Processing helping classes ------------------ //
using namespace boost::spirit;
using namespace boost::spirit::qi;
using namespace boost::fusion;



//! Skip parser for skipping whitespace and comments
template<typename Iterator>
struct OFSkipper : public grammar<Iterator>{
public:
	// Define a skipping function that skips whitespace and c-style comments //
	OFSkipper() : OFSkipper::base_type(skip, "OFSkipper"){
		// Create the rule //
		skip = boost::spirit::ascii::space | boost::spirit::repository::qi::confix("//", eol)[*(char_ - eol)] | 
			boost::spirit::repository::qi::confix("/*", "*/")[*(char_ -"*/")];
	}

	rule<Iterator> skip;
};

// ------------------ Temporary classes for processing the data into ------------------ //
namespace Leviathan{
	//! Temporary class for header definitions
	struct HeaderDefinition{

		std::string NameContent;
		std::string UnParsedValue;


		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};

	//! Temporary class for holding template expansion information
	struct ObjectTemplateInstance{

		std::string TemplateName;
		std::vector<std::string> TemplateArguments;

		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};


	//! Temporary class for holding list data
	typedef std::vector<boost::variant<std::string, Leviathan::HeaderDefinition>> ListLineData;

	struct ListDataHolder{

		std::string Name;
		ListLineData Variables;

		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};

	//! Temporary class for holding text block data
	struct TextBlockDataHolder{

		std::string Name;
		std::vector<std::string> Lines;

		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};


	struct ObjectFileDefinition;

	//typedef boost::recursive_wrapper<Leviathan::ObjectFileDefinition> ChildObjectsType;
	typedef Leviathan::ObjectFileDefinition ChildObjectsType;

	typedef boost::variant<Leviathan::ListDataHolder, Leviathan::TextBlockDataHolder> SingleObjectContent;

	//! Apparently these need to be put into a different place for this to work
	struct ObjectNameTypeDefinition{

		std::string TypeName;
		std::vector<std::string> Prefixes;
		std::string Name;

		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};

	//! Temporary class for holding an object during parsing
	struct ObjectFileDefinition{
		

		ObjectNameTypeDefinition MyName;



		//! Actual contents
		std::vector<SingleObjectContent> Data;


		//! Child objects of the same type
		std::vector<ChildObjectsType> ChildObjects;


		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};

	//! Temporary class for holding an template definition and the matching object file definition
	struct ObjectTemplateDef{

		std::vector<std::string> TemplateArguments;
		ObjectFileDefinition ObjectInstance;

		std::ostream & operator<<(std::ostream &os){
			// Serialize this class partially here //
			return os << "stringy";
		}
	};
}
// ------------------ Adapt fusion classes ------------------ //
BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::HeaderDefinition,
	(std::string, NameContent)
	(std::string, UnParsedValue))

BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::ListDataHolder,
	(std::string, Name)
	(Leviathan::ListLineData, Variables))

BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::TextBlockDataHolder,
	(std::string, Name)
	(std::vector<std::string>, Lines))


BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::ObjectNameTypeDefinition,
	(std::string, TypeName)
	(std::vector<std::string>, Prefixes)
	(std::string, Name))


BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::ObjectFileDefinition,
	(Leviathan::ObjectNameTypeDefinition, MyName)
	(std::vector<Leviathan::SingleObjectContent>, Data)
	(std::vector<ChildObjectsType>, ChildObjects))

BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::ObjectTemplateDef,
	(std::vector<std::string>, TemplateArguments)
	(Leviathan::ObjectFileDefinition, ObjectInstance))

BOOST_FUSION_ADAPT_STRUCT(
	Leviathan::ObjectTemplateInstance,
	(std::string, TemplateName)
	(std::vector<std::string>, TemplateArguments))


typedef boost::variant<HeaderDefinition, ObjectFileDefinition, ObjectTemplateInstance, ObjectTemplateDef> SingleLineDef;
typedef std::vector<SingleLineDef> ResultIntermediateType;


// ------------------ Printing functions for helper structs ------------------ //

std::ostream & operator<<(std::ostream &os, const HeaderDefinition &object){
	using namespace boost::spirit::karma;

	std::string generated;
	std::back_insert_iterator<std::string> sink(generated);

	bool result = generate(sink, 
		// Output format //
		karma::string << ' = ' << karma::string << ';'
		, object.NameContent, object.UnParsedValue);

	if(result)
		os << generated;

	return os;
}


// ------------------ Helper classes for on_error ------------------ //
struct ErrorReporter{
	template<class Val, class FFirst, class First, class Last>
	struct result { typedef void type; };

	ErrorReporter(const std::string &file) : ContainingFile(file){

	}

	template<class Val, class FFirst, class First, class Last>
	void operator()(Val& errorstr, FFirst filebegin, First errorpos, Last endpos) const {
		
		stringstream strs;
		size_t errorline = errorpos.base().position();
		size_t beginline = filebegin.base().position();
		size_t endline = endpos.base().position();

		strs << "ObjectFile Parsing error. Expecting " << errorstr << " in " << ContainingFile << 
			"(" << errorline << ") here: \"" << std::string(errorpos, endpos) << "\"";
		Logger::Get()->Error(Convert::StringToWstring(strs.str()));
	}

	std::string ContainingFile;
};



// ------------------ The grammar and rules ------------------ //
//! The main grammar class for handling the entire file
template<typename Iterator, typename Skipper = OFSkipper<Iterator>>
struct ObjectFileGrammar : grammar<Iterator, ResultIntermediateType(), Skipper>{
public:

	ObjectFileGrammar(const std::string &file) : ObjectFileGrammar::base_type(MainStructure, "MainStructure"), ErrorReportObjFunc(file){

		// Set rule names for better error support //
		MainStructure.name("MainStructure");
		SingleLineDefinitionProcess.name("SingleLineBlockStart");
		HeaderVariableParser.name("HeaderVariable");
		ObjectDefinitionParser.name("ObjectDefinition");
		TemplateInstantiation.name("TemplateInstantiation");
		TemplateDefinitionParser.name("TemplateDefinition");

		ObjectBlockParser.name("Object");
		ObjectDataContentParser.name("ObjectData");
		ObjectListDataParser.name("VariableList");
		ObjectTextBlockParser.name("TextBlock");
		ObjectNameParser.name("EntityName");

		EscapedCharacter.name("EscapedCharacter");
		BlockName.name("Name");
		QuotedString.name("QuotedString");


		// First the utility rules //
		EscapedCharacter %= '\\' >> char_("\\{};:=()\"'");

		BlockName %= lexeme[+(~char_('{') -eol | EscapedCharacter)] > '{';

		QuotedString %= (lit('"') | lit('\'')) > +(~char_("\"'") | EscapedCharacter) > (lit('"') | lit('\''));

		// Then the main rules //


		// First one is easy just get the name and then the value on a line //
		HeaderVariableParser %= +(~char_("=:")) > (lit('=') | lit(':')) > +(~char_(';') | EscapedCharacter) > lit(';');

		// Almost as easy is the TemplateInstantiation definition //
		TemplateInstantiation %= lit("template<>") > lexeme[+(~char_('<'))] > *(~char_(">,") | EscapedCharacter) > '>';

		// Then the more difficult thing: loading objects //

		// This parses the first name line //
		ObjectNameParser %= lexeme[+(char_ -' ')] >> -(lit('<') > (lexeme[+(~char_(", >"))] % ',') > lit('>')) > QuotedString > lit('{');

		ObjectBlockParser %= 
			// The first initial line 
			lit('o') > ObjectNameParser
			// Start of the inner content
			>> *(ObjectDataContentParser) 

			// Try to parse recursive object definitions
			>> *(ObjectBlockParser)

			// end of content
			> lit('}');


		// Defined in two parts to allow the recursion to happen
		ObjectDefinitionParser %= ObjectBlockParser;

		// List data parser //
		ObjectDataContentParser %= (ObjectListDataParser | ObjectTextBlockParser);

		// Parses variable list data //
		ObjectListDataParser %= lit('l') > BlockName
			// Content lines //
			>> *((lit("<t>") >> as_string[+(char_ -eol)]) | HeaderVariableParser) > lit('}');


		// Parses plain text blocks //
		ObjectTextBlockParser %= lit('t') > BlockName
			// Actual content lines //
			>> *(!lit('}') >> *(char_ -eol)) > '}';


		// The most complex is the template definition //
		TemplateDefinitionParser %= lit("template<") > +(+as_string[~char_(",>") >> -lit(',')]) > '>'
			// After the heading stuff parse the object
			> ObjectDefinitionParser;


		// Try to parse any type on every line starting something //
		SingleLineDefinitionProcess %= (TemplateDefinitionParser | TemplateInstantiation | ObjectDefinitionParser | HeaderVariableParser);



		// Parses sequentially all definitions in the file //
		MainStructure %= *(SingleLineDefinitionProcess);

		// Error reporting //


		on_error<fail>(MainStructure, (ErrorReportObjFunc(qi::_4, qi::_1, qi::_3, qi::_2)));


		// Debug EVERYTHING //
		debug(MainStructure);
		debug(SingleLineDefinitionProcess);
		debug(HeaderVariableParser);
		debug(ObjectDefinitionParser);
		debug(TemplateInstantiation);
		debug(TemplateDefinitionParser);
		debug(ObjectBlockParser);
		debug(ObjectDataContentParser);
		debug(ObjectListDataParser);
		debug(ObjectTextBlockParser);
		debug(ObjectNameParser);
		debug(EscapedCharacter);
		debug(BlockName);
		debug(QuotedString);
	}

	// Main processing rules //
	rule<Iterator, ResultIntermediateType(), Skipper> MainStructure;
	rule<Iterator, SingleLineDef(), Skipper> SingleLineDefinitionProcess;
	rule<Iterator, HeaderDefinition(), Skipper> HeaderVariableParser;
	rule<Iterator, ObjectFileDefinition(), Skipper> ObjectDefinitionParser;
	rule<Iterator, ObjectTemplateInstance(), Skipper> TemplateInstantiation;
	rule<Iterator, ObjectTemplateDef(), Skipper> TemplateDefinitionParser;

	// Object part rules //
	rule<Iterator, ObjectFileDefinition(), Skipper> ObjectBlockParser;
	rule<Iterator, SingleObjectContent(), Skipper> ObjectDataContentParser;
	rule<Iterator, ListDataHolder(), Skipper> ObjectListDataParser;
	rule<Iterator, TextBlockDataHolder(), Skipper> ObjectTextBlockParser;
	rule<Iterator, ObjectNameTypeDefinition(), Skipper> ObjectNameParser;


	// Helping rules //
	rule<Iterator, char()> EscapedCharacter;
	rule<Iterator, std::string()> BlockName;
	rule<Iterator, std::string()> QuotedString;

	// Handling phoenix functions
	boost::phoenix::function<ErrorReporter> ErrorReportObjFunc;
};


// ------------------ Processing function ------------------ //
DLLEXPORT std::vector<shared_ptr<ObjectFileObject>> Leviathan::ObjectFileProcessor::ProcessObjectFile(const std::wstring &file,
	std::vector<shared_ptr<NamedVariableList>> &HeaderVars)
{
	std::vector<shared_ptr<ObjectFileObject>> returned;

	// read the file entirely //
	std::string filecontents;

	std::string fileansi = Convert::WstringToString(file);

	try{
		FileSystem::ReadFileEntirely(fileansi, filecontents);
	}
	catch(const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"ObjectFileProcessor: ProcessObjectFile: file could not be read, exception:");
		e.PrintToLog();
		return returned;
	}
	
	// Skip empty files //
	if(filecontents.size() == 0){

		Logger::Get()->Warning(L"ObjectFileProcessor: file is empty, "+file);
		return returned;
	}

	// Skip the BOM if there is one //
	if(utf8::starts_with_bom(filecontents.begin(), filecontents.end())){

		// Pop the first 3 bytes //
		filecontents = filecontents.substr(3, filecontents.size()-3);
	}


	typedef boost::spirit::line_pos_iterator<std::string::const_iterator> source_iterator;

	typedef boost::u8_to_u32_iterator<source_iterator> iterator_type;

	source_iterator soi(filecontents.begin()), eoi(filecontents.end());
	iterator_type first(soi), last(eoi);


	// Parse the entire file using Boost::Spirit //
	ObjectFileGrammar<iterator_type> objectfilegrammar(fileansi);
	OFSkipper<iterator_type> skipper;


	ResultIntermediateType ParsedStructure;
	bool succeeded = phrase_parse(first, last, objectfilegrammar, skipper, ParsedStructure);

	if(!succeeded || first != last){

		// It failed //
		Logger::Get()->Error(L"ObjectFileProcessor: could not parse file: "+file);
		return returned;
	}


	// Post process the data into actual objects //
	WstringIterator itr(NULL, false);


	for(size_t i = 0; i < ParsedStructure.size(); i++){

		// Try to get all the pointers //
		HeaderDefinition* otype1 = boost::get<HeaderDefinition>(&ParsedStructure[i]);
		ObjectFileDefinition* otype2 = boost::get<ObjectFileDefinition>(&ParsedStructure[i]);
		ObjectTemplateInstance* otype3 = boost::get<ObjectTemplateInstance>(&ParsedStructure[i]);
		ObjectTemplateDef* otype4 = boost::get<ObjectTemplateDef>(&ParsedStructure[i]);

		if(otype1){

		}
	}


	return returned;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::ObjectFileProcessor::WriteObjectFile(std::vector<shared_ptr<ObjectFileObject>> &objects, const std::wstring &file, 
	std::vector<shared_ptr<NamedVariableList>> &headervars)
{
	// The output string which will then be written to the file //
	std::string generated;
	std::back_insert_iterator<std::string> sink(generated);



	return false;
}






