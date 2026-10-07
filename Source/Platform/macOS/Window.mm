#include <Cocoa/Cocoa.h>
#include <GameController/GameController.h>
#include <QuartzCore/CAMetalLayer.h>

#include <Event/Event.hpp>
#include <Input/Keycodes.hpp>
#include <Window.hpp>

#include <algorithm>
#include <cmath>
#include <iterator>

using namespace adh;

@interface AdHocApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AdHocApplicationDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    (void)sender;
    EventBus().publish<WindowEvent>(WindowEvent::Type::eCloseRequested);
    return NSTerminateCancel;
}
@end

@interface AdHocView : NSView <NSWindowDelegate, NSTextInputClient> {
  @public
    adh::Window* window;
  @private
    NSTrackingArea* trackingArea;
    NSMutableAttributedString* markedText;
    NSRange markedSelection;
    bool modifierKeys[128];
}
- (void)updateSize;
@end

@implementation AdHocView
- (instancetype)initWithFrame:(NSRect)frame {
    if ((self = [super initWithFrame:frame])) {
        self.wantsLayer = YES;
        self.layer      = [CAMetalLayer layer];
        markedText      = [[NSMutableAttributedString alloc] initWithString:@""];
        markedSelection = NSMakeRange(0, 0);
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)updateTrackingAreas {
    if (trackingArea) {
        [self removeTrackingArea:trackingArea];
    }
    trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                options:NSTrackingMouseMoved | NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect
                                                  owner:self
                                               userInfo:nil];
    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

- (void)updateSize {
    if (!window) {
        return;
    }
    const NSRect logical = self.bounds;
    const NSRect pixels  = [self convertRectToBacking:logical];
    CAMetalLayer* layer  = (CAMetalLayer*)self.layer;
    layer.contentsScale  = self.window.backingScaleFactor;
    layer.drawableSize   = pixels.size;
    window->OnResize(static_cast<int>(std::lround(logical.size.width)), static_cast<int>(std::lround(logical.size.height)),
                     static_cast<int>(std::lround(pixels.size.width)), static_cast<int>(std::lround(pixels.size.height)));
}

- (void)viewDidChangeBackingProperties {
    [super viewDidChangeBackingProperties];
    [self updateSize];
}

- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
    [self updateSize];
}

- (void)windowDidChangeBackingProperties:(NSNotification*)notification {
    (void)notification;
    [self updateSize];
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    (void)sender;
    EventBus().publish<WindowEvent>(WindowEvent::Type::eCloseRequested);
    return NO;
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    (void)notification;
    if (window) {
        window->OnFocus(true);
    }
}

- (void)windowDidResignKey:(NSNotification*)notification {
    (void)notification;
    std::fill(std::begin(modifierKeys), std::end(modifierKeys), false);
    [self unmarkText];
    if (window) {
        window->OnFocus(false);
    }
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
    (void)notification;
    if (window) {
        window->OnMinimized(true);
    }
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
    (void)notification;
    if (window) {
        window->OnMinimized(false);
        [self updateSize];
    }
}

- (void)keyDown:(NSEvent*)event {
    const bool composing = self.hasMarkedText;
    if (!(event.modifierFlags & NSEventModifierFlagCommand)) {
        [self interpretKeyEvents:@[ event ]];
    }
    if (!composing && !self.hasMarkedText) {
        EventBus().publish<KeyboardEvent>(event.isARepeat ? KeyboardEvent::Type::eKeyRepeat : KeyboardEvent::Type::eKeyDown,
                                          event.keyCode);
    }
}

- (void)keyUp:(NSEvent*)event {
    EventBus().publish<KeyboardEvent>(KeyboardEvent::Type::eKeyUp, event.keyCode);
}

- (BOOL)performKeyEquivalent:(NSEvent*)event {
    if (event.type == NSEventTypeKeyDown && (event.modifierFlags & NSEventModifierFlagCommand)) {
        if (event.keyCode == ADH_KEY_Q) {
            EventBus().publish<WindowEvent>(WindowEvent::Type::eCloseRequested);
            return YES;
        }
        [self keyDown:event];
        return YES;
    }
    return NO;
}

- (void)flagsChanged:(NSEvent*)event {
    const unsigned key = event.keyCode;
    NSEventModifierFlags flag{};
    switch (key) {
    case 0x38:
    case 0x3c:
        flag = NSEventModifierFlagShift;
        break;
    case 0x3b:
    case 0x3e:
        flag = NSEventModifierFlagControl;
        break;
    case 0x3a:
    case 0x3d:
        flag = NSEventModifierFlagOption;
        break;
    case 0x36:
    case 0x37:
        flag = NSEventModifierFlagCommand;
        break;
    case 0x39:
        flag = NSEventModifierFlagCapsLock;
        break;
    default:
        return;
    }
    const bool down   = (event.modifierFlags & flag) && !modifierKeys[key];
    modifierKeys[key] = down;
    EventBus().publish<KeyboardEvent>(down ? KeyboardEvent::Type::eKeyDown : KeyboardEvent::Type::eKeyUp, key);
}

- (NSPoint)getMouseLocalPoint:(NSEvent*)event {
    NSPoint point = [self convertPoint:event.locationInWindow fromView:nil];
    point.y       = self.bounds.size.height - point.y;
    return point;
}

- (void)mouseMoved:(NSEvent*)event {
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseMoveEvent>(p.x, p.y);
}

- (void)mouseEntered:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)mouseExited:(NSEvent*)event {
    (void)event;
    if (NSEvent.pressedMouseButtons == 0) {
        EventBus().publish<MouseMoveEvent>(-32768, -32768);
    }
}

- (void)mouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)otherMouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}

- (void)mouseDown:(NSEvent*)event {
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseButtonEvent>(MouseButtonEvent::Index::eLeftButton, MouseButtonEvent::Type::eLeftButtonDown, p.x, p.y);
}

- (void)mouseUp:(NSEvent*)event {
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseButtonEvent>(MouseButtonEvent::Index::eLeftButton, MouseButtonEvent::Type::eLeftButtonUp, p.x, p.y);
}

- (void)rightMouseDown:(NSEvent*)event {
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseButtonEvent>(MouseButtonEvent::Index::eRightButton, MouseButtonEvent::Type::eRightButtonDown, p.x, p.y);
}

- (void)rightMouseUp:(NSEvent*)event {
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseButtonEvent>(MouseButtonEvent::Index::eRightButton, MouseButtonEvent::Type::eRightButtonUp, p.x, p.y);
}

- (void)otherMouseDown:(NSEvent*)event {
    if (event.buttonNumber != 2) {
        return;
    }
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseButtonEvent>(MouseButtonEvent::Index::eMiddleButton, MouseButtonEvent::Type::eMiddleButtonDown, p.x, p.y);
}

- (void)otherMouseUp:(NSEvent*)event {
    if (event.buttonNumber != 2) {
        return;
    }
    const NSPoint p = [self getMouseLocalPoint:event];
    EventBus().publish<MouseButtonEvent>(MouseButtonEvent::Index::eMiddleButton, MouseButtonEvent::Type::eMiddleButtonUp, p.x, p.y);
}

- (void)scrollWheel:(NSEvent*)event {
    const NSPoint p   = [self getMouseLocalPoint:event];
    const float delta = event.scrollingDeltaY * (event.hasPreciseScrollingDeltas ? 0.1f : 1.0f);
    EventBus().publish<MouseWheelEvent>(delta, p.x, p.y);
}

- (void)insertText:(id)text replacementRange:(NSRange)replacementRange {
    (void)replacementRange;
    NSString* string = [text isKindOfClass:[NSAttributedString class]] ? [text string] : text;
    [self unmarkText];
    for (NSUInteger index = 0; index < string.length; ++index) {
        std::uint32_t character = [string characterAtIndex:index];
        if (character >= 0xd800 && character <= 0xdbff && index + 1 < string.length) {
            const std::uint32_t low = [string characterAtIndex:index + 1];
            if (low >= 0xdc00 && low <= 0xdfff) {
                character = 0x10000 + ((character - 0xd800) << 10) + low - 0xdc00;
                ++index;
            }
        }
        if (character >= 32 && character != 127 && !(character >= 0xd800 && character <= 0xdfff) &&
            !(character >= 0xf700 && character <= 0xf8ff)) {
            EventBus().publish<CharEvent>(character);
        }
    }
}

- (void)setMarkedText:(id)text selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
    (void)replacementRange;
    if ([text isKindOfClass:[NSAttributedString class]]) {
        [markedText setAttributedString:text];
    } else {
        [markedText.mutableString setString:text];
    }
    markedSelection = selectedRange;
}

- (void)unmarkText {
    [markedText.mutableString setString:@""];
    markedSelection = NSMakeRange(0, 0);
}

- (BOOL)hasMarkedText {
    return markedText.length != 0;
}

- (NSRange)markedRange {
    return self.hasMarkedText ? NSMakeRange(0, markedText.length) : NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange {
    return self.hasMarkedText ? markedSelection : NSMakeRange(NSNotFound, 0);
}

- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText {
    return @[];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    if (range.location == NSNotFound || range.location >= markedText.length) {
        return nil;
    }
    range.length = std::min(range.length, markedText.length - range.location);
    if (actualRange) {
        *actualRange = range;
    }
    return [markedText attributedSubstringFromRange:range];
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    (void)point;
    return NSNotFound;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    if (actualRange) {
        *actualRange = range;
    }
    const NSPoint mouse = self.window.mouseLocationOutsideOfEventStream;
    return [self.window convertRectToScreen:NSMakeRect(mouse.x, mouse.y, 1, 20)];
}

- (void)doCommandBySelector:(SEL)selector {
    (void)selector;
}
@end

namespace adh {
    static constexpr std::uint32_t maxControllers{ 4u };

    struct Window::Native {
        NSWindow* window;
        AdHocView* view;
        AdHocApplicationDelegate* delegate;
        GCController* controllers[maxControllers];
        bool buttons[maxControllers][16]{};
        bool cursorHidden{};

        void PollControllers(bool focused) {
            NSArray<GCController*>* connected = GCController.controllers;
            for (std::uint32_t slot = 0; slot < maxControllers; ++slot) {
                if (controllers[slot] && ![connected containsObject:controllers[slot]]) {
                    for (std::uint32_t button = 0; button < 16; ++button) {
                        if (buttons[slot][button]) {
                            EventBus().publish<ControllerEvent>(ControllerEvent::Type::eButtonUp, button, slot);
                            buttons[slot][button] = false;
                        }
                    }
                    controllers[slot] = nil;
                }
            }
            for (GCController* controller in connected) {
                if (!controller.extendedGamepad) {
                    continue;
                }
                bool known = false;
                for (GCController* existing : controllers) {
                    known |= existing == controller;
                }
                if (!known) {
                    for (auto& existing : controllers) {
                        if (!existing) {
                            existing = controller;
                            break;
                        }
                    }
                }
            }
            for (std::uint32_t slot = 0; slot < maxControllers; ++slot) {
                GCExtendedGamepad* gamepad = focused ? controllers[slot].extendedGamepad : nil;
                bool pressed[16]{};
                pressed[ADH_BUTTON_A]            = gamepad.buttonA.isPressed;
                pressed[ADH_BUTTON_B]            = gamepad.buttonB.isPressed;
                pressed[ADH_BUTTON_X]            = gamepad.buttonX.isPressed;
                pressed[ADH_BUTTON_Y]            = gamepad.buttonY.isPressed;
                pressed[ADH_BUTTON_DPAD_UP]      = gamepad.dpad.up.isPressed;
                pressed[ADH_BUTTON_DPAD_DOWN]    = gamepad.dpad.down.isPressed;
                pressed[ADH_BUTTON_DPAD_LEFT]    = gamepad.dpad.left.isPressed;
                pressed[ADH_BUTTON_DPAD_RIGHT]   = gamepad.dpad.right.isPressed;
                pressed[ADH_BUTTON_RSHOULDER]    = gamepad.rightShoulder.isPressed;
                pressed[ADH_BUTTON_LSHOULDER]    = gamepad.leftShoulder.isPressed;
                pressed[ADH_BUTTON_LTRIGGER]     = gamepad.leftTrigger.isPressed;
                pressed[ADH_BUTTON_RTRIGGER]     = gamepad.rightTrigger.isPressed;
                pressed[ADH_BUTTON_START]        = gamepad.buttonMenu.isPressed;
                pressed[ADH_BUTTON_BACK]         = gamepad.buttonOptions.isPressed;
                pressed[ADH_BUTTON_LTHUMB_PRESS] = gamepad.leftThumbstickButton.isPressed;
                pressed[ADH_BUTTON_RTHUMB_PRESS] = gamepad.rightThumbstickButton.isPressed;
                for (std::uint32_t button = 0; button < 16; ++button) {
                    if (pressed[button]) {
                        EventBus().publish<ControllerEvent>(buttons[slot][button] ? ControllerEvent::Type::eButtonRepeat
                                                                                  : ControllerEvent::Type::eButtonDown,
                                                            button, slot);
                    } else if (buttons[slot][button]) {
                        EventBus().publish<ControllerEvent>(ControllerEvent::Type::eButtonUp, button, slot);
                    }
                    buttons[slot][button] = pressed[button];
                }
            }
        }
    };

    Window::~Window() {
        Clear();
    }

    void Window::Create(const char* name, std::int32_t width, std::int32_t height, bool isPrepared, bool setFullscreen) {
        @autoreleasepool {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            m_Native            = new Native{};
            m_Native->delegate  = [AdHocApplicationDelegate new];
            NSApp.delegate      = m_Native->delegate;
            NSMenu* menu        = [NSMenu new];
            NSMenuItem* appItem = [NSMenuItem new];
            [menu addItem:appItem];
            NSMenu* appMenu = [NSMenu new];
            [appMenu addItemWithTitle:@"Quit AdHoc" action:@selector(terminate:) keyEquivalent:@"q"];
            appItem.submenu = appMenu;
            NSApp.mainMenu  = menu;

            const NSRect rect                        = NSMakeRect(0, 0, width, height);
            m_Native->window                         = [[NSWindow alloc] initWithContentRect:rect
                                                                                   styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                                                                     backing:NSBackingStoreBuffered
                                                                                       defer:NO];
            m_Native->window.releasedWhenClosed      = NO;
            m_Native->window.title                   = [NSString stringWithUTF8String:name];
            m_Native->window.acceptsMouseMovedEvents = YES;
            m_Native->view                           = [[AdHocView alloc] initWithFrame:rect];
            m_Native->view->window                   = this;
            m_Native->window.contentView             = m_Native->view;
            m_Native->window.delegate                = m_Native->view;
            m_IsOpen                                 = true;
            [m_Native->window center];
            [m_Native->window makeFirstResponder:m_Native->view];
            [m_Native->window makeKeyAndOrderFront:nil];
            [NSApp finishLaunching];
            [NSApp activateIgnoringOtherApps:YES];
            [m_Native->view updateSize];
            m_IsPrepared = isPrepared;
            if (setFullscreen) {
                [m_Native->window toggleFullScreen:nil];
            }
        }
    }

    void Window::Destroy() noexcept {
        Clear();
    }

    void* Window::GetHandle() const noexcept {
        return m_Native ? (__bridge void*)m_Native->view : nullptr;
    }

    void Window::PollEvents() noexcept {
        @autoreleasepool {
            while (NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                       untilDate:[NSDate distantPast]
                                                          inMode:NSDefaultRunLoopMode
                                                         dequeue:YES]) {
                [NSApp sendEvent:event];
            }
            [NSApp updateWindows];
            m_Native->PollControllers(m_IsInFocus);
        }
    }

    void Window::WaitEvents() noexcept {
        @autoreleasepool {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantFuture]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event) {
                [NSApp sendEvent:event];
            }
            PollEvents();
        }
    }

    void Window::SetTitle(const char* title) noexcept {
        m_Native->window.title = [NSString stringWithUTF8String:title];
    }

    void Window::Restore() noexcept {
        if (m_Native->window.miniaturized) {
            [m_Native->window deminiaturize:nil];
        }
    }

    std::string Window::GetClipboardText() {
        NSString* text = [NSPasteboard.generalPasteboard stringForType:NSPasteboardTypeString];
        return text ? std::string{ text.UTF8String } : std::string{};
    }

    void Window::SetClipboardText(const char* text) {
        [NSPasteboard.generalPasteboard clearContents];
        [NSPasteboard.generalPasteboard setString:[NSString stringWithUTF8String:text] forType:NSPasteboardTypeString];
    }

    void Window::SetCursor(Cursor cursor) noexcept {
        const bool hidden = cursor == Cursor::eHidden;
        if (hidden != m_Native->cursorHidden) {
            if (hidden) {
                [NSCursor hide];
            } else {
                [NSCursor unhide];
            }
            m_Native->cursorHidden = hidden;
        }
        if (hidden) {
            return;
        }
        NSCursor* native = NSCursor.arrowCursor;
        switch (cursor) {
        case Cursor::eTextInput:
            native = NSCursor.IBeamCursor;
            break;
        case Cursor::eResizeAll:
            native = NSCursor.closedHandCursor;
            break;
        case Cursor::eResizeNS:
            native = NSCursor.resizeUpDownCursor;
            break;
        case Cursor::eResizeEW:
            native = NSCursor.resizeLeftRightCursor;
            break;
        case Cursor::eResizeNESW:
        case Cursor::eResizeNWSE:
            native = NSCursor.crosshairCursor;
            break;
        case Cursor::eHand:
            native = NSCursor.pointingHandCursor;
            break;
        case Cursor::eNotAllowed:
            native = NSCursor.operationNotAllowedCursor;
            break;
        default:
            break;
        }
        [native set];
    }

    void Window::Clear() noexcept {
        if (!m_Native) {
            return;
        }
        @autoreleasepool {
            if (m_Native->cursorHidden) {
                [NSCursor unhide];
            }
            m_Native->view->window    = nullptr;
            m_Native->window.delegate = nil;
            [m_Native->window close];
            NSApp.delegate = nil;
            delete m_Native;
            m_Native = nullptr;
        }
    }
} // namespace adh
