// ------------------------------------ //
#include "JavaScriptHelper.h"

#include "Common/DataStoring/NamedVars.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT CefRefPtr<CefV8Value>
    Leviathan::JavaScriptHelper::ConvertNamedVariableListToJavaScriptValue(
        NamedVariableList* obj)
{
    // Check variable count //
    if(obj->GetVariableCount() > 1) {
        // We need to extract all objects //
        size_t count = obj->GetVariableCount();

        // Create array //
        CefRefPtr<CefV8Value> arrayval = CefV8Value::CreateArray((int)count);

        // Add all the objects //
        for(int i = 0; i < (int)count; i++) {

            // Set the object //
            arrayval->SetValue(
                i, ConvertVariableBlockToJavaScriptValue(obj->GetValueDirect(i)));
        }

        return arrayval;

    } else if(obj->GetVariableCount() == 1) {
        // Get the block //
        VariableBlock* block = obj->GetValueDirect(0);

        return ConvertVariableBlockToJavaScriptValue(block);
    }

    // No value //
    return NULL;
}
// ------------------------------------ //
DLLEXPORT CefRefPtr<CefV8Value>
    Leviathan::JavaScriptHelper::ConvertVariableBlockToJavaScriptValue(VariableBlock* block)
{
    // Switch on type //
    switch(block->GetBlockConst()->Type) {
    case DATABLOCK_TYPE_INT: {
        return CefV8Value::CreateInt(block->ConvertAndReturnVariable<int>());
    }
    case DATABLOCK_TYPE_DOUBLE:
    case DATABLOCK_TYPE_FLOAT: {
        return CefV8Value::CreateDouble(block->ConvertAndReturnVariable<double>());
    }
    case DATABLOCK_TYPE_BOOL: {
        return CefV8Value::CreateBool(block->ConvertAndReturnVariable<bool>());
    }
    case DATABLOCK_TYPE_WSTRING: {
        return CefV8Value::CreateString(block->ConvertAndReturnVariable<std::wstring>());
    }
    case DATABLOCK_TYPE_CHAR:
    case DATABLOCK_TYPE_STRING: {
        return CefV8Value::CreateString(block->ConvertAndReturnVariable<std::string>());
    }
    default:
        // Cannot be converted //
        return NULL;
    }
}
// ------------------------------------ //
