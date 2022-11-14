#include "SdlWindow.h"

#include <SDL_syswm.h>
#ifdef None
#undef None
#endif

#include <cassert>
#include <utility>

#include "Platform/PlatformEventHandler.h"

#include "SdlPlatformSharedState.h"
#include "SdlOpenGLContext.h"

SdlWindow::SdlWindow(SdlPlatformSharedState *state, std::unique_ptr<PlatformEventHandler> eventHandler, SDL_Window *window, uint32_t id):
    state_(state), eventHandler_(std::move(eventHandler)), window_(window), id_(id) {
    assert(state_);
    assert(eventHandler_);
    assert(window_);
    assert(id_);
}

SdlWindow::~SdlWindow() {
    state_->UnregisterWindow(this);
    SDL_DestroyWindow(window_);
}

void SdlWindow::SetTitle(const std::string &title) {
    SDL_SetWindowTitle(window_, title.c_str());
}

std::string SdlWindow::Title() const {
    return SDL_GetWindowTitle(window_);
}

void SdlWindow::Resize(const Sizei &size) {
    SDL_SetWindowSize(window_, size.w, size.h);
}

Sizei SdlWindow::Size() const {
    Sizei result;
    SDL_GetWindowSize(window_, &result.w, &result.h);
    return result;
}

void SdlWindow::SetPosition(const Pointi &pos) {
    SDL_SetWindowPosition(window_, pos.x, pos.y);
}

Pointi SdlWindow::Position() const {
    Pointi result;
    SDL_GetWindowPosition(window_, &result.x, &result.y);
    return result;
}

void SdlWindow::SetVisible(bool visible) {
    if (visible) {
        SDL_ShowWindow(window_);
    } else {
        SDL_HideWindow(window_);
    }
}

bool SdlWindow::IsVisible() const {
    uint32_t flags = SDL_GetWindowFlags(window_);
    if (flags & SDL_WINDOW_SHOWN)
        return true;
    if (flags & SDL_WINDOW_HIDDEN)
        return false;

    assert(false); // shouldn't get here.
    return false;
}

void SdlWindow::SetFullscreen(bool fullscreen) {
    if (SDL_SetWindowFullscreen(window_, fullscreen ? SDL_WINDOW_FULLSCREEN : 0) != 0)
        state_->LogSdlError("SDL_SetWindowFullscreen");
}

bool SdlWindow::IsFullscreen() const {
    uint32_t flags = SDL_GetWindowFlags(window_);
    return flags & SDL_WINDOW_FULLSCREEN;
}

void SdlWindow::SetFrameless(bool frameless) {
    SDL_SetWindowBordered(window_, frameless ? SDL_FALSE : SDL_TRUE);
}

bool SdlWindow::IsFrameless() const {
    uint32_t flags = SDL_GetWindowFlags(window_);
    return flags & SDL_WINDOW_BORDERLESS;
}

void SdlWindow::SetGrabsMouse(bool grabsMouse) {
    SDL_SetWindowGrab(window_, grabsMouse ? SDL_TRUE : SDL_FALSE);
}

bool SdlWindow::GrabsMouse() const {
    return SDL_GetWindowGrab(window_) == SDL_TRUE;
}

void SdlWindow::Activate() {
    SDL_RaiseWindow(window_);
}

uintptr_t SdlWindow::SystemHandle() const {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(window_, &info) != SDL_TRUE) {
        state_->LogSdlError("SDL_GetWindowWMInfo");
        return 0;
    }

#ifdef _WINDOWS
    return reinterpret_cast<uintptr_t>(info.info.win.window);
#elif __APPLE__
    return reinterpret_cast<uintptr_t>(info.info.cocoa.window);
#else
    return static_cast<uintptr_t>(info.info.x11.window);
#endif
}

std::unique_ptr<PlatformOpenGLContext> SdlWindow::CreateOpenGLContext(const PlatformOpenGLOptions &options) {
    int version;

    if (options.versionMajor != -1)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, options.versionMajor);

    if (options.versionMinor != -1)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, options.versionMinor);

    if (options.depthBits != -1)
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, options.depthBits);

    if (options.stencilBits != -1)
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, options.stencilBits);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, options.doubleBuffered);

    SDL_GLContext ctx = SDL_GL_CreateContext(window_);
    if (!ctx) {
        state_->LogSdlError("SDL_GL_CreateContext");
        return nullptr;
    }

    int vsyncValue = 0;
    if (options.vSyncMode == AdaptiveVSync) {
        vsyncValue = -1;
    } else if (options.vSyncMode == NormalVSync) {
        vsyncValue = 1;
    }

    int status = SDL_GL_SetSwapInterval(vsyncValue);
    if (status < 0 && vsyncValue == -1)
        status = SDL_GL_SetSwapInterval(1); // Retry with normal vsync.

    if (status < 0)
        state_->LogSdlError("SDL_GL_SetSwapInterval"); // Not a critical error, we still return context in this case.

    return std::make_unique<SdlOpenGLContext>(state_, window_, ctx);
}
