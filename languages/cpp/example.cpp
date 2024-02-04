/// @file example.cpp
/// @copyright Copyright (C) Zero IT Lab - All Rights Reserved
/// @brief SDK usage example for Theia.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential
#include "theia_sdk.hpp"

#include <cstdio>

theia::SDKCallbackAction MyCallbackAssertFailed(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    printf("MyCallbackAssertFailed %08X\n", (uint32_t)Code);
    return theia::SDKCallbackAction::Continue;
}

theia::SDKCallbackAction MyCallbackAssert(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    printf("MyCallbackAssert %08X\n", (uint32_t)Code);
    return theia::SDKCallbackAction::Continue;
}

theia::SDKCallbackAction MyCallback(NTSTATUS Code, uintptr_t Param1, uintptr_t Param2, uintptr_t Param3) {
    printf("MyCallback %08X\n", (uint32_t)Code);
    return theia::SDKCallbackAction::Continue;
}

THEIA_REGISTER_CALLBACK(MyCallbackAssertFailed, theia::CrashCode::Assert::Failed);
THEIA_REGISTER_CALLBACK(MyCallbackAssert, theia::CrashCodeCategory::Assert);
THEIA_REGISTER_CALLBACK(MyCallback, theia::CrashCode::All);

THEIA_GUARD_PAGE();

THEIA_ONCE();

int main() {
    printf("%d\n", theia::GetInterface()->IsProtected());
    theia::GetInterface()->ReadyForCallbacks();
    return 0;
}
