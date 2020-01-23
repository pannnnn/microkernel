#include <kernel.h>

KernelState _kernel_state;

int main() {
    bootstrap();
    k_main();
    return 0;
}