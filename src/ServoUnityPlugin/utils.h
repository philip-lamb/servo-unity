//
//  utils.h
//  servo_unity
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0.If a copy of the MPL was not distributed with this
// file, You can obtain one at https ://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla, Inc.
//
// Author(s): Philip Lamb
//
// Plugin utility functions
//

#ifndef utils_h
#define utils_h

#include <stdint.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Can be printed e.g. with printf("thread ID is %" PRIu64 "\n");
extern uint64_t getThreadID(void);

typedef struct {
    long secs;
    int  millisecs;
} utilTime;

extern utilTime getTimeNow(void);

extern unsigned long millisecondsElapsedSince(utilTime time);

/** Get the full pathname of the code module in which this function exists.
    Typically either an executable or a dynamic library path.
    The returned value is malloc()ed internally and must be free()d by the caller.
    Returns NULL in case of error.
 */
extern char *getModulePath(void);

/** Get the directory portion of a full pathname.
    Given a full or partial pathname passed in string path,
    returns a string with the directory name portion of path.
    The string is placed into the buffer of size n pointed to by dir.
    If the string and its terminating null byte cannot be accomodated by the
    buffer, NULL is returned, otherwise dir is returned.
    The string is terminated by the directory separator if addSeparator != 0.
*/
extern char *getDirectoryNameFromPath(char *dir, const char *path, const size_t n, const int addSeparator);

extern int setEnvVar(const char* name, const char* val);

#ifdef __cplusplus
}
#endif
#endif // !utils_h
