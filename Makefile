CFLAGS += -Wall

all: bindmount-v2 mount_set_group disable_cstates

bindmount-v2:
mount_set_group:
disable_cstates:

clean:
	rm -f bindmount-v2 mount_set_group disable_cstates
