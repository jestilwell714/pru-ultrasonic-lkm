#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <unistd.h>

#define DEVICE_PATH "/dev/sensor"

int write_to_sysfs(const char *path, int value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open sysfs attribute");
        return -1;
    }

    char buffer[16];
    int len = sprintf(buffer, "%d", value);

    if (write(fd, buffer, len) < 0) {
        perror("Failed to write to sysfs");
        close(fd);
        return -1;
    }

    close(fd);
    printf("Successfully updated %s to %d\n", path, value);
    return 0;
}

int configuration() {
    int value;

    printf("CONFIG: Speed of Sound (0 for default 343)\n");
    scanf("%d", &value);
    if(value == 0) value = 343;
    if (write_to_sysfs("/sys/sensor/speed_of_sound", value) < 0) return -1;

    printf("CONFIG: Number of Sensors (1 for default)\n");
    scanf("%d", &value);
    if (write_to_sysfs("/sys/sensor/num_sensors", value) < 0) return -1;


    printf("CONFIG: History Window (5 for default)\n");
    scanf("%d", &value);
    if (write_to_sysfs("/sys/sensor/history_window", value) < 0) return -1;
    return 0;
}


int main() {


    int result = configuration();
    if(result < 0) {
        return -1;
    }

    int fd = open(DEVICE_PATH, O_RDWR);
    if(fd < 0) {
        perror("Failed to open the device");
        return EXIT_FAILURE;
    }

    unsigned int distance;
    

    for(int i = 0; i < 1000000; ++i) {
        ssize_t bytes_read = read(fd, &distance, sizeof(distance)); 



        if (bytes_read < 0) {
            perror("Failed to read from device\n");
        } else {
            printf("[SENSOR] Distance: %d cms\n", distance);
        }
    }

    close(fd);
    return 0;
}