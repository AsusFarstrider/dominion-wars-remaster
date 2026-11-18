#pragma once
// pushad savedESP points at pushfd; +4 gives original ESP at the hook site.
// No return substitution: replay trampoline decides normal control flow.
#define MENU_BRIDGE(symbol,index,offset,table,observer) \
__declspec(naked) void symbol(){ \
    __asm pushfd \
    __asm pushad \
    __asm mov eax,esp \
    __asm sub esp,528 \
    __asm and esp,0FFFFFFF0h \
    __asm fxsave [esp] \
    __asm mov [esp+512],eax \
    __asm fninit \
    __asm push eax \
    __asm push index \
    __asm call observer \
    __asm fxrstor [esp] \
    __asm mov esp,[esp+512] \
    __asm popad \
    __asm popfd \
    __asm jmp dword ptr [table+offset] \
}
