#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <audioclient.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker, "/ENTRY:mainCRTStartup")

static bool consoleAttached() {
    return GetConsoleWindow() != NULL;
}

static void print_out(const char* msg) {
    if (!consoleAttached()) return;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE || hOut == NULL) return;
    DWORD written;
    WriteFile(hOut, msg, (DWORD)strlen(msg), &written, NULL);
}

static void print_err(const char* msg) {
    if (!consoleAttached()) return;
    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hErr == INVALID_HANDLE_VALUE || hErr == NULL) return;
    DWORD written;
    WriteFile(hErr, msg, (DWORD)strlen(msg), &written, NULL);
}

static void print_errf(const char* fmt, ...) {
    if (!consoleAttached()) return;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    print_err(buf);
}

static const char* hresult_str(HRESULT hr) {
    switch (hr) {
        case E_OUTOFMEMORY:                  return "out of memory";
        case E_INVALIDARG:                   return "invalid argument";
        case E_NOINTERFACE:                  return "interface not supported";
        case E_POINTER:                      return "null pointer";
        case E_HANDLE:                       return "invalid handle";
        case E_NOTIMPL:                      return "not implemented";
        case E_ACCESSDENIED:                 return "access denied";
        case AUDCLNT_E_DEVICE_INVALIDATED:   return "audio device was removed";
        case AUDCLNT_E_SERVICE_NOT_RUNNING:  return "Windows Audio service is not running";
        case CO_E_NOTINITIALIZED:            return "COM not initialized";
        case REGDB_E_CLASSNOTREG:            return "class not registered";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "HRESULT 0x%08X", (unsigned)hr);
            return buf;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_err("Usage: volume.exe <0-100>\n");
        return 1;
    }

    // Validate input is numeric
    const char* arg = argv[1];
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] < '0' || arg[i] > '9') {
            print_errf("Error: Invalid value \"%s\". Must be a number between 0 and 100.\n", arg);
            return 1;
        }
    }

    int level = atoi(arg);
    if (level < 0 || level > 100) {
        print_errf("Error: Volume %d is out of range. Must be between 0 and 100.\n", level);
        return 1;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        print_errf("Error: COM initialization failed (%s).\n", hresult_str(hr));
        return 1;
    }

    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioEndpointVolume* pVolume = NULL;
    int exitCode = 0;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator
    );
    if (FAILED(hr)) {
        print_errf("Error: Failed to create device enumerator (%s).\n", hresult_str(hr));
        exitCode = 1; goto cleanup;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (hr == E_NOTFOUND) {
        print_err("Error: No audio output device found.\n");
        exitCode = 1; goto cleanup;
    }
    if (FAILED(hr)) {
        print_errf("Error: Failed to get default audio endpoint (%s).\n", hresult_str(hr));
        exitCode = 1; goto cleanup;
    }

    hr = pDevice->Activate(
        __uuidof(IAudioEndpointVolume), CLSCTX_ALL,
        NULL, (void**)&pVolume
    );
    if (FAILED(hr)) {
        print_errf("Error: Failed to activate audio volume control (%s).\n", hresult_str(hr));
        exitCode = 1; goto cleanup;
    }

    hr = pVolume->SetMasterVolumeLevelScalar(level / 100.0f, NULL);
    if (FAILED(hr)) {
        print_errf("Error: Failed to set volume (%s).\n", hresult_str(hr));
        exitCode = 1; goto cleanup;
    }

    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Volume set to %d%%\n", level);
        print_out(buf);
    }

cleanup:
    if (pVolume)     pVolume->Release();
    if (pDevice)     pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();
    return exitCode;
}