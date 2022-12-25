
#pragma once

#if defined(BUILD_MONOLITHIC)

#ifdef __cplusplus
extern "C" {
#endif

extern int charter_svg_main(int argc, const char* argv[]);
extern int charter_tex_main(int argc, const char* argv[]);

extern int smartypants_main(int argc, const char* argv[]);
extern int upskirt_main(int argc, const char* argv[]);

#ifdef __cplusplus
}
#endif

#endif
