Run the emulation with:

  qemu-system-sparc -M SS-10 -kernel output/images/zImage -drive file=output/images/rootfs.ext2 -append "root=/dev/sda console=ttyS0,115200" -serial stdio -net nic,model=lance -net user

The login prompt will appear in the terminal that started Qemu.
The graphical window is the framebuffer.

Tested with QEMU 1.6.1
