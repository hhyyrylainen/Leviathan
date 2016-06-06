#pragma once
// ------------------------------------ //
#include "Include.h"

#include "ErrorReporter.h"

namespace Leviathan{

    //! \brief Logger public interface
	class Logger : public LErrorReporter{
	public:
        
		DLLEXPORT Logger(const std::string &file);
		DLLEXPORT ~Logger();

        DLLEXPORT void Write(const std::string &data);


        DLLEXPORT void Info(const std::string &data);
        DLLEXPORT void Error(const std::string &data);
        DLLEXPORT void Warning(const std::string &data);

        DLLEXPORT static void SendDebugMessage(const std::string &str);

		//! \brief Script wrapper
		DLLEXPORT static void Print(const std::string &message);

		DLLEXPORT void Save();

        //! \brief Adds raw data to the queue unmodified
        //! \note You will need to add new lines '\n' manually
        DLLEXPORT void DirectWriteBuffer(const std::string &data);


		DLLEXPORT static Logger* Get();
        

        DLLEXPORT void WriteLine(const std::string &Text) override;
        DLLEXPORT void Fatal(const std::string &Text) override;

    private:

        //! Call only after locking
        void _Save();

        //! \note The global logger mutex needs to be locked before this call
		void _LogUpdateEndPart();

		// ------------------------------------ //
        std::string PendingLog;
		std::string Path;

		static Logger* LatestLogger;
	};
}

