#ifndef UPSKIRT_WIN32_H
#define UPSKIRT_WIN32_H

// Redefinitions

/* Windows platform with MS compiler */
#if defined(_WIN32) && defined(_MSC_VER)
    #define strncasecmp _strnicmp
    #define ssize_t int
    #define inline __inline
    #define snprintf _snprintf
    #define vsnprintf _vsnprintf
    #define va_copy(dst, src) ((void)((dst) = (src)))
    #define __attribute__(x)

    #pragma warning( disable : 4996 4100 4204 )
#endif

/* Windows platform with Borland compiler */
#if defined(_WIN32) && defined(__BORLANDC__)
    #define strncasecmp _strnicmp
    //#define ssize_t int
    #define inline __inline
    //#define snprintf _snprintf
    //#define vsnprintf _vsnprintf
    #define va_copy(dst, src) ((void)((dst) = (src)))
    #define __attribute__(x)
#endif

/* Windows platform with GNU compiler (Mingw) */
#if defined(_WIN32) && defined(__MINGW32__)
#endif

/* Cygwin platform, GNU compiler */
#if defined(_WIN32) && defined(__CYGWIN__)
#endif

#endif
