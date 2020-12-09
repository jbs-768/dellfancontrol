#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>

#include "dell-bios-fan-control.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define HWMON_DIR "/sys/class/hwmon"

#define CPU_FAN HWMON_DIR "/hwmon2/pwm1"
#define GPU_FAN HWMON_DIR "/hwmon2/pwm2"

#define CPU_TEMP HWMON_DIR "/hwmon2/temp1_input"
#define GPU_TEMP HWMON_DIR "/hwmon2/temp6_input"

static const uint32_t hysteresis      = 10000; // 1000 = 1°C
static const uint32_t temp_levels[]   = {0, 10000, 20000, 81000, 82000};
static const uint32_t fan_levels[]    = {0, 0, 128, 128, 255}; // 255 = 100%
static const uint32_t update_interval = 2000;                  // ms

static size_t cpu_level = 0;
static size_t gpu_level = 0;

/** Called at abnormal exit */
static void
cleanup(int code)
{
    (void)code; // Unused

    set_bios_fan_control(1);
    exit(EXIT_FAILURE);
}

FILE*
FOPEN(const char* path, const char* type)
{
    FILE* fp = fopen(path, type);
    if (!fp) {
        fprintf(stderr, "Fatal error: could not open %s\n", path);
        raise(SIGABRT);
    }
    return fp;
}

uint32_t
read_value(const char* path)
{
    FILE* fp = FOPEN(path, "r");

    uint32_t temp;
    fscanf(fp, "%u", &temp);
    fclose(fp);
    return temp;
}

void
write_value(const char* path, const uint32_t value)
{
    FILE* fp = FOPEN(path, "w");

    fprintf(fp, "%u", value);
    fclose(fp);
}

void
print_status(void)
{
    const uint32_t cpu_temp = read_value(CPU_TEMP) / 1000;
    const uint32_t gpu_temp = read_value(GPU_TEMP) / 1000;
    const uint32_t cpu_fan  = 100 * read_value(CPU_FAN) / 255;
    const uint32_t gpu_fan  = 100 * read_value(GPU_FAN) / 255;

    printf("CPU: %3u°C, %3u%%\n", cpu_temp, cpu_fan);
    printf("GPU: %3u°C, %3u%%\n", gpu_temp, gpu_fan);
    printf("\n");
}

size_t
adjust_fan(const char* temp_path, const char* fan_path, size_t level)
{
    const uint32_t temp = read_value(temp_path);
    const uint32_t fan  = read_value(fan_path);

    if (fan_levels[level] != fan) {
        fprintf(stderr, "Fan level mismatch, expected %u, got %u\n",
                fan_levels[level], fan);
        // raise(SIGABRT);
    }

    if (temp > temp_levels[level]) {
        if (level < ARRAY_SIZE(temp_levels) - 1)
            ++level;
    }
    else if (temp < temp_levels[level] - hysteresis) {
        if (level > 0)
            --level;
    }

    write_value(fan_path, fan_levels[level]);
    return level;
}

int
main(void)
{
    if (geteuid() != 0) {
        fprintf(stderr, "Need root privileges.\n");
        return EXIT_FAILURE;
    }

    signal(SIGABRT, cleanup);
    signal(SIGFPE, cleanup);
    signal(SIGILL, cleanup);
    signal(SIGINT, cleanup);
    signal(SIGSEGV, cleanup);
    signal(SIGTERM, cleanup);

    set_bios_fan_control(false);
    write_value(CPU_FAN, fan_levels[cpu_level]);
    write_value(GPU_FAN, fan_levels[gpu_level]);

    while (1) {
        cpu_level = adjust_fan(CPU_TEMP, CPU_FAN, cpu_level);
        gpu_level = adjust_fan(GPU_TEMP, GPU_FAN, gpu_level);

        print_status();
        usleep(1000 * update_interval);
    }

    set_bios_fan_control(true);
    return EXIT_SUCCESS;
}
