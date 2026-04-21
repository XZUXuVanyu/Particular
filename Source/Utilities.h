#pragma once
#if DEBUG
/* If condition is true, tirgger breakpoint or return */
#define CRYSTAL_CHECK(condition, message, return_val) \
        do { \
            if (condition) { \
                DBG("[CRYSTAL-ERROR]: " << message); \
                jassertfalse; \
                return return_val; \
            } \
        } while (0)
#else
#define CRYSTAL_CHECK_RETURN(condition, message) \
        do { \
            if (condition) { \
                juce::Logger::writeToLog(juce::String("[CRYSTAL-ERROR]: ") + message); \
                return return_val; \
            } \
        } while (0)
#endif