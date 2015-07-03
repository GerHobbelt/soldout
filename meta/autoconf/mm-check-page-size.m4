dnl mm-check-page-size.m4

AC_DEFUN([MM_CHECK_PAGESIZE],
  [AC_CACHE_CHECK([page size],
     [MM_cv_pagesize],
     [AC_COMPUTE_INT([MM_cv_pagesize],[sysconf(_SC_PAGESIZE)],[
       #include <unistd.h>
     ],[MM_cv_pagesize=4096])])])

dnl end of file
