#ifndef _epub_compat_hpp_
#define _epub_compat_hpp_

#if __cpp_char8_t < 201811L
#define char8_t char
#define u8string string
#endif

#endif
