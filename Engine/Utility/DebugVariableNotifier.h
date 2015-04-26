#pragma once
// ------------------------------------ //
#include <map>
#include <string>
#include <memory>

#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

	class DebugVariableNotifier{
	public:

		DLLEXPORT static void UpdateVariable(const std::string &name,
            VariableBlock* valueofvariable);

		// prints all values to log //
		DLLEXPORT static void PrintVariables();

		DebugVariableNotifier() = delete;

	protected:
		// stored variables //
		static std::map<std::string, std::shared_ptr<VariableBlock>> CapturedValues;

	};

}

