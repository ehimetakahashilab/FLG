#include <stdio.h>
#include <stdlib.h>

#include "declare.h"
#include "def_flt.h"

ELEMENT *gate;
int *pitbl, *potbl, *fftbl, *ffintbl, *flist;

void make_line_list(char *circuit_path) {
  L_NODE *fnode, *head_node, **address;
  FIN_NODE *finnode, *head_fin;
  int i, ni, fil, line, fin, fol, saval;
  int count2 = 0;
#if CKT_FORM == 0
  readf(circuit_path);
  int ie;
  int ic = 0;

  for (ie = 1; ie <= lpnt; ie++) {
    if (gate[ie].type == 3) {
      gate[ie].type = TPI;
      ic++;
    }
  }
#else
  readf_ehime(circuit_path);
#endif

  address = (L_NODE **)calloc(lpnt + 1, sizeof(L_NODE *));
  if (address == NULL) printf("memory error @make_line_list\n"), exit(1);

  head_node = NULL;
  for (ni = lpnt; ni >= 1; ni--) {
    if (gate[ni].type == FAN) continue;

    if (NULL == (fnode = NALLOC)) printf(" error -984-\n"), exit(1);
    address[ni] = fnode;
    fnode->line = ni;
    fnode->type = gate[ni].type;
    fnode->innum = gate[ni].nfi;
    fnode->outnum = gate[ni].nfo;
    fnode->next = head_node;

    if (head_node != NULL) head_node->prev = fnode;
    head_node = fnode;
  }

  for (ni = 1; ni <= lpnt; ni++) {
    if (gate[ni].type == FAN) {
      fil = gate[ni].fil;
      address[ni] = address[fil];
    }
  }
  fnode = head_node;
  for (; fnode != NULL; fnode = fnode->next) {
    line = fnode->line;

    /*** make fan-in lists ***/
    fil = gate[line].fil;
    if (gate[line].type == PI) {
      fnode->finlst = NULL;
    } else if (gate[line].nfi == 1) {
      if (NULL == (finnode = INALLOC)) printf(" error -239-\n"), exit(1);

      finnode->node = address[fil];
      finnode->next = NULL;
      fnode->finlst = finnode;
    } else {
      head_fin = NULL;
      for (ni = 0; ni < gate[line].nfi; ni++) {
        fin = flist[ni + fil];

        if (NULL == (finnode = INALLOC)) printf(" error -233-\n"), exit(1);

        finnode->node = address[fin];
        finnode->next = head_fin;
        head_fin = finnode;
      }
      fnode->finlst = head_fin;
    }
  }

  for (ni = 1; ni <= lpnt; ni++) {
    if (gate[ni].type == FAN) {
      fol = gate[ni].fol;
      address[ni] = address[fol];
    }
  }
  fnode = head_node;
  for (; fnode != NULL; fnode = fnode->next) {
    line = fnode->line;

    /*** make fan-out lists ***/
    fol = gate[line].fol;
    if (gate[line].type == PO) {
      fnode->foutlst = NULL;
    } else if (gate[line].nfo == 1) {
      if (NULL == (finnode = INALLOC)) printf(" error -239-\n"), exit(1);
      finnode->node = address[fol];
      finnode->next = NULL;
      fnode->foutlst = finnode;
    } else {
      head_fin = NULL;
      for (ni = 0; ni < gate[line].nfo; ni++) {
        fin = flist[ni + fol];

        if (NULL == (finnode = INALLOC)) printf(" error -233-\n"), exit(1);
        finnode->node = address[fin];
        finnode->next = head_fin;
        head_fin = finnode;
      }
      fnode->foutlst = head_fin;
    }
  }

  head_fin = &pinode;
  for (ni = 1; ni <= inpnum; ni++) {
    line = pitbl[ni];

    if (NULL == (finnode = INALLOC)) printf(" error -259-\n"), exit(1);
    finnode->node = address[line];
    head_fin->next = finnode;
    head_fin = finnode;
  }
  head_fin->next = NULL;

  head_fin = &ffnode;
  for (ni = 1; ni <= ffnum; ni++) {
    line = fftbl[ni];

    if (NULL == (finnode = INALLOC)) printf(" error -259-\n"), exit(1);
    finnode->node = address[line];
    head_fin->next = finnode;
    head_fin = finnode;
  }
  head_fin->next = NULL;

  head_fin = NULL;
  for (ni = numout; ni >= 1; ni--) {
    line = gate[potbl[ni]].fol;

    if (NULL == (finnode = INALLOC)) printf(" error -259-\n"), exit(1);
    finnode->node = address[line];
    finnode->next = head_fin;
    head_fin = finnode;
  }
  ponode.next = head_fin;
  gnode.next = head_node;
  head_node->prev = &gnode;

#if TRANSITIONFAULT
  make_Tranfault_list(address);
#else
  make_fault_list(address);
#endif

  free(gate);
  free(flist);
  free(pitbl);
  free(potbl);
  free(fftbl);
  free(ffintbl);
}

void make_fault_list(L_NODE **address) {
  FIN_NODE *finnode, *head_fin;
  FLT_NODE *head_flt, *flttag;
  int ni, saval, fil, fol;

  /** make fault list **/
  head_flt = &fltlst;
  sum_flt = 0;

  for (ni = 1; ni <= lpnt; ni++) {
    fol = gate[ni].fol;

    /*    if(gate[ni].nfo==1 && gate[fol].type==PO || gate[ni].type==PO) */
    if (gate[ni].type == PO) continue;
    if (gate[ni].nfo == 1 && gate[fol].type != FF && gate[fol].type != PO) {
      switch (gate[fol].type) {
        case AND:
        case NAND:
          saval = 1;
          break;
        case OR:
        case NOR:
          saval = 0;
          break;
        case NOT:  //インバーター出力側のs0,s1故障は、入力のs1とs0と等価
        case TPI:  // TPIの故障を考慮しない
        case XOR:  // XORタイプＴＰＩ
          continue;
        default:
          printf(" ni = %d %d %d error -9183-\n", ni, fol, gate[fol].type),
              exit(1);
      }
      if (NULL == (flttag = FALLOC))
        printf(" ni = %d error -639-\n", ni), exit(1);
      /***
      flttag=(fltlst_tmp+sum_flt);
***/
      sum_flt++;
      flttag->num = sum_flt;
      flttag->line = gate[ni].line;
      if (sum_flt >= MAXFLT)
        printf(" error 19485 sumflt = %d\n", sum_flt), exit(1);

      if (gate[ni].type == FAN)
        flttag->back = address[gate[ni].fil];
      else
        flttag->back = address[ni];
      flttag->forwd = address[fol];
      flttag->saval = saval;

      /**
      printf(" ni %d fol %d address[fol] %d\n",ni,fol,address[fol]->line);
**/
      flttag->prev = head_flt;
      head_flt->next = flttag;
      head_flt = flttag;
    }

    else if (gate[ni].nfo == 1) { /** gate[fol].type==FF or PO **/
      for (saval = 0; saval < 2; saval++) {
        if (NULL == (flttag = FALLOC)) printf(" error -639-\n"), exit(1);
        /***
        flttag=(fltlst_tmp+sum_flt);
***/
        sum_flt++;
        flttag->num = sum_flt;
        flttag->line = gate[ni].line;
        if (sum_flt >= MAXFLT)
          printf(" error 19486 sumflt = %d\n", sum_flt), exit(1);

        if (gate[ni].type != FAN)
          flttag->back = address[ni];
        else
          flttag->back = address[gate[ni].fil];
        flttag->forwd = address[fol];
        flttag->saval = saval;

        flttag->prev = head_flt;
        head_flt->next = flttag;
        head_flt = flttag;
      }
    } else {
      for (saval = 0; saval < 2; saval++) {
        if (NULL == (flttag = FALLOC)) printf(" error -639-\n"), exit(1);
        /***
        flttag=(fltlst_tmp+sum_flt);
***/
        sum_flt++;

        if (sum_flt >= MAXFLT)
          printf(" error 19487 sumflt = %d\n", sum_flt), exit(1);

        flttag->back = address[ni];
        flttag->forwd = NULL;
        flttag->saval = saval;
        flttag->num = sum_flt;
        flttag->line = gate[ni].line;

        flttag->prev = head_flt;
        head_flt->next = flttag;
        head_flt = flttag;
      }
    }
  }

  head_flt->next = NULL;
}

void make_Tranfault_list(L_NODE **address) {
  FIN_NODE *finnode, *head_fin;
  FLT_NODE *head_flt, *flttag;
  int ni, saval, saval1, fil, fol;

  /** make fault list **/
  head_flt = &fltlst;
  sum_flt = 0;
  sum_Tran_flt = 0;

  for (ni = 1; ni <= lpnt; ni++) {
    /*     if(ni<=250 || ni>=300) */
    /*       continue; */

    fol = gate[ni].fol;

    /*    if(gate[ni].nfo==1 && gate[fol].type==PO || gate[ni].type==PO) */
    if (gate[ni].type == PO) continue;
    if (gate[ni].nfo == 1 && gate[fol].type != FF && gate[fol].type != PO) {
      if (gate[fol].type != NOT) sum_flt++;
      for (saval = 0; saval < 2; saval++) {
        // if(gate[fol].type==NOT)
        // continue;
        switch (gate[fol].type) {
          case AND:
          case NAND:
            flttag->typeflog = 1;
            break;
          case OR:
          case NOR:
            flttag->typeflog = 2;
            break;
          case NOT:  //インバーター出力側のs0,s1故障は、入力のs1とs0と等価
          case TPI:  // TPIの故障を考慮しない
          case XOR:  // XORタイプＴＰＩ
            continue;
          default:
            printf(" ni = %d %d %d error -9183-\n", ni, fol, gate[fol].type),
                exit(1);
        }

        if (NULL == (flttag = FALLOC))
          printf(" ni = %d error -639-\n", ni), exit(1);
        /***
      flttag=(fltlst_tmp+sum_flt);

***/

        sum_Tran_flt++;
        flttag->num = sum_Tran_flt;
        if (sum_Tran_flt >= MAXFLT)
          printf(" error 19485 sumflt = %d\n", sum_Tran_flt), exit(1);

        if (gate[ni].type == FAN)
          flttag->back = address[gate[ni].fil];

        else
          flttag->back = address[ni];
        flttag->forwd = address[fol];
        flttag->saval = saval;

        /**
      printf(" ni %d fol %d address[fol] %d\n",ni,fol,address[fol]->line);
**/
        flttag->prev = head_flt;
        head_flt->next = flttag;
        head_flt = flttag;
      }
    }

    else if (gate[ni].nfo == 1) { /** gate[fol].type==FF or PO **/
      for (saval = 0; saval < 2; saval++) {
        if (NULL == (flttag = FALLOC)) printf(" error -639-\n"), exit(1);
        /***
        flttag=(fltlst_tmp+sum_flt);
***/
        sum_flt++;
        sum_Tran_flt++;
        flttag->num = sum_Tran_flt;
        if (sum_flt >= MAXFLT)
          printf(" error 19486 sumflt = %d\n", sum_flt), exit(1);

        if (gate[ni].type != FAN)
          flttag->back = address[ni];
        else
          flttag->back = address[gate[ni].fil];
        flttag->forwd = address[fol];
        flttag->saval = saval;
        flttag->typeflog = 0;

        flttag->prev = head_flt;
        head_flt->next = flttag;
        head_flt = flttag;
      }
    } else {
      for (saval = 0; saval < 2; saval++) {
        if (NULL == (flttag = FALLOC)) printf(" error -639-\n"), exit(1);
        /***
        flttag=(fltlst_tmp+sum_flt);
***/
        sum_flt++;
        sum_Tran_flt++;
        flttag->num = sum_Tran_flt;
        if (sum_flt >= MAXFLT)
          printf(" error 19487 sumflt = %d\n", sum_flt), exit(1);

        flttag->back = address[ni];
        flttag->forwd = NULL;
        flttag->saval = saval;
        flttag->typeflog = 0;

        flttag->prev = head_flt;
        head_flt->next = flttag;
        head_flt = flttag;
      }
    }
  }
  head_flt->next = NULL;
}

void output_flist(char *output_path, FLT_NODE *flist) {
  FILE *fp;
  if (NULL == (fp = fopen(output_path, "w"))) {
    printf("cannot open path: %s\n", output_path);
    exit(1);
  }

  fprintf(fp, "%d\n", sum_flt);
  for (; flist != NULL; flist = flist->next) {
    fprintf(fp, "%d %d\n", flist->line, flist->saval);
  }

  fclose(fp);
}

void readf(char *circuit_path) {
  int i, j;
  FILE *fp;
  char name[30];
  if (NULL == (fp = fopen(circuit_path, "r"))) {
    printf("There is not such a file(%s).\n", circuit_path);
    exit(2);
  }

  fscanf(fp, "%d%d%d%d%d%d", &lpnt, &numout, &inpnum, &slist, &ffnum, &numgate);
  gate = (ELEMENT *)calloc(lpnt + 1, sizeof(ELEMENT));
  flist = (int *)calloc(slist + 1, sizeof(int));
  pitbl = (int *)calloc(inpnum + 1, sizeof(int));
  potbl = (int *)calloc(numout + 1, sizeof(int));
  fftbl = (int *)calloc(ffnum + 1, sizeof(int));
  ffintbl = (int *)calloc(ffnum + 1, sizeof(int));

  if ((gate == NULL) || (flist == NULL) || (pitbl == NULL) || (potbl == NULL) ||
      (fftbl == NULL) || (ffintbl == NULL))
    printf("memory error @readf\n"), exit(1);

  for (i = 1; i <= lpnt; i++) {
    fscanf(fp, "%d", &j);
    gate[i].line = (CHGATA)j;
    fscanf(fp, "%d", &j);
    gate[i].type = (CHGATA)j;
    fscanf(fp, "%d", &j);
    gate[i].nfi = (CHGATA)j;
    fscanf(fp, "%d", &j);
    gate[i].fil = j;
#if NAME
    fscanf(fp, "%s", gate[i].name);
#else
    fscanf(fp, "%s", name);
#endif
    fscanf(fp, "%d", &j);
    gate[i].nfo = (CHGATA)j;

    fscanf(fp, "%d", &j);
    gate[i].fol = j;
  }
  for (i = 1; i <= slist; i++) {
    fscanf(fp, "%d%d", &j, &j);
    flist[i] = j;
  }

/* following is to make pilst[], polst[] and fflst[] in seqential circuit */
#if SEQEN
  for (i = 1; i <= inpnum; i++) {
    fscanf(fp, "%d%s%d", &j, name, &j);
    pitbl[i] = j;
  }
  for (i = 1; i <= numout; i++) {
    fscanf(fp, "%d%s%d", &j, name, &j);
    potbl[i] = j;
  }
  for (i = 1; i <= ffnum; i++) {
    fscanf(fp, "%d%d", &j, &j);
    fftbl[i] = j;
    ffintbl[i] = gate[j].fil;
  }
  /* following in using combinational circuit */

#else
  for (i = 1; i <= inpnum; i++) {
    fscanf(fp, "%d%s%d", &j, name, &j);
    pitbl[i] = j;
  }
  for (i = 1; i <= numout; i++) {
    fscanf(fp, "%d%s%d", &j, name, &j);
    potbl[i] = j;
  }
#endif
  fclose(fp);
}

void readf_ehime(char *circuit_path) {
  int i, j;
  FILE *fp;
  char name[30];

  if (NULL == (fp = fopen(circuit_path, "r"))) {
    printf("There is not such a file(%s).\n", circuit_path);
    exit(2);
  }

  fscanf(fp, "%d", &lpnt);

  // gate = (ELEMENT *)calloc(lpnt+1, sizeof(ELEMENT));
  flist = (int *)calloc(lpnt + 1, sizeof(int));

  if ((gate == NULL) || (flist == NULL))
    printf("memory error @readf\n"), exit(1);

  for (i = 1; i <= lpnt; i++) {
    fscanf(fp, "%d", &j);
    if (j == 9) j = 6;
    gate[i].type = (CHGATA)j;
    fscanf(fp, "%d", &j);
    gate[i].nfi = (CHGATA)j;
    fscanf(fp, "%d", &j);
    gate[i].fil = j;
    fscanf(fp, "%d", &j);
    gate[i].nfo = (CHGATA)j;

    fscanf(fp, "%d", &j);
    gate[i].fol = j;
  }

  fscanf(fp, "%d", &slist);

  for (i = 1; i <= slist; i++) {
    fscanf(fp, "%d", &j);
    flist[i] = j;
  }

  /* following is to make pilst[], polst[] and fflst[] in seqential circuit */
  fscanf(fp, "%d", &inpnum);
  // printf("%d\n", inpnum);
  for (i = 1; i <= inpnum; i++) {
    fscanf(fp, "%d", &j, &j);
    pitbl[i] = j;
  }
  fscanf(fp, "%d", &numout);
  for (i = 1; i <= numout; i++) {
    fscanf(fp, "%d", &j);
    potbl[i] = j;
  }

  ffnum = 0;
  for (i = 1; i <= lpnt; i++) {
    if (gate[i].type == FF) {
      fftbl[++ffnum] = i;
      ffintbl[ffnum] = gate[i].fil;
    }
  }

  fclose(fp);

  {
    int ni;
    for (ni = 1; ni <= lpnt; ni++) {
      printf(" %4d %2d %4d %2d %4d\n", gate[ni].type, gate[ni].nfi,
             gate[ni].fil, gate[ni].nfo, gate[ni].fol);
    }
  }
}
