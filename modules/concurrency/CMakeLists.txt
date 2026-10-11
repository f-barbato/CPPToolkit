find_package(Threads REQUIRED)

cpptoolkit_add_module(
    NAME concurrency
    SOURCES
        src/CancellationToken.cpp
        src/Dispatcher.cpp
        src/NotificationWorker.cpp
    PUBLIC_DEPS
        Threads::Threads
)
