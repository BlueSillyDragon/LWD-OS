#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "font.h"
#include "kernel_colors.h"
 
// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
 
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);
 
// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
 
__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};
 
// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.
 
__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;
 
__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;
 
// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.
 
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
 
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
 
    return dest;
}
 
void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
 
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
 
    return s;
}
 
void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
 
    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
 
    return dest;
}
 
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
 
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
 
    return 0;
}
 
// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

void check_for_fb() {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
}

static void plotPixels (int x, int y, uint32_t pixel, struct limine_framebuffer *fb) {
    volatile uint32_t *fb_ptr = fb->address;

    fb_ptr[x * (fb->pitch / 4) + y] = pixel;
}

/*extern char _binary_zap_ext_light16_psf_start[];

#define PIXEL uint32_t

static void putchar(unsigned short int c, int cx, int cy, uint32_t fg, uint32_t bg, struct limine_framebuffer *fb) {
    psf_font *font = (psf_font*)&_binary_zap_ext_light16_psf_start;
    int bytesperline = (font->width+7)/8;

    unsigned char *glyph = (unsigned char*)&_binary_zap_ext_light16_psf_start + font->headersize + (c>0&&c<font->numglyph?c:0);

    int offs = (cy * font->height * fb->pitch) + (cx * (font->width + 1) * sizeof(PIXEL));

    int x,y, line,mask;

    for (y = 0; y < font->height; y++) {
        line = offs;
        mask = 1<<(font->width - 1);

        for (x = 0; x < font->width; x++) {
            *((PIXEL*) (fb + line)) = *((unsigned int*)glyph) & mask ? fg : bg;
            
            mask >>= 1;
            line += sizeof(PIXEL);
        }

        glyph += bytesperline;
        offs += fb->pitch;
    }
}*/

extern char _binary_zap_ext_light16_psf_start[];

void putchar (unsigned short int c, int x, int y, uint32_t fg, uint32_t bg, struct limine_framebuffer *fb) {
    int cx, cy;

    cy = 0;
    cx = 0;
    
    psf_font *kernel_font = (psf_font*)&_binary_zap_ext_light16_psf_start;

    uint8_t *first_gylph = (uint8_t*)&_binary_zap_ext_light16_psf_start + kernel_font->headersize;

    // A glyph is 8x16, cy is the y of a glyph
    uint32_t pixel = (first_gylph[c + cy] >> (7 - cx)) & 1 ? fg : bg;
    plotPixels(x, y, pixel, fb);

}
 
// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }
 
    // Ensure we got a framebuffer.
    check_for_fb();
 
    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
 
    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    //plotPixels(10, 10, KRNL_WHITE, framebuffer);

    putchar(20, 1, 1, KRNL_WHITE, KRNL_BLACK, framebuffer);
 
    // We're done, just hang...
    hcf();
}