#include "Utility.hpp"

#include <Cocoa/Cocoa.h>

namespace adh {
#if defined(ADH_DEBUG)
    bool ShowThrowDialog(const std::string& text) noexcept {
        std::cerr << text << std::endl;
        @autoreleasepool {
            NSAlert* alert        = [[NSAlert alloc] init];
            alert.messageText     = @"AdHoc error";
            alert.informativeText = [NSString stringWithUTF8String:text.c_str()];
            [alert addButtonWithTitle:@"Throw"];
            [alert addButtonWithTitle:@"Ignore"];
            return [alert runModal] == NSAlertSecondButtonReturn;
        }
    }
#endif
} // namespace adh
