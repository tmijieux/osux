#ifndef COMPILER_H
#define COMPILER_H


// ---------- INITIALIZER:

// Initializer/finalizer sample for MSVC and GCC/Clang.
// 2010-2016 Joe Lowe. Released into the public domain.

#ifdef __cplusplus
#    define INITIALIZER(f)                                              \
    static void f(void);                                                \
    struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_;        \
    static void f(void)
#elif defined(_MSC_VER)
#    pragma section(".CRT$XCU",read)
#    define INITIALIZER2_(f,p)                                   \
    static void f(void);                                         \
    __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f;     \
    __pragma(comment(linker,"/include:" p #f "_"))               \
    static void f(void)
        
#    ifdef _WIN64
#        define INITIALIZER(f) INITIALIZER2_(f,"")
#    else
#        define INITIALIZER(f) INITIALIZER2_(f,"_")
#    endif
#else
#    define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

// INITIALIZER -----------------<


/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// VISIBILITY

#define __internal
#define __export

#ifdef _WIN32
#    unset __export
#    define __export  __declspec(ddlexport)
#endif

#ifdef __GNUC__
#  define UNUSED(x) x __attribute__((unused))
#else
#  define UNUSED(x) x
#endif // __GNUC__


#endif //COMPILER_H
