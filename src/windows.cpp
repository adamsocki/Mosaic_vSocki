#define WINDOWS 1

#include <ws2tcpip.h>
#include <Mstcpip.h> // only need this if turning on promiscuous mode for socket.

#include <windows.h>
#include <windowsx.h>


#include <mmdeviceapi.h>
#include <audioclient.h>

#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/wglew.h>

#include "types.h"

struct WindowsPlatform {
    HWND *window;
    int32 screenHeight;
    int32 screenWidth;

    WSADATA wsaData;
};

WindowsPlatform *Platform = NULL;

#include "windows_api.cpp"

#include <assert.h>
#define ASSERT(...) assert(__VA_ARGS__)

#include "game.cpp"

bool PlatformRunning = true;


struct OpenGLInfo {
    const uint8 *vendor;
    const uint8 *renderer;
    const uint8 *version;
    const uint8 *shadingLanguageVersion;
    const uint8 *extensions;
};

#define AUDIO_SAMPLERATE 44100
#define AUDIO_NUM_CHANNELS 2
#define AUDIO_BITS_PER_SAMPLE 32

struct WinAudioOutput {
    IMMDevice *device;
    IAudioClient *audioClient;
    IAudioRenderClient *renderClient;
    HANDLE renderEvent;
    HANDLE audioSema;

    WAVEFORMATEX waveFormat;

    uint32 bufferSampleCount;
    uint32 samplesRendered;
};

struct GamePlatform {
    bool running;
    WinAudioOutput audio;

    GameMemory gameMem;
};


LRESULT CALLBACK MainWindowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    switch(msg) {
        case WM_SIZE: {
        } break;

        case WM_DESTROY: {
            PlatformRunning = false;
        } break;

        case WM_CLOSE: {
            PlatformRunning = false;
        } break;

        case WM_QUIT: {
            PlatformRunning = false;
        } break;

        case WM_ACTIVATEAPP: {
        } break;

        // the default case means that if none of the other cases happened, we fall thru to this one
        // "Calls the default window procedure to provide default processing for any window messages
        // that an application does not process. This function ensures that every message is processed"
        default: {
            result = DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
    }


    return result;
}

#if 1
void InitOpenGL(HWND window, OpenGLInfo *glInfo) {
    HDC deviceContext = GetDC(window);

    PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
    desiredPixelFormat.nSize = sizeof(desiredPixelFormat);
    desiredPixelFormat.nVersion = 1;
    desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    desiredPixelFormat.cColorBits = 32;
    desiredPixelFormat.cAlphaBits = 8;
    desiredPixelFormat.cDepthBits = 24;
    desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int suggestedPixelFormatIndex = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
    PIXELFORMATDESCRIPTOR suggestedPixelFormat;
    DescribePixelFormat(deviceContext, suggestedPixelFormatIndex,
                        sizeof(suggestedPixelFormat), &suggestedPixelFormat);
    SetPixelFormat(deviceContext, suggestedPixelFormatIndex, &suggestedPixelFormat);
    
    HGLRC renderContext = wglCreateContext(deviceContext);
    if (wglMakeCurrent(deviceContext, renderContext)) {

        {
            glInfo->vendor = glGetString(GL_VENDOR);
            glCheckError();
            glInfo->renderer = glGetString(GL_RENDERER);
            glCheckError();
            glInfo->version = glGetString(GL_VERSION);
            glCheckError();
            glInfo->shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
            glCheckError();
            glInfo->extensions = glGetString(GL_EXTENSIONS);
            glCheckError();

            Print("%s", glInfo->vendor);
            Print("%s", glInfo->renderer);
            Print("%s", glInfo->version);
            Print("%s", glInfo->shadingLanguageVersion);
            Print("%s", glInfo->extensions);
        }

        GLenum err = glewInit();

        if (err == GLEW_OK) {

            ASSERT(WGLEW_ARB_create_context);

            int attribs[] = {
                             WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                             WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                             WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                             WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                             0
            };
                
            HGLRC sharedContext = 0;
            HGLRC modernContext = wglCreateContextAttribsARB(deviceContext, sharedContext, attribs);

            ASSERT(modernContext != NULL);
            wglMakeCurrent(deviceContext, modernContext);
            wglDeleteContext(renderContext);
            renderContext = modernContext;


            if (WGLEW_EXT_swap_control) {
                // vsync
                wglSwapIntervalEXT(1);
            }
        }
        else {
            Print((char *)glewGetErrorString(err));
        }
    }
}
#endif


void InitWASAPI(WinAudioOutput *audio) {
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
    HANDLE renderEvent;

    uint32 actualBufferSampleCount;

    REFERENCE_TIME bufferDuration = 1000000;

    HRESULT result;
    result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (SUCCEEDED(result)) {

        // @TODO: make sure to close this device later!
        result = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        if (SUCCEEDED(result)) {
            result = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
            if (SUCCEEDED(result)) {

                WAVEFORMATEX *waveFormat = &audio->waveFormat;
                waveFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
                waveFormat->nChannels = AUDIO_NUM_CHANNELS;
                waveFormat->nSamplesPerSec = AUDIO_SAMPLERATE;
                waveFormat->wBitsPerSample = AUDIO_BITS_PER_SAMPLE;
                waveFormat->nBlockAlign = (waveFormat->nChannels * waveFormat->wBitsPerSample) / 8;
                waveFormat->nAvgBytesPerSec = waveFormat->nSamplesPerSec * waveFormat->nBlockAlign;
                waveFormat->cbSize = 0;

                WAVEFORMATEXTENSIBLE *closestWaveFormat;
                result = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, waveFormat, (WAVEFORMATEX **)&closestWaveFormat);

                if (result == S_FALSE) {
                    if (closestWaveFormat->Format.nSamplesPerSec != waveFormat->nSamplesPerSec) {
                        // @TODO: need to resample, there appears to be an API...
                        // https://msdn.microsoft.com/en-us/library/windows/desktop/ff819070(v=vs.85).aspx
                        // https://sourceforge.net/p/playpcmwin/wiki/HowToUseResamplerMFT/
                    }

                    waveFormat = (WAVEFORMATEX *)closestWaveFormat;
                }
                    
                result = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                  AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                                  bufferDuration, 0, waveFormat, NULL);
                if (SUCCEEDED(result)) {

                    result = pAudioClient->GetBufferSize(&actualBufferSampleCount);
                    if (SUCCEEDED(result)) {

                        result = pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRenderClient);
                        if (SUCCEEDED(result)) {

                            renderEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                            result = pAudioClient->SetEventHandle(renderEvent);
                            if (SUCCEEDED(result)) {
                                // @NOTE: success!
                            }
                        }
                    }
                }
            }
        }
    }

    audio->device = pDevice;
    audio->audioClient = pAudioClient;
    audio->renderClient = pRenderClient;
    audio->renderEvent = renderEvent;
    audio->bufferSampleCount = actualBufferSampleCount;

    audio->audioSema = CreateSemaphoreEx(NULL, 1, 1, NULL, 0, SEMAPHORE_ALL_ACCESS);
}

void WASAPIThreadProc(void *data) {
    CoInitialize(NULL);

    GamePlatform *platform = (GamePlatform *)data;
    WinAudioOutput *audio = &platform->audio;

    audio->audioClient->Start();

    uint32 framesMissed = 0;

    while (platform->running) {
        WaitForSingleObject(audio->renderEvent, INFINITE);

        uint32 numFramesPadding;
        if (SUCCEEDED(audio->audioClient->GetCurrentPadding(&numFramesPadding))) {
            uint32 numFramesAvailable = audio->bufferSampleCount - numFramesPadding;

            if (numFramesAvailable == 0) {
                continue;
            }

            real32 *buffer;
            if (SUCCEEDED(audio->renderClient->GetBuffer(numFramesAvailable, (BYTE **)&buffer))) {

                if (WaitForSingleObject(audio->audioSema, 0) == WAIT_OBJECT_0) {

                    while (framesMissed > 0) {
                        uint32 framesToRender = Min(numFramesAvailable, framesMissed);
                        WriteSoundSamples(&platform->gameMem, framesToRender, buffer);
                        framesMissed -= framesToRender;
                    }
                    
                    WriteSoundSamples(&platform->gameMem, numFramesAvailable, buffer);
                    ReleaseSemaphore(audio->audioSema, 1, NULL);
                }
                else {
                    memset(buffer, 0, sizeof(real32) * 2 * numFramesAvailable);
                    framesMissed += numFramesAvailable;
                }

                audio->renderClient->ReleaseBuffer(numFramesAvailable, 0);
            }
        }
    }

    CoUninitialize();
}

void StartWASAPIThread(GamePlatform *platform) {
    DWORD threadID;
    HANDLE threadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WASAPIThreadProc, platform, 0, &threadID);
    CloseHandle(threadHandle);
}



void WindowsGetInput(InputQueue *inputQueue) {

    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

        switch (msg.message) {
            case WM_KEYDOWN: {
                TranslateMessage(&msg);
                
                int keycode = msg.wParam;

                if (keycode == 0x1B) {
                    PushInputPress(inputQueue, Input_Escape, 0);
                }
                
                if (keycode == VK_BACK) {
                    PushInputPress(inputQueue, Input_Backspace, 0);
                }

                if (keycode == VK_RETURN) {
                    PushInputPress(inputQueue, Input_Return, 0);
                }

                if (keycode == 0x41) {
                    PushInputPress(inputQueue, Input_Left, 0);
                }
                if (keycode == 0x44) {
                    PushInputPress(inputQueue, Input_Right, 0);
                }
                if (keycode == 0x53) {
                    PushInputPress(inputQueue, Input_Down, 0);
                }
                if (keycode == 0x57) {
                    PushInputPress(inputQueue, Input_Up, 0);
                }

                if (keycode == 0x25) {
                    PushInputPress(inputQueue, Input_LeftArrow, 0);
                }
                if (keycode == 0x26) {
                    PushInputPress(inputQueue, Input_UpArrow, 0);
                }
                if (keycode == 0x27) {
                    PushInputPress(inputQueue, Input_RightArrow, 0);
                }
                if (keycode == 0x28) {
                    PushInputPress(inputQueue, Input_DownArrow, 0);
                }


                if (keycode == 0x5A) {
                    PushInputPress(inputQueue, Input_Z, 0);
                }

                if (keycode == 0x52) {
                    PushInputPress(inputQueue, Input_R, 0);
                }

                if (keycode == VK_SPACE) {
                    PushInputPress(inputQueue, Input_Space, 0);
                }
            } break;

            case WM_KEYUP: {
                int keycode = msg.wParam;
                    
                if (keycode == 0x1B) {
                    PushInputRelease(inputQueue, Input_Escape, 0);
                }

                if (keycode == VK_BACK) {
                    PushInputRelease(inputQueue, Input_Backspace, 0);
                }

                if (keycode == VK_RETURN) {
                    PushInputRelease(inputQueue, Input_Return, 0);
                }

                
                if (keycode == 0x41) {
                    PushInputRelease(inputQueue, Input_Left, 0);
                }
                if (keycode == 0x44) {
                    PushInputRelease(inputQueue, Input_Right, 0);
                }
                if (keycode == 0x53) {
                    PushInputRelease(inputQueue, Input_Down, 0);
                }
                if (keycode == 0x57) {
                    PushInputRelease(inputQueue, Input_Up, 0);
                }

                
                if (keycode == 0x25) {
                    PushInputRelease(inputQueue, Input_LeftArrow, 0);
                }
                if (keycode == 0x26) {
                    PushInputRelease(inputQueue, Input_UpArrow, 0);
                }
                if (keycode == 0x27) {
                    PushInputRelease(inputQueue, Input_RightArrow, 0);
                }
                if (keycode == 0x28) {
                    PushInputRelease(inputQueue, Input_DownArrow, 0);
                }
                

                if (keycode == 0x5A) {
                    PushInputRelease(inputQueue, Input_Z, 0);
                }

                if (keycode == 0x52) {
                    PushInputRelease(inputQueue, Input_R, 0);
                }

                if (keycode == VK_SPACE) {
                    PushInputRelease(inputQueue, Input_Space, 0);
                }

            } break;

            case WM_CHAR: {
                uint16 character = (uint16)msg.wParam;
                inputQueue->inputChars[inputQueue->charCount++] = character;
            } break;

            case WM_MOUSEMOVE : {
                int posX = GET_X_LPARAM(msg.lParam);
                int posY = GET_Y_LPARAM(msg.lParam);

                inputQueue->mousePos.x = posX;
                inputQueue->mousePos.y = screenHeight - posY;
            } break;

                    
            case WM_LBUTTONDOWN : {
                PushInputPress(inputQueue, Input_MouseLeft, 0);
            } break;

            case WM_LBUTTONUP : {
                PushInputRelease(inputQueue, Input_MouseLeft, 0);
            } break;
                    
            case WM_RBUTTONDOWN : {
                PushInputPress(inputQueue, Input_MouseRight, 0);
            } break;

            case WM_RBUTTONUP : {
                PushInputRelease(inputQueue, Input_MouseRight, 0);
            } break;

            default : {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmndLine, int nCndShow) {

    GamePlatform platform = {};
    platform.running = true;

    char executablePath[MAX_PATH];

    GetModuleFileName(NULL, executablePath, sizeof(executablePath));
    char drive[MAX_PATH];
    char dir[MAX_PATH];
    _splitpath(executablePath, drive, dir, NULL, NULL);

    char newWorkingDirectory[MAX_PATH];
#if !ETC_RELEASE
    wsprintf(newWorkingDirectory, "%s%s..\\", drive, dir);
#else
    wsprintf(newWorkingDirectory, "%s%s", drive, dir);
#endif
    SetCurrentDirectory(newWorkingDirectory);




    WNDCLASS windowClass;
    memset(&windowClass, 0, sizeof(WNDCLASS));

    windowClass.lpszClassName = "OurGameWindowClass";

    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallback;
    windowClass.hInstance = hInstance;
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    int registerResult = RegisterClass(&windowClass);
    if (registerResult == 0) {
        printf("There was an error registering the class");
        return 0;
    }

    
    DWORD dwExStyle = 0; WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX;

    RECT windowRect;
    windowRect.left = 0;
    windowRect.right = screenWidth;
    windowRect.top = 0;
    windowRect.bottom = screenHeight; 
    
    HWND window = CreateWindowEx(dwExStyle, windowClass.lpszClassName, "GAME",
                                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 0, 0,
                                 NULL, NULL,
                                 hInstance, NULL);

    // @NOTE: the reason we do right - left, and bottom - top, is because AdjustWindowRect may change the
    // left/right/top/bottom coordinates of the rect, so the proper size is the difference between coordinates
    AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);
    
    SetWindowPos(window, HWND_NOTOPMOST,
                 64, 64,
                 windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                 SWP_NOZORDER);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    WindowsPlatform plat = {};

    Platform = &plat;
    plat.window = &window;
    plat.screenWidth = screenWidth;
    plat.screenHeight = screenHeight;



    OpenGLInfo glInfo;
    InitOpenGL(window, &glInfo);

    InitWASAPI(&platform.audio);

    StartWASAPIThread(&platform);

    LARGE_INTEGER startSystemTime;
    LARGE_INTEGER systemTime;
    LARGE_INTEGER systemFrequency;
    QueryPerformanceFrequency(&systemFrequency);
    QueryPerformanceCounter(&systemTime);

    startSystemTime = systemTime;
    SeedRand(startSystemTime.QuadPart);

    
    WSAStartup(MAKEWORD(2,2), &Platform->wsaData);

    // UDP 

    // Init Game Memomry
    GameMemory *gameMem = &platform.gameMem;
    memset(gameMem, 0, sizeof(GameMemory));

    // @GACK: need this for seeding the random number generator in GameInit
    // @BUG: It seems like the seeded value is almost always exactly the same tho?
    gameMem->systemTime = (real32)systemTime.QuadPart;

    GameInit(gameMem);
    gameMem->startTime = 0.0f;

    gameMem->running = true;

    InputQueue *inputQueue = &gameMem->inputQueue;

    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
    BITMAPINFO bitmapInfo;
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = screenWidth;
    bitmapInfo.bmiHeader.biHeight = -screenHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    // ALLOCATION/POINTERS
    int bitmapWidth = screenWidth;
    int bitmapHeight = screenHeight;
    int pixelCount = (bitmapWidth * bitmapHeight);
    int bytesPerPixel = 4; // one byte for each color
    int bitmapMemorySize = bytesPerPixel * pixelCount;

    
    HDC hdc = GetDC(window);

    real64 timeSinceRender = 0.0;

    HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
    SetCursor(cursor);
    //ShowCursor(false);
    WinMoveMouse(window, screenWidth / 2.0f, screenHeight / 2.0f, screenHeight);

    while(gameMem->running && PlatformRunning) {

        LARGE_INTEGER prevSystemTime = systemTime;
        int32 error = QueryPerformanceCounter(&systemTime);

        gameMem->deltaTime = ((real64)systemTime.QuadPart - (real64)prevSystemTime.QuadPart) / (real64)systemFrequency.QuadPart;

        gameMem->time += gameMem->deltaTime;

        timeSinceRender += gameMem->deltaTime;

        ClearInputQueue(inputQueue);
        WindowsGetInput(inputQueue);

        // @TODO: use an actual accumulator
        if (timeSinceRender < FRAME_RATE) {
            real64 timeUntilRender = FRAME_RATE - timeSinceRender;
            //Sleep(timeUntilRender * 1000);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GameUpdateAndRender(gameMem);

        HDC deviceContext = GetDC(window);
        SwapBuffers(deviceContext);
        ReleaseDC(window, deviceContext);
    }

    GameDeinit();

    WSACleanup();
    
    return 0;
}
