#include <windows.h>
#include <iostream>
#include <string>
#include <napi.h>
#include <windows.h>
#include <TlHelp32.h>
#include "util.h"

const char* PREFIX = "\x1b[31mnullbyte \x1b[0m~ ";

Napi::Value patch(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "first argument pid must be of type number").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    if (!info[1].IsArray()) {
        Napi::TypeError::New(env, "second argument patterns must be of type string[]").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    if (info[2] && !info[2].IsBoolean()) {
        Napi::TypeError::New(env, "third argument matchOne must be of type boolean").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    int pid = info[0].As<Napi::Number>().Int32Value();
    Napi::Array patterns = info[1].As<Napi::Array>();
    bool matchOne = info[2] ? info[2].As<Napi::Boolean>().ToBoolean() : false;

    std::cout << PREFIX << "attempting to attach process" << std::endl;

    char* error = "";
    auto process = Util::openProcess(pid, &error);
    if (error != "") {
        std::cout << PREFIX << "failed to attach to process" << std::endl;
        return Napi::Boolean::New(env, false);
    }

    std::cout << PREFIX << "process attached successfully" << std::endl;

    int failed = 0;
    for (int i = 0; i < patterns.Length(); i++) {
        int patternIndex = i + 1;

        std::cout << PREFIX << "pattern " << patternIndex << ": searching" << std::endl;

        std::string pattern(patterns.Get(i).As<Napi::String>().Utf8Value());

        char* regionsError = "";
        std::vector<MODULEENTRY32> modules = Util::getModules(GetProcessId(process.handle), &regionsError);

        if (regionsError != "") {
            std::cout << PREFIX << "failed to get process modules: " << regionsError << std::endl;
            failed++;
            continue;
        }

        uintptr_t address = 0;

        bool regions = Util::searchPattern(process.handle, modules, 0, pattern.c_str(), 0, 0, &address);
        if (!regions || address == 0) {
            std::cout << PREFIX << "trying to find module";

            address = Util::findPatternByModule(process.handle, process.process.szExeFile, pattern.c_str(), 0, 0);

            if (address == 0) {
                std::cout << PREFIX << "pattern " << patternIndex << ": failed to find match" << std::endl;
                failed++;
                continue;
            }
        }

        std::cout << PREFIX << "pattern " << patternIndex << ": match found" << std::endl;
        std::cout << PREFIX << "pattern " << patternIndex << ": attempting to null out bytes" << std::endl;
        std::vector segments = Util::split(const_cast<char*>(pattern.c_str()), " ");

        int bytes = 0;
        for (auto i : segments) {
            DWORD64 addr = address;

            std::cout << PREFIX << "pattern " << patternIndex << ": spraying " << std::left << "0x" << std::hex << addr << std::endl;
            byte value[] = { 0x90 };

            WriteProcessMemory(process.handle, (LPVOID)addr, value, sizeof(value), NULL);
            bytes++;
            address++;
        }

        std::cout << PREFIX << "pattern " << patternIndex << ": successfully sprayed " << bytes << " bytes" << std::endl;
    }

    return Napi::Boolean::New(env, matchOne ? (failed != patterns.Length() ? true : false) : (failed > 0 ? false : true));
}

Napi::Object initialize(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "patch"), Napi::Function::New(env, patch));
    return exports;
}

NODE_API_MODULE(addon, initialize)