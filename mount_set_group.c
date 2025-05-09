#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <linux/fs.h>

static inline int sys_move_mount(int from_dirfd, const char *from_pathname, int to_dirfd, const char *to_pathname,
                                 unsigned int flags)
{
        return syscall(SYS_move_mount, from_dirfd, from_pathname, to_dirfd, to_pathname, flags);
}

int main(int argc, char **argv)
{
	int srcfd, dstfd;

	if (argc != 3) {
		printf("usage: %s <path/to/src/mountpoint> <path/to/dst/mountpoint>\n", argv[0]);
		return 1;
	}

	srcfd = open(argv[1], O_RDONLY | O_DIRECTORY);
	if (srcfd < 0) {
		perror("open src");
		return 1;
	}
	dstfd = open(argv[2], O_RDONLY | O_DIRECTORY);
	if (dstfd < 0) {
		perror("open dst");
		close(srcfd);
		return 1;
	}

	if (sys_move_mount(srcfd, "", dstfd, "", MOVE_MOUNT_SET_GROUP | MOVE_MOUNT_F_EMPTY_PATH | MOVE_MOUNT_T_EMPTY_PATH)) {
		perror("sys_move_mount");
		close(srcfd);
		close(dstfd);
		return 1;
	}

	close(srcfd);
	close(dstfd);
	return 0;
}
