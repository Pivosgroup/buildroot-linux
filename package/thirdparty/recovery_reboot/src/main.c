#define __BEGIN_DECLS
#define __END_DECLS
#include "cutils/android_reboot.h"

int main(int argc, char *argv[])
{
  reboot_main(argc, argv);
  return 0;
}
