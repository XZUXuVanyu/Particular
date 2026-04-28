#pragma once
#if DEBUG
/* If condition is true, tirgger breakpoint or return */
#define CRYSTAL_CHECK(condition, message, ...) \
        do { \
            if (condition) { \
                DBG("[CRYSTAL-ERROR]: " << message); \
                jassertfalse; \
                return __VA_ARGS__; \
            } \
        } while (0)
#else
#define CRYSTAL_CHECK(condition, message, ...) \
        do { \
            if (condition) { \
                juce::Logger::writeToLog("[CRYSTAL-ERROR]: " << message); \
                return __VA_ARGS__; \
            } \
        } while (0)
#endif