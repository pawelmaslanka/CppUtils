# defer

## How to use?

### Cancel a deferred function or mark it as already called
```cpp
void foo() {
    DEFER_SCOPE; // Anchor the queue to the function scope

    std::cout << "Starting function..." << std::endl;

    auto cleanupHandle = DEFER_TASK([]() {
        std::cout << "-> Final fallback cleanup running!" << std::endl;
    });

    {
        std::cout << "Entering nested block..." << std::endl;
        
        auto earlyTask = DEFER_TASK([]() {
            std::cout << "-> Block-created task running!" << std::endl;
        });

        // We invoke this one early. It runs NOW, and won't run later.
        earlyTask();
        
        std::cout << "Exiting nested block..." << std::endl;
    } 
    // Notice: earlyTask goes out of scope here, but its logic 
    // was tied to the function-level queue.

    // Let's say the function succeeded, so we cancel the fallback cleanup.
    cleanupHandle.cancel();

    std::cout << "Ending function..." << std::endl;
} // <--- Active, non-cancelled tasks in the queue execute here.
```

### Passing an argument when calling a deferred function
```cpp
void closeConnection(int connectionId, const std::string& reason) {
    std::cout << "Closing connection " << connectionId << " Reason: " << reason << std::endl;
}

void processNetwork() {
    DEFER_SCOPE;
    int activeConn = 404;

    // The arguments 'activeConn' and the string are copied right here.
    auto cleanup = DEFER_TASK(closeConnection, activeConn, "Routine shutdown");
    
    activeConn = 999; // Changing this later does NOT affect the deferred task.
} 
// Output: Closing connection 404 Reason: Routine shutdown
```

### Using a lambda to capture state
```cpp
void processData() {
    DEFER_SCOPE;
    int recordsProcessed = 0;

    // Capturing by reference so it sees the final count
    DEFER_TASK([&]() {
        std::cout << "Final tally: " << recordsProcessed << " records processed." << std::endl;
    });

    recordsProcessed += 10;
    recordsProcessed += 5;
}
// Output: Final tally: 15 records processed.
```

### Early invocation with pre-bound arguments
```cpp
void handleFile() {
    DEFER_SCOPE;

    auto fileCloseTask = DEFER_TASK([](const char* name) {
        std::cout << "Closing file: " << name << std::endl;
    }, "config.json");

    std::cout << "Reading file..." << std::endl;

    // Trigger it early! We don't pass arguments here, because 
    // "config.json" was already bound to the task.
    fileCloseTask(); 
}
```