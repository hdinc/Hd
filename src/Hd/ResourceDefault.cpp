// weak fallbacks for executables that have no resources. these must not live in
// the same translation unit as the code that references them: on mingw gas
// leaks the fallback's offset inside .bss into the relocation addend, so the
// reference ends up pointing past the real symbol when it is overridden.

#ifdef __EMSCRIPTEN__
extern "C" {
__attribute__((weak)) extern const char exeres_hdres[] = {0};
__attribute__((weak)) extern const long exeres_hdres_size = 0;
}
#else
__attribute__((weak)) char _binary_exeres_hdres_start = 0;
__attribute__((weak)) char _binary_exeres_hdres_end   = 0;
#endif
