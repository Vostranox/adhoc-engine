#pragma once
#include <iostream>
#include <cstdint>

namespace adh {
    struct WindowEvent {
        enum class Type : char {
            ePrepared,
            eResized,
            eMinimized,
            eDrop,
            eFocus,
            eKillfocus,
            eCloseRequested
        };

        WindowEvent(Type newType) : type{ newType } {
        }

        Type type;
    };

    struct WindowDropEvent : WindowEvent {
        WindowDropEvent(Type newType, const char* newFilePath) : WindowEvent{ newType },
                                                                 filePath{ newFilePath } {
        }

        const char* filePath;
    };

    struct SwapchainEvent {
        enum class Type : char {
            eRecreated
        };

        SwapchainEvent(Type newType, void* newImageViews, std::uint32_t newWidth, std::uint32_t newHeight) : type{ newType },
                                                                                                             imageViews{ newImageViews },
                                                                                                             width{ newWidth },
                                                                                                             height{ newHeight } {
        }

        Type type;
        void* imageViews;
        std::uint32_t width;
        std::uint32_t height;
    };

    struct KeyboardEvent {
        enum class Type : char {
            eInvalid,
            eKeyDown,
            eKeyUp,
            eKeyRepeat
        };

        KeyboardEvent(Type newType, std::uint64_t newKeycode) : type{ newType },
                                                                keycode{ newKeycode } {
        }

        Type type;
        std::uint64_t keycode;
    };

    struct CharEvent {
        CharEvent(std::uint32_t newKeycode) : keycode{ newKeycode } {
        }

        std::uint32_t keycode;
    };

    struct MouseMoveEvent {
        MouseMoveEvent(std::int16_t newX, std::int16_t newY) : x{ newX },
                                                               y{ newY } {
        }

        std::int16_t x;
        std::int16_t y;
    };

    struct MouseButtonEvent : MouseMoveEvent {
        enum class Type : char {
            eInvalid,
            eLeftButtonDown,
            eLeftButtonUp,
            eRightButtonDown,
            eRightButtonUp,
            eMiddleButtonDown,
            eMiddleButtonUp,
        };

        enum class Index : char {
            eLeftButton,
            eRightButton,
            eMiddleButton
        };

        MouseButtonEvent(Index newIndex, Type newType, std::int16_t newX, std::int16_t newY) : MouseMoveEvent{ newX, newY },
                                                                                               index{ newIndex },
                                                                                               type{ newType } {
        }

        Index index;
        Type type;
    };

    struct MouseWheelEvent : MouseMoveEvent {
        MouseWheelEvent(float newDelta, std::int16_t newX, std::int16_t newY) : MouseMoveEvent{ newX, newY },
                                                                                delta{ newDelta } {
        }

        float delta;
    };

    struct ControllerEvent {
        enum class Type : char {
            eInvalid,
            eButtonDown,
            eButtonUp,
            eButtonRepeat
        };

        ControllerEvent(Type newType, std::uint64_t newKeycode, std::uint32_t newId) : type{ newType },
                                                                                       keycode{ newKeycode },
                                                                                       id{ newId } {
        }

        Type type;
        std::uint64_t keycode;
        std::uint32_t id;
    };

    struct CollisionEvent {
        enum class Type {
            eCollisionInvalid,
            eCollisionEnter,
            eCollisionPersist,
            eCollisionExit,
            eTriggerEnter,
            eTriggerPersist,
            eTriggerExit
        };
        CollisionEvent(Type newType, std::uint64_t lhs, std::uint64_t rhs)
            : type{ newType },
              entityA{ lhs },
              entityB{ rhs } {
        }
        Type type;
        std::uint64_t entityA;
        std::uint64_t entityB;
    };

    struct CameraEvent {
        enum class Type : char {
            eHasUpdate
        };

        CameraEvent(Type newType) : type{ newType } {
        }
        Type type;
    };

    struct StatusEvent {
        enum class Type {
            eRun,
            eStop,
            ePause,
            eUnpause
        };

        StatusEvent(Type newType) : type{ newType } {
        }

        Type type;
    };

    struct EditorLogEvent {
        enum class Type {
            eLog,
            eError
        };

        EditorLogEvent(Type newType, const char* newMessage)
            : type{ newType },
              message{ newMessage } {
        }

        Type type;
        const char* message;
    };
} // namespace adh
