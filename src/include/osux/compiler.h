#ifndef OSUX_COMPILER_H
#define OSUX_COMPILER_H

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

#ifdef __GNUC__

#define UNUSED __attribute__((unused))
#define MUST_CHECK __attribute__((warn_unused_result))
#define likely(x) (__builtin_constant_p(x) ? (x) : __builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_constant_p(x) ? (x) : __builtin_expect(!!(x), 0))
#else

#define UNUSED
#define MUST_CHECK
#define likely(x) (x)
#define unlikely(x) (x)
#endif // __GNUC__

#define MARK_USED(x) ((void) (x))

#endif // OUSX_COMPILER_H
