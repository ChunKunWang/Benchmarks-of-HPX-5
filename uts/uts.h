#ifndef _UTS_H
#define _UTS_H

#include "brg_sha1.h"

#define UTS_VERSION "2.1"

/***********************************************************
 *  Tree node descriptor and statistics                    *
 ***********************************************************/

#define MAXNUMCHILDREN    100  // cap on children (BIN root is exempt)


struct node_t {
  int height;        // depth of this node in the tree
  int numChildren;   // number of children, -1 => not yet determined
 
  /* for RNG state associated with this node */
  struct state_t state;
};

typedef struct node_t Node;

/* Tree type
 *   Trees are generated using a Galton-Watson process, in 
 *   which the branching factor of each node is a random 
 *   variable.
 *   
 *   The random variable can follow a binomial distribution
 *   or a geometric distribution.  Hybrid tree are
 *   generated with geometric distributions near the
 *   root and binomial distributions towards the leaves.
 */

enum   uts_trees_e    { BIN = 0 };

typedef enum uts_trees_e    tree_t;

/* Strings for the above enums */
extern char * uts_trees_str[];


/* Tree  parameters */
extern tree_t     type;
extern double     b_0;
extern int        rootId;
extern int        nonLeafBF;
extern double     nonLeafProb;

/* Benchmark parameters */
extern int    computeGranularity;
extern int    debug;
extern int    verbose;

typedef unsigned long long counter_t; // for the count

/* Utility Functions */
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

counter_t parTreeSearch(int depth, Node *parent, int numChildren);

int    uts_paramsToStr(char *strBuf, int ind);
void   uts_read_file(char *file);
void   uts_print_params();

void   uts_showStats( void );

double rng_toProb(int n);

/* Common tree routines */
void   uts_initRoot(Node * root, int type);
int    uts_numChildren(Node *parent);
int    uts_numChildren_bin(Node * parent);
int    uts_numChildren_geo(Node * parent);
int    uts_childType(Node *parent);

#endif /* _UTS_H */



