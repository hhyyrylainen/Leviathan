#pragma once
// ------------------------------------ //
#include <vector>
#include <string>
#include <memory>

namespace Leviathan{

    //! \todo Make this thread safe
	class ComplainOnce{
	public:

		DLLEXPORT static bool PrintWarningOnce(const std::string& warning,
            const std::string& message);
		DLLEXPORT static bool PrintErrorOnce(const std::string& error, const std::string& message);

	private:
		ComplainOnce() = delete;

		// fired warnings/errors //
		static std::vector<std::shared_ptr<std::string>> FiredErrors;
	};

}

