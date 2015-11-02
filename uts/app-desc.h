#include "omp-tasks-app.h"
#include "uts.h"

#define BOTS_APP_NAME "Unbalance Tree Search"
#define BOTS_APP_PARAMETERS_DESC "%s"
#define BOTS_APP_PARAMETERS_LIST ,bots_arg_file

#define BOTS_APP_USES_ARG_FILE
#define BOTS_APP_DEF_ARG_FILE "Input filename"
#define BOTS_APP_DESC_ARG_FILE "UTS input file (mandatory)"

#define BOTS_APP_INIT \
  Node root; \
  uts_read_file(bots_arg_file);

#define KERNEL_INIT uts_initRoot(&root, type);

#define KERNEL_CALL bots_number_of_tasks = parallel_uts(&root);
 
#define KERNEL_FINI uts_show_stats();

#define KERNEL_CHECK uts_check_result();


