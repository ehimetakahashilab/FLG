#define SEQEN 1
#define NAME 0

/* maximum limits */
#define MAXFLT 100000  /* maximum number of flist */

#define NALLOC (L_NODE *)malloc(sizeof(L_NODE))
#define FALLOC (FLT_NODE *)malloc(sizeof(FLT_NODE))
#define INALLOC (FIN_NODE *)malloc(sizeof(FIN_NODE))

#define CKT_FORM 0         /* 0 for Osaka-format, 1 for Ehime-format */
#define TRANSITIONFAULT 0  // operate transition faults detection

/* gate type */
#define PI 0 /* primary input */
#define PO 4 /* primary output */
#define OR 1
#define AND 2
#define FAN 3 /* fanout branch */
#define NAND -1
#define NOR -2
#define NOT -3
#define XOR 5
#define XNOR -5
#define FF 6
#define TPI 7

/* signal value */
#define X 2
#define U 3
#define U0 4
#define U1 5
