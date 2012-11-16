reboot:
	$(CC) -I. main.c reboot.c android_reboot.c __set_errno.c __reboot.S -o reboot

install:
	install -m 755 reboot $(DESTDIR)/sbin

clean:
	-rm reboot

all: reboot
