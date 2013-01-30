# Get a build host.

You need 32 (i386) bit libraries for the code sorcerer toolchain. One
simple way to do this is in an i386 chroot or virtual machine.

# Install the build pre-reqs.

In a fresh minimal Debian Unstable install, 

```shell
% sudo apt-get install git build-essential zip gawk libtool gettext  \
      automake autoconf  nasm unzip gettext flex bison libsdl-image1.2 \
      liblzo2-2 lzma python openjdk-7-jre-headless \
      texinfo libxml-parser-perl wget pkg-config swig cpio zlib1g-dev \
      libgmp-dev libmpfr-dev
```

Note that as of current (2012-10-10) Debian Unstable, python defaults
to python 2.7. Earlier pythons (or python3) will not work.

# Set up a key to sign the image

```shell
% keytool -genkey -keystore ~/.android/debug.keystore -v -alias \
      androiddebugkey -dname "CN=Android Debug,O=Android,C=US" -keypass \
      android -storepass android -keyalg RSA -keysize 2048 -validity 10000
```

# Build
First configure the source tree for the device you want to compile the image for. 
The following devices are supported at this moment;

      Pivos XIOS DS M1 == amlogic_xios-xbmc_defconfig

      Pivos XIOS DS M3 == amlogic_xios_m3-xbmc_defconfig

      Refee/OE Smart TV Box == amlogic_f16ref-xbmc_defconfig

      GBox Midnight == amlogic_f16ref-xbmc_defconfig

      Sumvision Nano M1 (non slim) == amlogic_stvmc-xbmc_defconfig (Actually the same as the xios M1)

      Sumvision Nano M3 (slim) == amlogic_stvmc-xbmc_defconfig

      Geniatech/MyGica Enjoy TV 510b == amlogic_stvmc-xbmc_defconfig

```shell
% make amlogic_xios-xbmc_defconfig
```
or
```shell
% make amlogic_xios_m3-xbmc_defconfig
```
or
```shell
% make amlogic_f16ref-xbmc_defconfig
```
or
```shell
% make amlogic_stvm3-xbmc_defconfig
```
or
```shell
% make amlogic_stvmc-xbmc_defconfig
```

Then build the update file
```shell
% make
```
