#include <kernel.h>

// global kernel_state variable defined
KernelState _kernel_state;

int main() {
	// initialize the system
    bootstrap();

    // start the main kernel loop
    k_main();

    // clear up
    clear_up();

    // exit the program
    return 0;
}