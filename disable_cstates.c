#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <linux/limits.h>

const int five_minutes = 60 * 5;

int find_max(char *dir_path, char *prefix)
{
	struct dirent *entry;
	int max = 0;
	DIR *dir;
	int val;

	dir = opendir(dir_path);
	if (!dir) {
		fprintf(stderr, "Failed to open %s directory: %m\n", dir_path);
		return -1;
	}

	while ((entry = readdir(dir))) {
		// Check name match
		if (!strncmp(prefix, entry->d_name, strlen(prefix))) {
			// Extract the C-state number
			val = atoi(entry->d_name + strlen(prefix));
			max = MAX(max, val);
		}
	}
	closedir(dir);

	return max;
}

void sigint_handler(int signum)
{
	printf("Caught signal %d, finishing sleep...\n", signum);
}

int main()
{
	int cpu, cstate;
	int max_cpu, max_cstate;

	max_cpu = find_max("/sys/devices/system/cpu", "cpu");
	if (max_cpu < 0) {
		fprintf(stderr, "Failed to determine maximum CPU number\n");
		return 1;
	}

	max_cstate = find_max("/sys/devices/system/cpu/cpu0/cpuidle", "state");
	if (max_cstate < 0) {
		fprintf(stderr, "Failed to determine maximum C-state number\n");
		return 1;
	}

	for (cpu = 0; cpu <= max_cpu; cpu++) {
		for (cstate = 1; cstate <= max_cstate; cstate++) {
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/disable", cpu, cstate);
			FILE *file = fopen(path, "w");
			if (file) {
				fprintf(file, "1\n");
				fclose(file);
			} else {
				fprintf(stderr, "Failed to disable C-state %d for CPU %d: %m\n", cstate, cpu);
			}
		}
	}
	printf("Disabled C-states [0-%d] for CPUs [0-%d]\n", max_cstate, max_cpu);

	signal(SIGINT, sigint_handler);
	printf("Sleep for 5 minutes\n");
	sleep(five_minutes);
	signal(SIGINT, SIG_DFL);

	for (cpu = 0; cpu <= max_cpu; cpu++) {
		for (cstate = 1; cstate <= max_cstate; cstate++) {
			char path[PATH_MAX];
			snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpuidle/state%d/disable", cpu, cstate);
			FILE *file = fopen(path, "w");
			if (file) {
				fprintf(file, "0\n");
				fclose(file);
			} else {
				fprintf(stderr, "Failed to enable C-state %d for CPU %d: %m\n", cstate, cpu);
			}
		}
	}
	printf("Re-enabled C-states [0-%d] for CPUs [0-%d]\n", max_cstate, max_cpu);

	return 0;
}
