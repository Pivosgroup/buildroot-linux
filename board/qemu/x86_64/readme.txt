Run the emulation with:

  qemu-system-x86_64 -M pc -kernel output/images/bzImage -drive file=output/images/rootfs.ext2,if=ide -append root=/dev/sda

The login prompt will appear in the graphical window.
