#pragma once
// An indirect JMP through a private stack slot, with no return substitution.
// Allocate 544 rather than 528 so the target at savedFrame-4 cannot overlap
// FXSAVE or its saved-frame metadata, even at the worst alignment.
#define CANVAS_PREDICATE_BRIDGE(symbol,decide) \
__declspec(naked) void symbol(){ \
    __asm pushfd \
    __asm pushad \
    __asm mov eax,esp \
    __asm sub esp,544 \
    __asm and esp,0FFFFFFF0h \
    __asm fxsave [esp] \
    __asm mov [esp+512],eax \
    __asm fninit \
    __asm push eax \
    __asm call decide \
    __asm mov ecx,[esp+512] \
    __asm mov [ecx-4],eax \
    __asm fxrstor [esp] \
    __asm mov esp,[esp+512] \
    __asm popad \
    __asm popfd \
    __asm jmp dword ptr [esp-40] \
}
