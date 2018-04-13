#include "FileGenerator.h"
#include <iterator>

using namespace std;


#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
    //_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    //_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

    string commandline = string(lpCmdLine);

#else
int main(int argcount, char* args[])
{

    string commandline = "";

    // Copy to one string //
    for(int i = 1; i < argcount; i++) {
        if(i != 1)
            commandline += " ";
        commandline += string(args[i]);
    }

#endif

    // Handle the commandline //
    std::vector<string> tokens;

    std::vector<size_t> cutpositions;

    bool incomment = false;
    bool started = false;

    // Some nasty stuff here //
    for(size_t i = 0; i < commandline.size(); i++) {
        if(commandline[i] == '"' && !incomment) {
            incomment = true;
            continue;
        }

        if(commandline[i] == '"' && incomment) {
            incomment = false;
            if(!started)
                continue;
            // End it //
            if(i > 0) {
                cutpositions.push_back(i - 1);
            }
            started = false;
            continue;
        }
        if(commandline[i] == ' ' && !incomment) {
            if(!started)
                continue;
            // End it //
            if(i > 0) {
                cutpositions.push_back(i - 1);
            }
            started = false;
            continue;
        }
        if(!started) {
            cutpositions.push_back(i);
            started = true;
        }
    }

    if(cutpositions.size() % 2 != 0)
        cutpositions.push_back(commandline.size() - 1);

    // Cut them to strings //
    for(size_t i = 0; i < cutpositions.size(); i += 2) {

        tokens.push_back(
            commandline.substr(cutpositions[i], cutpositions[i + 1] - cutpositions[i] + 1));
    }


    // Check first one //
    if(tokens.size() != 3 && tokens.size() != 4) {

        cout << "Invalid commandline, expected 3 or 4 arguments (got " << tokens.size() << ")"
             << endl;
        for(const auto& token : tokens)
            cout << token << std::endl;
        return -1;
    }

    if(tokens[0] == "V8EXT") {

        if(!FileGenerator::DoJSExtensionGeneration(
               tokens[1], tokens[2], tokens.size() > 3 ? tokens[3] : "Leviathan")) {

            cout << "Generating error" << endl;
            return -1;
        }
        return 0;
    }


    cout << "Invalid commandline, unknown command" << endl;
    return -1;
}
