#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "krnlfont.h"
#include "kernel_colors.h"
#include "logo.h"
 
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

static void plotPixels (int y, int x, uint32_t pixel, struct limine_framebuffer *fb) {
    volatile uint32_t *fb_ptr = fb->address;

    fb_ptr[x * (fb->pitch / 4) + y] = pixel;
}

int cursor_x;
int cursor_y;

void putchar (unsigned short int c, uint32_t fg, uint32_t bg, struct limine_framebuffer *fb) {
    uint32_t pixel;
    
    for (int cy = 0; cy < 16; cy++) {
        for (int cx = 0; cx < 8; cx++) {
            pixel = (kernel_font[(c * 16) + cy] >> (7 - cx)) & 1 ? fg : bg;

            // Offset the cx and cy by x and y so that x and y are in characters instead of pixels
            plotPixels((cx + (cursor_x * 8)), (cy + (cursor_y * 16)), pixel, fb);
        }
    }

}

void print(char* string, struct limine_framebuffer *fb) {

    for (int i = 0;;i++) {

        // Check if the character is NULL, if it is, we've hit the end of the string
        if (string[i] == 0x00) {
            break;
        }

        // Check if it's '\n' if it is. move the cursor down.
        if (string[i] == 0x0A) {
            cursor_y++;
            cursor_x = 0;
            continue;
        }

        // Check if it's '\t' if it is, move the cursor foward by 6 spaces
        if (string[i] == 0x09) {
            cursor_x += 6;
            continue;
        }

        putchar(string[i], KRNL_WHITE, KRNL_BLACK, fb);

        cursor_x++;
    }
}

void display_logo(int line, struct limine_framebuffer *fb) {
    print(kernel_logo_line1, fb);
    line++;
    print(kernel_logo_line2, fb);
    line++;
    print(kernel_logo_line3, fb);
    line++;
    print(kernel_logo_line4, fb);
    line++;
    print(kernel_logo_line5, fb);
    line++;
    print(kernel_logo_line6, fb);
    line++;
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

    cursor_x = 0;
    cursor_y = 0;
    int c = 0;

    display_logo(0, framebuffer);

    cursor_y++;

    print("\tCopyright BlueSillyDragon (c)2023-2024\n\n\n", framebuffer);

    print("AquaOS kernel started successfully!\n", framebuffer);
    print("Printing font to screen...\n", framebuffer);

    cursor_y++;

    do {
        putchar(c, KRNL_WHITE, KRNL_BLACK, framebuffer);
        
        cursor_x++;
        c++;

        if (cursor_x > 64) {
            cursor_y++;
            cursor_x = 0;
        }

    } while (c < 256);
 
    // We're done, just hang...
    hcf();
}