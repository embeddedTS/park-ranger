#include <stdio.h>
#include <cairo/cairo.h>
#include <math.h>
#include <fcntl.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdlib.h>

#include "lcd-display.h"
#include "gpiolib.h"

static lcdInfo_t *lcd;

// Define the stop distance in ft
#define STOP_DIST 6 

// Define LED value status
#define ON 1
#define OFF 0

float read_port(void)
{
    int fd = 0; /* File descriptor for the port */
    char buffer[6];
    int rangeMm = 0;
    float rangeIn = 0;
    float rangeFt = 0;

    fd = open("/dev/ttymxc6", O_RDONLY | O_NOCTTY);
    if (fd == -1) {
        perror("open_port: Unable to open /dev/ttymxc6 - ");
        return(-1);
    }

    int n = read(fd, buffer, sizeof(buffer));
    close(fd);

    if (n < 0) {
        fputs("read failed!\n", stderr);
        return(-1);
    }

    // Remove the initial "R" at the biginning
    memmove(buffer, buffer + 1, sizeof(buffer + 1) + 1);

    // Remove the newline character at the end of the string
    buffer[strcspn(buffer, "\n")] = 0;

    rangeMm = atoi(buffer);
    rangeIn = rangeMm / 25.4;
    rangeFt = rangeIn / 12;

    printf("%d mm, %.1f in, %.1f ft\n", rangeMm, rangeIn, rangeFt);

    memset(buffer, 0, sizeof(buffer));

    return(rangeFt);
}

int main(void)
{
    cairo_t *cr;
    cairo_surface_t *sfc;
    char output[16];
    char *instruction;
    int green_led = 117;
    int yellow_led = 118;
    int red_led = 119;
    
    if ((lcd = openDisplay()) == NULL) {
        fprintf(stderr, "ERROR: Can't open display\n");
        return 1;
    }

    gpio_export(green_led);
    gpio_export(yellow_led);
    gpio_export(red_led);

    gpio_direction(green_led, 1);
    gpio_direction(yellow_led, 1);
    gpio_direction(red_led, 1);

    sfc = cairo_image_surface_create_for_data((unsigned char *)lcd->frameBuffer, 
        CAIRO_FORMAT_A1, lcd->displayWidth, lcd->displayHeight, lcd->stride);
         
    if (sfc) {                        
        if ((cr = cairo_create(sfc))) {
            cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
            cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                CAIRO_FONT_WEIGHT_BOLD);

            float range = 0;
            float current_range = 0;

            while(1) {
                range = read_port();
                if (range != current_range) {

                    // Clear the screen first
                    cairo_save (cr);
                    cairo_set_source_rgba (cr, 0, 0, 0, 0);
                    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
                    cairo_paint (cr);
                    cairo_restore (cr);

                    cairo_set_font_size(cr, 16.0);
                    cairo_move_to(cr, lcd->displayWidth - 60, lcd->displayHeight - 5);
                    snprintf(output, sizeof(output), "%.1f ft", range);
                    cairo_show_text(cr, output);

                    if (range <= STOP_DIST) {
                        instruction = "STOP!"; 

                        gpio_write(green_led, OFF);
                        gpio_write(yellow_led, OFF);
                        gpio_write(red_led, ON);
                    }
                    else if (range > STOP_DIST && range <= STOP_DIST + 1) {
                        instruction = "SLOW"; 
                        gpio_write(green_led, OFF);
                        gpio_write(yellow_led, ON);
                        gpio_write(red_led, OFF);
                    }
                    else  {
                        instruction = "GO";
                        gpio_write(green_led, ON);
                        gpio_write(yellow_led, OFF);
                        gpio_write(red_led, OFF);
                    }

                    cairo_set_font_size(cr, 32.0);
                    cairo_move_to(cr, 0, 32);
                    cairo_show_text(cr, instruction);

                    cairo_surface_flush(sfc);

                    current_range = range;
                }

                sleep(.1);
            }
        }
    }

    //TODO: Capture SIGINT and close everything out gracefully
    return 0;
}
