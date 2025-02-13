#pragma once

#include <vector>
#include <string_view>
#include <memory>

#include "Platform/PlatformEvents.h"

struct EventTraceHeader {
    int saveFileSize = -1;
    // TODO(captainurist): std::string saveFileChecksum;
};

struct EventTrace {
    static void saveToFile(std::string_view path, const EventTrace &trace);
    static EventTrace loadFromFile(std::string_view path, PlatformWindow *window);

    static bool isTraceable(const PlatformEvent *event);
    static std::unique_ptr<PlatformEvent> cloneEvent(const PlatformEvent *event);

    EventTraceHeader header;
    std::vector<std::unique_ptr<PlatformEvent>> events;
};
