/*  Boolector: Satisfiablity Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2007-2009 Robert Daniel Brummayer.
 *  Copyright (C) 2007-2014 Armin Biere.
 *  Copyright (C) 2012-2016 Mathias Preiner.
 *  Copyright (C) 2013-2017 Aina Niemetz.
 *
 *  All rights reserved.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#include "btorsat.h"

#include "sat/btorsatlgl.h"
#include "sat/btorsatminisat.h"
#include "sat/btorsatpicosat.h"

#include "btorabort.h"
#include "utils/btorutil.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

/*------------------------------------------------------------------------*/
// #define BTOR_PRINT_DIMACS_FOR_LINGELING // enable to print dimacs files
#ifdef BTOR_PRINT_DIMACS_FOR_LINGELING
#include <sys/types.h>  // for getpid
#include <unistd.h>     // for getpid
#endif
/*------------------------------------------------------------------------*/

#if defined(BTOR_USE_LINGELING)
#define btor_enable_default_sat(SMGR) btor_sat_enable_lingeling ((SMGR), 0, 0)
#elif defined(BTOR_USE_PICOSAT)
#define btor_enable_default_sat btor_sat_enable_picosat
#elif defined(BTOR_USE_MINISAT)
#define btor_enable_default_sat btor_sat_enable_minisat
#else
#error "no SAT solver configured"
#endif

/*------------------------------------------------------------------------*/

BtorSATMgr *
btor_sat_mgr_new (BtorMemMgr *mm, BtorMsg *msg)
{
  BtorSATMgr *smgr;

  assert (mm != NULL);

  BTOR_CNEW (mm, smgr);
  smgr->mm     = mm;
  smgr->msg    = msg;
  smgr->output = stdout;
  btor_enable_default_sat (smgr);
  BTOR_MSG (msg, 1, "enabled %s as default SAT solver", smgr->name);
  return smgr;
}

bool
btor_sat_mgr_has_clone_support (const BtorSATMgr *smgr)
{
  if (!smgr) return true;
  return smgr->api.clone != 0;
}

bool
btor_sat_mgr_has_term_support (const BtorSATMgr *smgr)
{
  if (!smgr) return false;
  return (!strcmp (smgr->name, "Lingeling"));
}

void
btor_sat_mgr_set_term (BtorSATMgr *smgr, int (*fun) (void *), void *state)
{
  assert (smgr);
  smgr->term.fun   = fun;
  smgr->term.state = state;
}

// FIXME log output handling, in particular: sat manager name output
// (see lingeling_sat) should be unique, which is not the case for
// clones
BtorSATMgr *
btor_sat_mgr_clone (BtorMemMgr *mm, BtorMsg *msg, BtorSATMgr *smgr)
{
  assert (smgr);
  assert (btor_sat_mgr_has_clone_support (smgr));
  assert (msg);
  assert (mm);

  BtorSATMgr *res;

  BTOR_ABORT (!btor_sat_mgr_has_clone_support (smgr),
              "SAT solver does not support cloning");
  BTOR_NEW (mm, res);
  res->solver = smgr->api.clone (smgr, mm);
  res->mm     = mm;
  res->msg    = msg;
  assert (res->mm->sat_allocated == smgr->mm->sat_allocated);
  res->name   = smgr->name;
  res->optstr = btor_mem_strdup (mm, smgr->optstr);
  memcpy (&res->inc_required,
          &smgr->inc_required,
          (char *) smgr + sizeof (*smgr) - (char *) &smgr->inc_required);
  return res;
}

bool
btor_sat_is_initialized (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  return smgr->initialized;
}

int
btor_sat_mgr_next_cnf_id (BtorSATMgr *smgr)
{
  int result;
  assert (smgr);
  assert (smgr->initialized);
  result = smgr->api.inc_max_var (smgr);
  if (abs (result) > smgr->maxvar) smgr->maxvar = abs (result);
  BTOR_ABORT (result <= 0, "CNF id overflow");
  if (btor_opt_get (smgr->msg->btor, BTOR_OPT_VERBOSITY) > 2
      && !(result % 100000))
    BTOR_MSG (smgr->msg, 2, "reached CNF id %d", result);
  return result;
}

void
btor_sat_mgr_release_cnf_id (BtorSATMgr *smgr, int lit)
{
  assert (smgr);
  if (!smgr->initialized) return;
  assert (abs (lit) <= smgr->maxvar);
  if (abs (lit) == smgr->true_lit) return;
  if (smgr->api.melt) smgr->api.melt (smgr, lit);
}

#if 0
int
btor_get_last_cnf_id_sat_mgr (BtorSATMgr * smgr)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  (void) smgr;
  return smgr->api.variables (smgr);
}
#endif

void
btor_sat_mgr_delete (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  /* if SAT is still initialized, then
   * reset_sat has not been called
   */
  if (smgr->initialized) btor_sat_reset (smgr);
  if (smgr->optstr) btor_mem_freestr (smgr->mm, smgr->optstr);
  BTOR_DELETE (smgr->mm, smgr);
}

/*------------------------------------------------------------------------*/

void
btor_sat_set_output (BtorSATMgr *smgr, FILE *output)
{
  char *prefix, *q;
  const char *p;

  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (output != NULL);
  (void) smgr;
  smgr->api.set_output (smgr, output);
  smgr->output = output;

  prefix = btor_mem_malloc (smgr->mm, strlen (smgr->name) + 4);
  sprintf (prefix, "[%s] ", smgr->name);
  q = prefix + 1;
  for (p = smgr->name; *p; p++) *q++ = tolower ((int) *p);
  smgr->api.set_prefix (smgr, prefix);
  btor_mem_free (smgr->mm, prefix, strlen (smgr->name) + 4);
}

void
btor_sat_init (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  assert (!smgr->initialized);
  BTOR_MSG (smgr->msg, 1, "initialized %s", smgr->name);

  smgr->solver = smgr->api.init (smgr);
  smgr->api.enable_verbosity (
      smgr, btor_opt_get (smgr->msg->btor, BTOR_OPT_VERBOSITY));
  smgr->initialized  = true;
  smgr->inc_required = true;
  smgr->sat_time     = 0;

  smgr->true_lit = btor_sat_mgr_next_cnf_id (smgr);
  btor_sat_add (smgr, smgr->true_lit);
  btor_sat_add (smgr, 0);
  btor_sat_set_output (smgr, stdout);
}

void
btor_sat_print_stats (BtorSATMgr *smgr)
{
  if (!smgr || !smgr->initialized) return;
  smgr->api.stats (smgr);
  BTOR_MSG (smgr->msg,
            1,
            "%d SAT calls in %.1f seconds",
            smgr->satcalls,
            smgr->sat_time);
}

void
btor_sat_add (BtorSATMgr *smgr, int lit)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  assert (!smgr->satcalls || smgr->inc_required);
  if (!lit) smgr->clauses++;
  (void) smgr->api.add (smgr, lit);
}

BtorSolverResult
btor_sat_sat (BtorSATMgr *smgr, int limit)
{
  double start = btor_util_time_stamp ();
  int sat_res;
  BtorSolverResult res;
  assert (smgr != NULL);
  assert (smgr->initialized);
  BTOR_MSG (
      smgr->msg, 2, "calling SAT solver %s with limit %d", smgr->name, limit);
  assert (!smgr->satcalls || smgr->inc_required);
  smgr->satcalls++;
  if (smgr->api.setterm) smgr->api.setterm (smgr);
  sat_res = smgr->api.sat (smgr, limit);
  smgr->sat_time += btor_util_time_stamp () - start;
  switch (sat_res)
  {
    case 10: res = BTOR_RESULT_SAT; break;
    case 20: res = BTOR_RESULT_UNSAT; break;
    default: assert (sat_res == 0); res = BTOR_RESULT_UNKNOWN;
  }
  return res;
}

int
btor_sat_deref (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  return smgr->api.deref (smgr, lit);
}

int
btor_sat_repr (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  return smgr->api.repr (smgr, lit);
}

void
btor_sat_reset (BtorSATMgr *smgr)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  BTOR_MSG (smgr->msg, 2, "resetting %s", smgr->name);
  smgr->api.reset (smgr);
  smgr->solver = 0;
  if (smgr->optstr)
  {
    btor_mem_freestr (smgr->mm, smgr->optstr);
    smgr->optstr = 0;
  }
  smgr->initialized = false;
}

int
btor_sat_fixed (BtorSATMgr *smgr, int lit)
{
  int res;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  res = smgr->api.fixed (smgr, lit);
  return res;
}

/*------------------------------------------------------------------------*/

void
btor_sat_assume (BtorSATMgr *smgr, int lit)
{
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  assert (!smgr->satcalls || smgr->inc_required);
  smgr->api.assume (smgr, lit);
}

int
btor_sat_failed (BtorSATMgr *smgr, int lit)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  assert (abs (lit) <= smgr->maxvar);
  return smgr->api.failed (smgr, lit);
}

#if 0
int
btor_sat_inconsistent (BtorSATMgr * smgr)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  return smgr->api.inconsistent (smgr);
}

int
btor_sat_changed (BtorSATMgr * smgr)
{
  (void) smgr;
  assert (smgr != NULL);
  assert (smgr->initialized);
  return smgr->api.changed (smgr);
}
#endif
