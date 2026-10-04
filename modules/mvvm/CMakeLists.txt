find_package(Threads REQUIRED)

cpptoolkit_add_module(
    NAME mvvm
    SOURCES
        src/Dispatcher.cpp
        src/NotificationWorker.cpp
    PUBLIC_DEPS
        cpptoolkit::struct
        Threads::Threads
)
