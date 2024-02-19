/***
 # Released under the MIT License
 
 Copyright (C) 2024 Jeff Platzer <jeff@platzers.us>

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 ***/

#pragma once

#define BEGIN_COND_LOOP for (;;) {
#define EXIT_COND_LOOP break;
#define IF_COND_EXIT_LOOP(cond) if (cond) break;
#define IF_COND_PROC_EXIT_LOOP(cond, proc) if (cond) { proc; break; }
#define IF_COND_ASSIGN_EXIT_LOOP(cond, assign_to) { assign_to = (cond); \
    if (assign_to) break; }
#define IF_COND_ASSIGN_PROC_EXIT_LOOP(cond, assign_to, proc) { assign_to = (cond); \
   if (assign_to) { proc; break; } }
#define CONT_COND_LOOP continue;
#define END_COND_LOOP break; }
#define REPEAT_COND_LOOP }
