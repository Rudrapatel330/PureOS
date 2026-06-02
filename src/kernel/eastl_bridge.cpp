#include <stddef.h>
#include <stdint.h>

extern "C" {
    void* kmalloc(size_t size);
    void kfree(void* ptr);
    void kernel_printf(const char* format, ...);
}

// Standard C++ new/delete overloads
void* operator new(size_t size) { return kmalloc(size); }
void* operator new[](size_t size) { return kmalloc(size); }
void operator delete(void* p) noexcept { kfree(p); }
void operator delete[](void* p) noexcept { kfree(p); }
void operator delete(void* p, size_t) noexcept { kfree(p); }
void operator delete[](void* p, size_t) noexcept { kfree(p); }

// EASTL specific overrides
void* operator new[](size_t size, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) {
    return kmalloc(size);
}

void* operator new[](size_t size, size_t /*alignment*/, size_t /*alignmentOffset*/, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) {
    return kmalloc(size);
}

// EASTL expects an implementation of Vsnprintf8 for strings
int Vsnprintf8(char* pDestination, size_t n, const char* pFormat, __builtin_va_list arguments) {
    // Basic stub, real vsnprintf needed for full eastl::string::sprintf support
    (void)pDestination; (void)n; (void)pFormat; (void)arguments;
    return 0;
}
