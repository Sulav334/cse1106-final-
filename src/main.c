#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "temperature_log.txt"
#define GPIO_PIN 17


void control_led(int state) {
    char cmd[128];
    
    system("if [ ! -d /sys/class/gpio/gpio17 ]; then echo 17 > /sys/class/gpio/export; fi");
    system("echo out > /sys/class/gpio/gpio17/direction");
    
    sprintf(cmd, "echo %d > /sys/class/gpio/gpio17/value", state);
    system(cmd);
}

int main(void) {
    int choice;
    double min_temp = 68.0;
    double max_temp = 76.0;
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return 1;
    }
    fprintf(file, "Temperature (F)\tStatus\n");
    fclose(file);

    printf("=== Smart Classroom Comfort Monitor ===\n");
    printf("Default Acceptable Range: %.1f°F - %.1f°F\n", min_temp, max_temp);

    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. Enter classroom temperature readings (batch mode)\n");
        printf("2. Change acceptable temperature range\n");
        printf("3. Exit program\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while(getchar() != '\n'); // clear buffer
            continue;
        }

        if (choice == 1) {
            int num_inputs;
            printf("How many temperature readings would you like to enter? ");
            if (scanf("%d", &num_inputs) != 1 || num_inputs <= 0) {
                printf("Invalid number of inputs. Please enter a positive integer.\n");
                while(getchar() != '\n');
                continue;
            }

            double temp_sum = 0.0;
            int valid_count = 0;
            for (int i = 0; i < num_inputs; i++) {
                double temp;
                printf("Enter temperature reading %d of %d (°F): ", i + 1, num_inputs);
                if (scanf("%lf", &temp) != 1) {
                    printf("Invalid temperature value. Skipping this entry.\n");
                    while(getchar() != '\n');
                    i--; 
                    continue;
                }

                temp_sum += temp;
                valid_count++;

                const char *status;
                int led_state;

                
                if (temp >= min_temp && temp <= max_temp) {
                    status = "NORMAL";
                    led_state = 0; // LED OFF
                } else {
                    status = "WARNING";
                    led_state = 1; // LED ON
                }
                control_led(led_state);
                printf("\nTemperature \t   System Status \t   Warning LED\n");
                printf("%.1f°F\t\t\t%s\t\t\t%s\n\n", temp, status, (led_state ? "ON" : "OFF")); 
                file = fopen(FILENAME, "a");
                if (file != NULL) {
                    fprintf(file, "%.1f°F\t\t%s\n", temp, status);
                    fclose(file);
                }
            }
            if (valid_count > 0) {
                printf("--- Batch Summary ---\n");
                printf("Total readings processed: %d\n", valid_count);
                printf("Average temperature for this batch: %.2f°F\n", temp_sum / valid_count);
            }

        } else if (choice == 2) {
            printf("Enter new minimum acceptable temperature (°F): ");
            scanf("%lf", &min_temp);
            printf("Enter new maximum acceptable temperature (°F): ");
            scanf("%lf", &max_temp);
            printf("Range updated successfully: %.1f°F - %.1f°F\n", min_temp, max_temp);

        } else if (choice == 3) {
            printf("\nExiting program and ensuring LED is OFF...\n");
            break;
        } else {
            printf("Invalid choice. Please select 1, 2, or 3.\n");
        }
    }

    // Ensure LED turns OFF upon program exit
    control_led(0);
    return 0;
}
