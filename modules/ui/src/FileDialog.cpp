#include "cpptoolkit/ui/FileDialog.h"

#include <cpptoolkit/platform/Platform.h>
#include <nfd.h>

// nfd_glfw3.h only converts windows of the native systems exposed here; without
// these definitions it leaves the parent handle unset. The X11/Wayland macros are
// set by CMake when the corresponding development headers were found.
#if CPPTOOLKIT_OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif CPPTOOLKIT_OS_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#elif CPPTOOLKIT_OS_LINUX
#if defined(CPPTOOLKIT_UI_NATIVE_X11)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#if defined(CPPTOOLKIT_UI_NATIVE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#endif
#include <nfd_glfw3.h>

#include <atomic>
#include <stdexcept>
#include <thread>
#include <utility>

namespace cpptoolkit::ui {
namespace {

enum class Kind { OpenFile, OpenFiles, SaveFile, PickFolder };

// Everything a dialog needs, owned so it can outlive the caller on a worker thread.
struct Request {
    Kind kind = Kind::OpenFile;
    std::vector<FileFilter> filters;
    std::string defaultPath;
    std::string defaultName;
    nfdwindowhandle_t parent{};
};

// Keeps NFD initialized on the current thread for the duration of one dialog.
class NfdSession {
public:
    NfdSession() {
        if (NFD_Init() != NFD_OKAY) throw std::runtime_error(std::string("File dialog init failed: ") + NFD_GetError());
        // Lets NFD talk to the compositor when GLFW runs on Wayland; a no-op elsewhere.
        NFD_SetDisplayPropertiesFromGLFW();
    }
    ~NfdSession() { NFD_Quit(); }
    NfdSession(const NfdSession&) = delete;
    NfdSession& operator=(const NfdSession&) = delete;
};

// glfwGetCurrentContext() is thread-local, so the parent must be captured on the render thread.
nfdwindowhandle_t CaptureParent() {
    nfdwindowhandle_t parent{};
    // The helper leaves the handle unset when the windowing system is unsupported.
    if (GLFWwindow* window = glfwGetCurrentContext()) NFD_GetNativeWindowFromGLFWWindow(window, &parent);
    return parent;
}

const char* OrNull(const std::string& value) { return value.empty() ? nullptr : value.c_str(); }

std::string TakePath(nfdu8char_t*& path) {
    std::string result(path);
    NFD_FreePathU8(path);
    path = nullptr;
    return result;
}

[[noreturn]] void Fail(const char* what) {
    throw std::runtime_error(std::string(what) + ": " + NFD_GetError());
}

// Blocks the calling thread; an empty result means the user cancelled.
std::vector<std::string> RunDialog(const Request& request) {
    NfdSession session;
    std::vector<nfdu8filteritem_t> filters;
    filters.reserve(request.filters.size());
    for (const auto& filter : request.filters) filters.push_back({filter.name.c_str(), filter.extensions.c_str()});
    const auto filterCount = static_cast<nfdfiltersize_t>(filters.size());

    nfdu8char_t* path = nullptr;
    nfdresult_t result = NFD_ERROR;
    switch (request.kind) {
    case Kind::OpenFile: {
        nfdopendialogu8args_t args{};
        args.filterList = filters.data();
        args.filterCount = filterCount;
        args.defaultPath = OrNull(request.defaultPath);
        args.parentWindow = request.parent;
        result = NFD_OpenDialogU8_With(&path, &args);
        if (result != NFD_OKAY && result != NFD_CANCEL) Fail("Open file dialog failed");
        break;
    }
    case Kind::OpenFiles: {
        nfdopendialogu8args_t args{};
        args.filterList = filters.data();
        args.filterCount = filterCount;
        args.defaultPath = OrNull(request.defaultPath);
        args.parentWindow = request.parent;
        const nfdpathset_t* paths = nullptr;
        result = NFD_OpenDialogMultipleU8_With(&paths, &args);
        if (result == NFD_CANCEL) return {};
        if (result != NFD_OKAY) Fail("Open files dialog failed");
        std::vector<std::string> selected;
        nfdpathsetsize_t count = 0;
        if (NFD_PathSet_GetCount(paths, &count) == NFD_OKAY) {
            for (nfdpathsetsize_t i = 0; i < count; ++i) {
                nfdu8char_t* item = nullptr;
                if (NFD_PathSet_GetPathU8(paths, i, &item) == NFD_OKAY) selected.push_back(TakePath(item));
            }
        }
        NFD_PathSet_Free(paths);
        return selected;
    }
    case Kind::SaveFile: {
        nfdsavedialogu8args_t args{};
        args.filterList = filters.data();
        args.filterCount = filterCount;
        args.defaultPath = OrNull(request.defaultPath);
        args.defaultName = OrNull(request.defaultName);
        args.parentWindow = request.parent;
        result = NFD_SaveDialogU8_With(&path, &args);
        if (result != NFD_OKAY && result != NFD_CANCEL) Fail("Save file dialog failed");
        break;
    }
    case Kind::PickFolder: {
        nfdpickfolderu8args_t args{};
        args.defaultPath = OrNull(request.defaultPath);
        args.parentWindow = request.parent;
        result = NFD_PickFolderU8_With(&path, &args);
        if (result != NFD_OKAY && result != NFD_CANCEL) Fail("Pick folder dialog failed");
        break;
    }
    }
    if (result == NFD_CANCEL) return {};
    return {TakePath(path)};
}

std::optional<std::string> First(std::vector<std::string> paths) {
    if (paths.empty()) return std::nullopt;
    return std::move(paths.front());
}

Request MakeRequest(Kind kind, const std::vector<FileFilter>& filters, const std::string& defaultPath,
                    const std::string& defaultName = {}) {
    return Request{kind, filters, defaultPath, defaultName, CaptureParent()};
}

} // namespace

struct PendingFileDialog::State {
    std::atomic<bool> ready{false};
    FileDialogResult result;
};

PendingFileDialog::PendingFileDialog() = default;
PendingFileDialog::PendingFileDialog(std::shared_ptr<State> state) : state_(std::move(state)) {}
PendingFileDialog::~PendingFileDialog() = default;
PendingFileDialog::PendingFileDialog(PendingFileDialog&&) noexcept = default;
PendingFileDialog& PendingFileDialog::operator=(PendingFileDialog&&) noexcept = default;
bool PendingFileDialog::Valid() const { return state_ != nullptr; }

bool PendingFileDialog::IsReady() const { return state_ && state_->ready.load(std::memory_order_acquire); }

FileDialogResult PendingFileDialog::Take() {
    if (!IsReady()) throw std::logic_error("File dialog result is not ready");
    FileDialogResult result = std::move(state_->result);
    state_.reset();
    return result;
}

namespace {

std::shared_ptr<PendingFileDialog::State> StartAsync(Request request) {
    auto state = std::make_shared<PendingFileDialog::State>();
    auto run = [state, request = std::move(request)] {
        try {
            state->result.paths = RunDialog(request);
        } catch (const std::exception& error) {
            state->result.error = error.what();
        }
        state->ready.store(true, std::memory_order_release);
    };
#if CPPTOOLKIT_OS_MACOS
    // Cocoa dialogs must run on the main thread, so macOS completes synchronously.
    run();
#else
    // Detached: the worker only touches the shared state, so an abandoned request is harmless.
    std::thread(std::move(run)).detach();
#endif
    return state;
}

} // namespace

std::optional<std::string> FileDialog::OpenFile(const std::vector<FileFilter>& filters, const std::string& defaultPath) {
    return First(RunDialog(MakeRequest(Kind::OpenFile, filters, defaultPath)));
}

std::vector<std::string> FileDialog::OpenFiles(const std::vector<FileFilter>& filters, const std::string& defaultPath) {
    return RunDialog(MakeRequest(Kind::OpenFiles, filters, defaultPath));
}

std::optional<std::string> FileDialog::SaveFile(const std::vector<FileFilter>& filters, const std::string& defaultPath,
                                                const std::string& defaultName) {
    return First(RunDialog(MakeRequest(Kind::SaveFile, filters, defaultPath, defaultName)));
}

std::optional<std::string> FileDialog::PickFolder(const std::string& defaultPath) {
    return First(RunDialog(MakeRequest(Kind::PickFolder, {}, defaultPath)));
}

PendingFileDialog FileDialog::OpenFileAsync(const std::vector<FileFilter>& filters, const std::string& defaultPath) {
    return PendingFileDialog(StartAsync(MakeRequest(Kind::OpenFile, filters, defaultPath)));
}

PendingFileDialog FileDialog::OpenFilesAsync(const std::vector<FileFilter>& filters, const std::string& defaultPath) {
    return PendingFileDialog(StartAsync(MakeRequest(Kind::OpenFiles, filters, defaultPath)));
}

PendingFileDialog FileDialog::SaveFileAsync(const std::vector<FileFilter>& filters, const std::string& defaultPath,
                                            const std::string& defaultName) {
    return PendingFileDialog(StartAsync(MakeRequest(Kind::SaveFile, filters, defaultPath, defaultName)));
}

PendingFileDialog FileDialog::PickFolderAsync(const std::string& defaultPath) {
    return PendingFileDialog(StartAsync(MakeRequest(Kind::PickFolder, {}, defaultPath)));
}

} // namespace cpptoolkit::ui
