#include <App.h>
#include <app_conf.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define DELAY_TIME        10000u
#define STARTUP_DELAY     500u

pxl::App app;
/* --------------------------- */

void
setup()
{
    app.setup();

    return;
}

void
loop()
{

    app.loop();
    delay(10000u);
    
    return;
}
