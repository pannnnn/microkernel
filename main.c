#include <k.h>

KernelState *_kernel_state;

int main() {
    bootstrap();
    k_main();
}