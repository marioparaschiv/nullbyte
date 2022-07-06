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

    bool isValidPID = info[0].IsNumber();
    bool hasValidPatterns = info[1].IsArray();

    if (!isValidPID) {
        Napi::TypeError::New(env, "first argument pid must be of type number").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!hasValidPatterns) {
        Napi::TypeError::New(env, "second argument patterns must be of type string[]").ThrowAsJavaScriptException();
        return env.Null();
    }

    int pid = info[0].As<Napi::Number>().Int32Value();
    Napi::Array buf = info[1].As<Napi::Array>();

    std::cout << PREFIX << "attempting to attach process" << "\n";

    char* error = "";
    auto process = Util::openProcess(pid, &error);
    if (error != "") {
        std::cout << PREFIX << "failed to attach to process" << "\n";
        return env.Null();
    }

    for (int i = 0; i < buf.Length(); i++) {
        std::cout << PREFIX << "searching with pattern " << i + 1 << "\n";
        auto pattern = buf.Get(i).As<Napi::String>().Utf8Value();

        char* c = const_cast<char*>(pattern.c_str());

        auto res = Util::findPatternByModule(process.handle, process.process.szExeFile, pattern.c_str(), 0, 0);

        if (res == 0) {
            std::cout << PREFIX << "failed to find one of the patterns provided" << "\n";
            continue;
        }

        std::cout << PREFIX << "pattern " << i + 1 << ": match found" << "\n";
        std::cout << PREFIX << "pattern " << i + 1 << ": attempting to null out bytes" << "\n";
        auto segments = Util::split(const_cast<char*>(pattern.c_str()), " ");

        int bytes = 0;
        for (auto i : segments) {
            DWORD64 addr = res;
            byte value[] = { 0x90 };

            WriteProcessMemory(process.handle, (LPVOID)addr, value, sizeof(value), NULL);
            bytes++;
            res++;
        }

        std::cout << PREFIX << "pattern " << i + 1 << ": successfully sprayed " << bytes << " bytes" << "\n";
    }
    
    return env.Null();
}

Napi::Object initialize(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "patch"), Napi::Function::New(env, patch));
    return exports;
}

NODE_API_MODULE(addon, initialize)
