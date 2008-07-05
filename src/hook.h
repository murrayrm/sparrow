/*!
 * \file hook.h 
 * \brief include file for using hook functions
 *
 * \author Richard M. Murray
 * \date 29 Oct 94
 *
 * \ingroup hook
 */

#ifndef _HOOK_INCLUDED_
#define _HOOK_INCLUDED_

/* Hook list */
struct hook_list {
    int length;                 /* length of list */
    int nhooks;                 /* number of hooks currently defined */
    int lock; 
    int (**fcnlist)(void);      /* list of hook functions */
};
typedef struct hook_list HOOK_LIST;

/* Macro for allocating space */
#define DECL_HOOKLIST(name, length) \
  static int (*_##name##_list[length])() = {NULL}; \
  HOOK_LIST name##_data = {length, 0, 0, _##name##_list}; \
  HOOK_LIST *name = &name##_data;

/* Function declarations */
extern int hook_add(HOOK_LIST *, int (*)());
extern int hook_remove(HOOK_LIST *, int (*)());
extern int hook_execute(HOOK_LIST *);
extern int hook_clear(HOOK_LIST *);

/* standard hooks used in the sparrow system */
extern HOOK_LIST *hook_foreground;
extern HOOK_LIST *hook_update;
extern HOOK_LIST *hook_exit;
extern HOOK_LIST *dd_loop_hooks;

#endif


