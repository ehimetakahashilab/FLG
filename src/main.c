#include "def_flt.h"

void main(int argc, char *argv[]) {
  make_line_list(argv[1]);
  output_flist(argv[2], fltlst.next);
  return;
}


