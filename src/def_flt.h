typedef struct l_node L_NODE;
typedef struct fin_node FIN_NODE;
typedef struct flt_node FLT_NODE;

typedef int CHGATA;           /* int when debugging, char otherwise */

struct l_node {
  int line, type, innum, outnum;
  L_NODE *next, *prev;
  FIN_NODE *finlst, *foutlst;
  char *Name;
};

struct fin_node {
  L_NODE *node;
  FIN_NODE *next;
};

struct flt_node {
  int saval, typeflog, num, line;
  L_NODE *back, *forwd;
  FLT_NODE *next, *prev;
};

L_NODE gnode, inode;
FIN_NODE pinode, ponode, ffnode;
FLT_NODE fltlst;

/* net list */
typedef struct element {
  int type, nfi, nfo, line;
  int fil;
  int fol;
} ELEMENT;

/*Instance Variable Defination*/
int numout, slist, numgate;
int lpnt, inpnum, ffnum, sum_flt, sum_Tran_flt;

void make_line_list(char *circuit_path);
void make_fault_list(L_NODE **address);
void make_Tranfault_list(L_NODE **address);
void output_flist(char *output_path, FLT_NODE *flist);
void readf(char *circuit_path);
void readf_ehime(char *circuit_path);