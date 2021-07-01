//
//  utils.c
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

#include "utils.h"

#ifdef _WIN32
#  include <windows.h>
#  include <sys/timeb.h>
#  include <Processthreadsapi.h> // GetCurrentThreadId
#  include <libloaderapi.h>
#  include <processenv.h> // SetEnvironmentVariableA
#else
#  include <time.h>
#  include <sys/time.h>
#  include <stdlib.h> // setenv
#  include <dlfcn.h> // dladdr
#  if defined(__APPLE__) || defined(__unix__)
#    include <pthread.h>
#  elif defined(__linux__)
#    include <sys/types.h>
#  endif
#endif
#include <string.h> // strdup/_strdup

uint64_t getThreadID()
{
    uint64_t tid = 0;
#if defined(__APPLE__)
    pthread_threadid_np(NULL, &tid);
#elif defined(_WIN32)
    tid = (uint64_t)GetCurrentThreadId(); // Cast from DWORD.
#elif defined(__linux__)
    tid = (uint64_t)gettid(); // Cast from pid_t.
#elif defined(__unix__) // Other BSD not elsewhere defined.
    tid = (uint64_t)pthread_getthreadid_np(); // Cast from int.
#endif
    return tid;
}

utilTime getTimeNow(void)
{
    utilTime timeNow;
#ifdef _WIN32
    struct _timeb sys_time;

    _ftime_s(&sys_time);
    timeNow.secs = (long)sys_time.time;
    timeNow.millisecs = sys_time.millitm;
#else
    struct timeval time;

#  if defined(__linux) || defined(__APPLE__) || defined(EMSCRIPTEN)
    gettimeofday(&time, NULL);
#  else
    gettimeofday(&time);
#  endif
    timeNow.secs = time.tv_sec;
    timeNow.millisecs = time.tv_usec/1000;
#endif

    return timeNow;
}

unsigned long millisecondsElapsedSince(utilTime time)
{
    utilTime timeNow = getTimeNow();
    return ((timeNow.secs - time.secs)*1000 + (timeNow.millisecs - time.millisecs)); // The second addend can be negative.
}

char *getModulePath(void)
{
#ifdef _WIN32
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)getModulePath, &hm) == 0) {
        //int ret = GetLastError();
        //fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
        return NULL;
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
        //int ret = GetLastError();
        //fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
        return NULL;
    }
    return (_strdup(path));
#else
    Dl_info info;
    if (dladdr(getModulePath, &info) == 0) {
        return NULL;
    }
    return (strdup(info.dli_fname));
#endif
}

char *getDirectoryNameFromPath(char *dir, const char *path, const size_t n, const int addSeparator)
{
    char *sep;
#ifdef _WIN32
    char *sep1;
#endif
    size_t toCopy;

    if (!dir || !path || !n) return (NULL);

    sep = strrchr(path, '/');
#ifdef _WIN32
    sep1 = strrchr(path, '\\');
    if (sep1 > sep) sep = sep1;
#endif

    if (!sep) dir[0] = '\0';
    else {
        toCopy = sep + (addSeparator ? 1 : 0) - path;
        if (toCopy + 1 > n) return (NULL); // +1 because we need space for null-terminator.
        strncpy(dir, path, toCopy); // strlen(path) >= toCopy, so won't ever be null-terminated.
        dir[toCopy] = '\0';
    }
    return dir;
}

int setEnvVar(const char* name, const char* val)
{
    if (!name) return (-1);

#if defined(_WIN32)
    if (SetEnvironmentVariableA(name, val)) {
        return (0);
    }
#else
    return (setenv(name, val, 1));
#endif
    return (-1);
}
