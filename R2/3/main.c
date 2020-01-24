#include "kernel.h"
#include "tasks.h"
#include "fifo.h"

int main(void){

    basicConfig();

    initializeFifos();

    registerTask(switchLEDGreen, HIGH, 10);
    registerTask(switchLEDRed, LOW, 10);

    startRTOS();

}
