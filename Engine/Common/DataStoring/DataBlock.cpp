#include "Include.h"
// ------------------------------------ //
#include "../StringOperations.h"
#include "DataBlock.h"
#include "Iterators/StringIterator.h"
#include <float.h>

#ifdef LEVIATHAN_USING_ANGELSCRIPT
#include "Script/ScriptExecutor.h"
#endif // LEVIATHAN_USING_ANGELSCRIPT
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::VariableBlock::VariableBlock(
    const std::string& valuetoparse, map<string, std::shared_ptr<VariableBlock>>* predefined)
{
    // the text should have all preceding and trailing spaces removed //
    if(valuetoparse.size() == 0) {
    // can't be anything //
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("no data passed");
#else
        return;
#endif
    }
    // try to figure out what type of block is required for this variable //

    // easy one first, quotes //
    if(valuetoparse[0] == '"') {
        // it's a string //

        // use iterator to get data inside quotes //
        StringIterator itr(valuetoparse);

        auto tempdata = itr.GetStringInQuotes<std::string>(QUOTETYPE_DOUBLEQUOTES);

        // set data //
        // WstringBlock takes the pointer as it's own here //
        BlockData = new StringBlock(tempdata ? tempdata.release() : new std::string());

        return;
    }

    // check does it contain non numeric characters //
    if(!StringOperations::IsStringNumeric<std::string>(valuetoparse)) {

        // check does it match true/false //
        bool possiblevalue = false;

        if(Convert::IsStringBool(valuetoparse, &possiblevalue)) {

            BlockData = new BoolBlock(possiblevalue);

            return;
        }

        // check does some special value match it //
        if(predefined != NULL) {
            // check do them match //

            auto ivaliterator = predefined->find(valuetoparse);

            if(ivaliterator != predefined->end()) {
                // found! //

                BlockData = ivaliterator->second->GetBlockConst()->AllocateNewFromThis();
                return;
            }
        }

        // create a string from the whole thing //
        BlockData = new StringBlock(valuetoparse);

        return;
    }

    // Try to figure out what kind of a number it is //
    size_t decimalspot = valuetoparse.find_first_of('.');
    if(decimalspot != std::wstring::npos) {
        // has decimal separator //

        // check does it need more decimal digits than a float has //

        if(valuetoparse.size() - 1 - decimalspot > FLT_DIG) {
            // create a double //
            BlockData = new DoubleBlock(Convert::StringTo<double>(valuetoparse));

        } else {

            // float should have space to hold all characters //
            BlockData = new FloatBlock(Convert::StringTo<float>(valuetoparse));
        }

        return;
    }

    // Should be a plain old int //
    BlockData = new IntBlock(Convert::StringTo<int>(valuetoparse));
}


// ------------------ ScriptSafeVariableBlock ------------------ //
#ifdef LEVIATHAN_USING_ANGELSCRIPT
Leviathan::ScriptSafeVariableBlock::ScriptSafeVariableBlock(
    VariableBlock* copyfrom, const std::string& name) :
    NamedVariableBlock(copyfrom->GetBlock()->AllocateNewFromThis(), name)
{
    // we need to copy all settings from the block //
    switch(copyfrom->GetBlock()->Type) {
    case DATABLOCK_TYPE_INT:
        ASTypeID = AngelScriptTypeIDResolver<int>::Get(ScriptExecutor::Get());
        break;
    case DATABLOCK_TYPE_FLOAT:
        ASTypeID = AngelScriptTypeIDResolver<float>::Get(ScriptExecutor::Get());
        break;
    case DATABLOCK_TYPE_BOOL:
        ASTypeID = AngelScriptTypeIDResolver<bool>::Get(ScriptExecutor::Get());
        break;
    case DATABLOCK_TYPE_WSTRING: {
        // we'll use automatic conversion here //
        unique_ptr<DataBlockAll> tmp(new StringBlock(ConvertAndReturnVariable<std::string>()));

        SAFE_DELETE(BlockData);
        BlockData = tmp.release();

        ASTypeID = AngelScriptTypeIDResolver<std::string>::Get(ScriptExecutor::Get());
        break;
    } break;
    case DATABLOCK_TYPE_STRING:
        ASTypeID = AngelScriptTypeIDResolver<string>::Get(ScriptExecutor::Get());
        break;
    case DATABLOCK_TYPE_CHAR:
        ASTypeID = AngelScriptTypeIDResolver<char>::Get(ScriptExecutor::Get());
        break;
    case DATABLOCK_TYPE_DOUBLE:
        ASTypeID = AngelScriptTypeIDResolver<double>::Get(ScriptExecutor::Get());
        break;

    default:
        throw InvalidArgument("cannot convert non-named, generic type block to script "
                              "safe block");
    }
}
#endif // LEVIATHAN_USING_ANGELSCRIPT
// ------------------ Loading/saving from/to packets ------------------ //
#define DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(BlockTypeName, VarTypeName, TmpTypeName) \
    template<>                                                                         \
    DLLEXPORT void BlockTypeName::AddDataToPacket(sf::Packet& packet)                  \
    {                                                                                  \
        packet << *Value;                                                              \
    }                                                                                  \
    template<>                                                                         \
    DLLEXPORT BlockTypeName::DataBlock(sf::Packet& packet)                             \
    {                                                                                  \
        Type = DataBlockNameResolver<VarTypeName>::TVal;                               \
        TmpTypeName tmpval;                                                            \
        if(!(packet >> tmpval)) {                                                      \
            throw InvalidArgument("invalid packet format");                            \
        }                                                                              \
        Value = new VarTypeName(tmpval);                                               \
    }



// ------------------ Loading/saving from/to packets ------------------ //
#ifdef SFML_PACKETS
namespace Leviathan {
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(IntBlock, int, int);
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(FloatBlock, float, float);
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(BoolBlock, bool, bool);
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(WstringBlock, wstring, wstring);
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(StringBlock, string, string);
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(DoubleBlock, double, double);
DEFAULTTOANDFROMPACKETCONVERTFUNCTINS(CharBlock, char, sf::Int8);



// Fill in the gaps in the templates with these defaults //
template<class DBlockT>
DLLEXPORT void Leviathan::DataBlock<DBlockT>::AddDataToPacket(sf::Packet&)
{
    // The default one cannot do anything, only the specialized functions can try to do
    // something
    throw Exception("this type doesn't support saving to a packet");
}
template<class DBlockT>
DLLEXPORT Leviathan::DataBlock<DBlockT>::DataBlock(sf::Packet&)
{
    // The default one cannot do anything, only the specialized functions can try to do
    // something
    throw Exception("this type doesn't support loading from a packet");
}

// ------------------ VariableBlock ------------------ //
DLLEXPORT void VariableBlock::AddDataToPacket(sf::Packet& packet) const
{
    // Set the type //
    if(BlockData != NULL) {
        packet << BlockData->Type;
    } else {
        packet << 0;
        return;
    }

    // Set the data //
    if(BlockData->Type == DATABLOCK_TYPE_INT)
        return TvalToTypeResolver<DATABLOCK_TYPE_INT>::Conversion(BlockData)->AddDataToPacket(
            packet);
    else if(BlockData->Type == DATABLOCK_TYPE_FLOAT)
        return TvalToTypeResolver<DATABLOCK_TYPE_FLOAT>::Conversion(BlockData)
            ->AddDataToPacket(packet);
    else if(BlockData->Type == DATABLOCK_TYPE_BOOL)
        return TvalToTypeResolver<DATABLOCK_TYPE_BOOL>::Conversion(BlockData)->AddDataToPacket(
            packet);
    else if(BlockData->Type == DATABLOCK_TYPE_WSTRING)
        return TvalToTypeResolver<DATABLOCK_TYPE_WSTRING>::Conversion(BlockData)
            ->AddDataToPacket(packet);
    else if(BlockData->Type == DATABLOCK_TYPE_STRING)
        return TvalToTypeResolver<DATABLOCK_TYPE_STRING>::Conversion(BlockData)
            ->AddDataToPacket(packet);
    else if(BlockData->Type == DATABLOCK_TYPE_CHAR)
        return TvalToTypeResolver<DATABLOCK_TYPE_CHAR>::Conversion(BlockData)->AddDataToPacket(
            packet);
    else if(BlockData->Type == DATABLOCK_TYPE_DOUBLE)
        return TvalToTypeResolver<DATABLOCK_TYPE_DOUBLE>::Conversion(BlockData)
            ->AddDataToPacket(packet);

    // type that shouldn't be used is used //
    throw InvalidType("unallowed datatype in datablock for writing to packet");
}

DLLEXPORT VariableBlock::VariableBlock(sf::Packet& packet)
{

    // Get the type //
    short type;
    packet >> type;

    // Load the actual data based on the type //
    switch(type) {
    case 0: {
        // No data //
        BlockData = NULL;
        return;
    }
    case DATABLOCK_TYPE_INT: {
        BlockData = new IntBlock(packet);
        return;
    }
    case DATABLOCK_TYPE_FLOAT: {
        BlockData = new FloatBlock(packet);
        return;
    }
    case DATABLOCK_TYPE_BOOL: {
        BlockData = new BoolBlock(packet);
        return;
    }
    case DATABLOCK_TYPE_WSTRING: {
        BlockData = new WstringBlock(packet);
        return;
    }
    case DATABLOCK_TYPE_STRING: {
        BlockData = new StringBlock(packet);
        return;
    }
    case DATABLOCK_TYPE_CHAR: {
        BlockData = new CharBlock(packet);
        return;
    }
    case DATABLOCK_TYPE_DOUBLE: {
        BlockData = new DoubleBlock(packet);
        return;
    }
    }

    // Invalid packet //
    throw InvalidArgument("invalid packet format");
}
} // namespace Leviathan
#endif // SFML_PACKETS

namespace Leviathan {

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const VariableBlock& value)
{

    if(!value.GetBlockConst()) {

        stream << "Empty Variable";
        return stream;
    }

    if(!value.IsConversionAllowedNonPtr<std::string>()) {

        stream << "No Text Presentation";
        return stream;
    }

    stream << value.operator std::string();
    return stream;
}

} // namespace Leviathan
