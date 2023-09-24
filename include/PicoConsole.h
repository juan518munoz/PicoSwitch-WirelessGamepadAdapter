// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 Ricardo Quesada
// http://retro.moe/unijoysticle2

#ifndef BP32_CONSOLE_H
#define BP32_CONSOLE_H

#include <stddef.h>

// Pico includes
#include <string>

class Console {
   public:
    void print(const std::string& str);
    void print(const char* str);

    void println(const std::string& str);
    void println(const char* str);

    void printf(const char* fmt, ...);
};

extern Console Console;

#endif  // BP32_CONSOLE_H