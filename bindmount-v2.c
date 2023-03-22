#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <linux/fs.h>

static inline int sys_open_tree(int dfd, const char *filename, unsigned int flags)
{
	return syscall(SYS_open_tree, dfd, filename, flags);
}

static inline int sys_move_mount(int from_dirfd, const char *from_pathname, int to_dirfd, const char *to_pathname,
                                 unsigned int flags)
{
        return syscall(SYS_move_mount, from_dirfd, from_pathname, to_dirfd, to_pathname, flags);
}

static inline long sys_mount_setattr(int dfd, const char *path, unsigned int flags, struct mount_attr *uattr,
                                     size_t usize)
{
        return syscall(SYS_mount_setattr, dfd, path, flags, uattr, usize);
}

int close_safe(int *fd)
{
        int ret = 0;

        if (*fd > -1) {
                ret = close(*fd);
                if (!ret)
                        *fd = -1;
                else
                        printf("Unable to close fd %d: %m", *fd);
        }

        return ret;
}

int main(int argc, char **argv)
{
	char *src_ns = NULL, *src_mnt, *dst_ns = NULL, *dst_mnt, *match;
	int fd = -1, src_nsfd = -1, dst_nsfd = -1;
	struct mount_attr attr = {};

	if (argc != 3) {
		printf("usage: %s [<path/to/src/namespace>:]<path/to/mount/src> [<path/to/dst/namespace>:]<path/to/mount/dst>\n", argv[0]);
		return 1;
	}

	match = strchr(argv[1], ':');
	if (match) {
		src_ns = argv[1];
		src_mnt = match + 1;
		*match = '\0';
	} else {
		src_mnt = argv[1];
	}

	match = strchr(argv[2], ':');
	if (match) {
		dst_ns = argv[2];
		dst_mnt = match + 1;
		*match = '\0';
	} else {
		dst_mnt = argv[2];
	}

	if (src_ns) {
		src_nsfd = open(src_ns, O_RDONLY);
		if (src_nsfd == -1) {
			perror("open src ns");
			goto err;
		}
	}

	if (dst_ns) {
		dst_nsfd = open(dst_ns, O_RDONLY);
		if (dst_nsfd == -1) {
			perror("open dst ns");
			goto err;
		}
	}

	if (src_ns) {
		if (setns(src_nsfd, CLONE_NEWNS)) {
			perror("setns src");
			goto err;
		}
		close_safe(&src_nsfd);
	}

	fd = sys_open_tree(AT_FDCWD, src_mnt, AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW | OPEN_TREE_CLONE);
	if (fd == -1) {
		perror("sys_open_tree");
		goto err;
	}

	attr.propagation = MS_PRIVATE;
	if (sys_mount_setattr(fd, "", AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT, &attr, sizeof(attr))) {
		perror("sys_mount_setattr");
		goto err;
	}

	printf("Mounting %s%s%s into %s%s%s\n",
	       src_ns ? src_ns : "", src_ns ? ":" : "", src_mnt,
	       dst_ns ? dst_ns : "", dst_ns ? ":" : "", dst_mnt);

	if (dst_ns) {
		if (setns(dst_nsfd, CLONE_NEWNS)) {
			perror("setns dst");
			goto err;
		}
		close_safe(&dst_nsfd);
	}

	if (sys_move_mount(fd, "", AT_FDCWD, dst_mnt, MOVE_MOUNT_F_EMPTY_PATH)) {
		perror("sys_move_mount");
		goto err;
	}

	close_safe(&fd);
	return 0;
err:
	close_safe(&fd);
	close_safe(&dst_nsfd);
	close_safe(&src_nsfd);
	return 1;
}
