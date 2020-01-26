#include <kernel.h>

// global kernel_state variable defined
KernelState _kernel_state;

int main() {
	// initialize the system
    bootstrap();
    // initialize the user task

    // start the main kernel loop
    k_main();
    // exit the program
    return 0;
}