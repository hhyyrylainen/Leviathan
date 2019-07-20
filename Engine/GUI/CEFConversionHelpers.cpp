// ------------------------------------ //
#include "CEFConversionHelpers.h"

#include "Common/DataStoring/NamedVars.h"
// ------------------------------------ //
namespace Leviathan {

DLLEXPORT void JSArrayToList(
    const CefRefPtr<CefV8Value>& source, const CefRefPtr<CefListValue>& target)
{
    const auto sourceLength = source->GetArrayLength();

    // Nothing needs to be done to convert empty arrays
    if(sourceLength == 0)
        return;

    // We set the size first to avoid reallocations
    target->SetSize(sourceLength);

    for(int i = 0; i < sourceLength; ++i) {

        const CefRefPtr<CefV8Value>& value = source->GetValue(i);

        if(value->IsBool()) {

            target->SetBool(i, value->GetBoolValue());

        } else if(value->IsInt() || value->IsUInt()) {

            target->SetInt(i, value->GetIntValue());

        } else if(value->IsDouble()) {

            target->SetDouble(i, value->GetDoubleValue());

        } else if(value->IsNull()) {

            target->SetNull(i);

        } else if(value->IsString() || value->IsDate()) {

            target->SetString(i, value->GetStringValue());

        } else if(value->IsArray()) {

            CefRefPtr<CefListValue> list = CefListValue::Create();
            JSArrayToList(value, list);
            target->SetList(i, list);

        } else if(value->IsObject()) {
            CefRefPtr<CefDictionaryValue> dictionary = CefDictionaryValue::Create();
            JSObjectToDictionary(value, dictionary);
            target->SetDictionary(i, dictionary);
        }
    }
}

DLLEXPORT void JSObjectToDictionary(
    const CefRefPtr<CefV8Value>& source, const CefRefPtr<CefDictionaryValue>& target)
{
    std::vector<CefString> keys;
    source->GetKeys(keys);

    for(const auto& key : keys) {
        const CefRefPtr<CefV8Value>& value = source->GetValue(key);

        // Check type and convert
        if(value->IsBool()) {

            target->SetBool(key, value->GetBoolValue());

        } else if(value->IsDouble()) {

            target->SetDouble(key, value->GetDoubleValue());

        } else if(value->IsInt() || value->IsUInt()) {

            target->SetInt(key, value->GetIntValue());

        } else if(value->IsNull()) {

            target->SetNull(key);

        } else if(value->IsString() || value->IsDate()) {

            target->SetString(key, value->GetStringValue());

        } else if(value->IsArray()) {

            CefRefPtr<CefListValue> list = CefListValue::Create();
            JSArrayToList(value, list);
            target->SetList(key, list);

        } else if(value->IsObject()) {

            CefRefPtr<CefDictionaryValue> dictionaryValue = CefDictionaryValue::Create();
            JSObjectToDictionary(value, dictionaryValue);
            target->SetDictionary(key, dictionaryValue);
        }
    }
}
// ------------------------------------ //
template<class SourceT, class KeyT>
std::unique_ptr<DataBlockAll> HandleSimpleVar(
    CefValueType type, const CefRefPtr<SourceT>& source, const KeyT& key)
{
    switch(type) {
    case VTYPE_BOOL: {
        return std::make_unique<BoolBlock>(source->GetBool(key));
        break;
    }
    case VTYPE_DOUBLE: {
        return std::make_unique<DoubleBlock>(source->GetDouble(key));
        break;
    }
    case VTYPE_INT: {
        return std::make_unique<IntBlock>(source->GetInt(key));
        break;
    }
    case VTYPE_STRING: {
        return std::make_unique<StringBlock>(source->GetString(key));
        break;
    }
    case VTYPE_NULL: {
        return std::make_unique<BoolBlock>(false);
        break;
    }
    default: return nullptr;
    }
}

DLLEXPORT void CEFDictionaryToNamedVars(
    const CefRefPtr<CefDictionaryValue>& source, NamedVars& destination)
{
    std::vector<CefString> keys;
    if(!source->GetKeys(keys)) {
        LOG_ERROR("CEFDictionaryToNamedVars: GetKeys failed");
        return;
    }

    for(const auto& key : keys) {

        CefValueType type = source->GetType(key);

        auto simpleHandle = HandleSimpleVar(type, source, key);

        if(simpleHandle) {
            destination.Add(std::make_shared<NamedVariableList>(
                key.ToString(), new VariableBlock(simpleHandle.release())));
        } else {
            switch(type) {
            case VTYPE_LIST: {

                const CefRefPtr<CefListValue>& sourceList = source->GetList(key);

                auto list = std::make_shared<NamedVariableList>(key.ToString());

                for(size_t i = 0; i < sourceList->GetSize(); ++i) {

                    auto handled = HandleSimpleVar(sourceList->GetType(i), sourceList, i);

                    if(!handled) {

                        LOG_ERROR("CEFDictionaryToNamedVars: list has invalid value (only "
                                  "simple types in lists are supported)");
                    } else {

                        list->PushValue(std::make_unique<VariableBlock>(handled.release()));
                    }
                }

                destination.Add(list);
                break;
            }
            case VTYPE_DICTIONARY: {
                LOG_ERROR("CEFDictionaryToNamedVars: recursive NamedVars is not supported");
                break;
            }
            default:
                LOG_ERROR("CEFDictionaryToNamedVars: unknown type to convert: " +
                          std::to_string(type));
                break;
            }
        }
    }
}
} // namespace Leviathan
