/*  Boolector: Satisfiablity Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2007-2009 Robert Daniel Brummayer.
 *  Copyright (C) 2007-2013 Armin Biere.
 *  Copyright (C) 2012 Mathias Preiner.
 *  Copyright (C) 2013 Aina Niemetz.
 *
 *  All rights reserved.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

/*------------------------------------------------------------------------*/

#include "boolector.h"
#include "btorabort.h"
#include "btorclone.h"
#include "btordump.h"
#include "btorexit.h"
#include "btorexp.h"
#include "btorutil.h"

/*------------------------------------------------------------------------*/

#include <limits.h>
#include <stdio.h>
#include <string.h>

/*------------------------------------------------------------------------*/

#define BTOR_TRAPI(msg, args...)    \
  do                                \
  {                                 \
    if (!btor->apitrace) break;     \
    btor_trapi (btor, msg, ##args); \
  } while (0)

static void
btor_trapi (Btor *btor, const char *msg, ...)
{
  assert (btor);
  assert (btor->apitrace);

  va_list args;

  va_start (args, msg);
  vfprintf (btor->apitrace, msg, args);
  va_end (args);
  fputc ('\n', btor->apitrace);
  fflush (btor->apitrace);
}

static void
btor_open_apitrace (Btor *btor, const char *name)
{
  assert (btor);
  assert (name);

  FILE *file;
  char *cmd;
  int len = strlen (name);

  if (len >= 3 && !strcmp (name + len - 3, ".gz"))
  {
    len += 20;
    BTOR_NEWN (btor->mm, cmd, len);
    sprintf (cmd, "gzip -c > %s", name);
    if ((file = popen (cmd, "w"))) btor->closeapitrace = 2;
    BTOR_DELETEN (btor->mm, cmd, len);
  }
  else
  {
    if ((file = fopen (name, "w"))) btor->closeapitrace = 1;
  }

  if (file)
    btor->apitrace = file;
  else
    printf ("[boolector] WARNING failed to write API trace file to '%s'", name);
}

void
boolector_set_trapi (Btor *btor, FILE *apitrace)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_BOOLECTOR (btor->apitrace, "API trace already set");
  btor->apitrace = apitrace;
}

FILE *
boolector_get_trapi (Btor *btor)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  return btor->apitrace;
}

/*------------------------------------------------------------------------*/

void
boolector_chkclone (Btor *btor)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
#ifndef BTOR_USE_LINGELING
  BTOR_ABORT_BOOLECTOR (1, "cloning requires lingeling as SAT solver");
#endif
  BTOR_TRAPI ("chkclone");
  if (btor->clone) btor_delete_btor (btor->clone);
  btor->clone = btor_clone_btor (btor);
  assert (btor->clone->mm);
  assert (btor->clone->avmgr);
}

#define BTOR_CHKCLONE_STATE(field)        \
  do                                      \
  {                                       \
    assert (clone->field == btor->field); \
  } while (0)

static void
btor_chkclone_state (Btor *btor)
{
#ifndef NDEBUG
  assert (btor);

  Btor *clone;

  clone = btor->clone;
  assert (clone);

  BTOR_CHKCLONE_STATE (bv_lambda_id);
  BTOR_CHKCLONE_STATE (array_lambda_id);
  BTOR_CHKCLONE_STATE (dvn_id);
  BTOR_CHKCLONE_STATE (dan_id);
  BTOR_CHKCLONE_STATE (dpn_id);
  BTOR_CHKCLONE_STATE (rec_rw_calls);
  BTOR_CHKCLONE_STATE (rec_read_acond_calls);
  BTOR_CHKCLONE_STATE (valid_assignments);
  BTOR_CHKCLONE_STATE (rewrite_level);
  BTOR_CHKCLONE_STATE (verbosity);
#ifndef NBTORLOG
  BTOR_CHKCLONE_STATE (loglevel);
#endif
  BTOR_CHKCLONE_STATE (vis_idx);
  BTOR_CHKCLONE_STATE (vread_index_id);
  BTOR_CHKCLONE_STATE (inconsistent);
  BTOR_CHKCLONE_STATE (model_gen);
  BTOR_CHKCLONE_STATE (external_refs);
  BTOR_CHKCLONE_STATE (inc_enabled);
  BTOR_CHKCLONE_STATE (btor_sat_btor_called);
  BTOR_CHKCLONE_STATE (msgtick);
  BTOR_CHKCLONE_STATE (rewrite_writes);
  BTOR_CHKCLONE_STATE (rewrite_reads);
  BTOR_CHKCLONE_STATE (rewrite_aconds);
  BTOR_CHKCLONE_STATE (pprint);
  BTOR_CHKCLONE_STATE (last_sat_result);
  BTOR_CHKCLONE_STATE (generate_model_for_all_reads);
#endif
}

#define BTOR_CHKCLONE_STATS(field)                    \
  do                                                  \
  {                                                   \
    assert (clone->stats.field == btor->stats.field); \
  } while (0)

#define BTOR_CHKCLONE_CONSTRAINTSTATS(constraints, field)                     \
  do                                                                          \
  {                                                                           \
    assert (clone->stats.constraints.field == btor->stats.constraints.field); \
  } while (0)

static void
btor_chkclone_stats (Btor *btor)
{
#ifndef NDEBUG
  assert (btor);

  Btor *clone;

  clone = btor->clone;
  assert (clone);

  BTOR_CHKCLONE_STATS (max_rec_rw_calls);
  BTOR_CHKCLONE_STATS (lod_refinements);
  BTOR_CHKCLONE_STATS (synthesis_assignment_inconsistencies);
  BTOR_CHKCLONE_STATS (array_axiom_1_conflicts);
  BTOR_CHKCLONE_STATS (array_axiom_2_conflicts);
  BTOR_CHKCLONE_STATS (var_substitutions);
  BTOR_CHKCLONE_STATS (array_substitutions);
  BTOR_CHKCLONE_STATS (ec_substitutions);
  BTOR_CHKCLONE_STATS (vreads);
  BTOR_CHKCLONE_STATS (linear_equations);
  BTOR_CHKCLONE_STATS (gaussian_eliminations);
  BTOR_CHKCLONE_STATS (eliminated_slices);
  BTOR_CHKCLONE_STATS (skeleton_constraints);
  BTOR_CHKCLONE_STATS (adds_normalized);
  BTOR_CHKCLONE_STATS (muls_normalized);
  BTOR_CHKCLONE_STATS (read_props_construct);
  BTOR_CHKCLONE_STATS (lemmas_size_sum);
  BTOR_CHKCLONE_STATS (lclause_size_sum);

  BTOR_CHKCLONE_CONSTRAINTSTATS (constraints, varsubst);
  BTOR_CHKCLONE_CONSTRAINTSTATS (constraints, embedded);
  BTOR_CHKCLONE_CONSTRAINTSTATS (constraints, unsynthesized);
  BTOR_CHKCLONE_CONSTRAINTSTATS (constraints, synthesized);
  BTOR_CHKCLONE_CONSTRAINTSTATS (oldconstraints, varsubst);
  BTOR_CHKCLONE_CONSTRAINTSTATS (oldconstraints, embedded);
  BTOR_CHKCLONE_CONSTRAINTSTATS (oldconstraints, unsynthesized);
  BTOR_CHKCLONE_CONSTRAINTSTATS (oldconstraints, synthesized);

  BTOR_CHKCLONE_STATS (expressions);
  BTOR_CHKCLONE_STATS (beta_reduce_calls);
  BTOR_CHKCLONE_STATS (eval_exp_calls);
  BTOR_CHKCLONE_STATS (lambda_synth_reads);
  BTOR_CHKCLONE_STATS (lambda_chains_merged);
  BTOR_CHKCLONE_STATS (lambdas_merged);
  BTOR_CHKCLONE_STATS (propagations);
#endif
}

#define BTOR_CHKCLONE_TIME(field)                   \
  do                                                \
  {                                                 \
    assert (clone->time.field == btor->time.field); \
  } while (0)

static void
btor_chkclone_time (Btor *btor)
{
#ifndef NDEBUG
  assert (btor);

  Btor *clone;

  clone = btor->clone;
  assert (clone);

  BTOR_CHKCLONE_TIME (rewrite);
  BTOR_CHKCLONE_TIME (sat);
  BTOR_CHKCLONE_TIME (subst);
  BTOR_CHKCLONE_TIME (embedded);
  BTOR_CHKCLONE_TIME (slicing);
  BTOR_CHKCLONE_TIME (skel);
  BTOR_CHKCLONE_TIME (beta);
  BTOR_CHKCLONE_TIME (eval);
#endif
}

#define BTOR_CHKCLONE()         \
  do                            \
  {                             \
    if (!btor->clone) break;    \
    btor_chkclone_state (btor); \
    btor_chkclone_stats (btor); \
    btor_chkclone_time (btor);  \
  } while (0)

#define BTOR_CHKCLONE_AIG(field)                   \
  do                                               \
  {                                                \
    assert (real_clone->field == real_aig->field); \
  } while (0)

#define BTOR_CHKCLONE_AIGPID(field)                \
  do                                               \
  {                                                \
    if (!real_aig->field)                          \
    {                                              \
      assert (!real_clone->field);                 \
      break;                                       \
    }                                              \
    assert (real_clone->field == real_aig->field); \
  } while (0)

#define BTOR_CHKCLONE_AIG_TAGGED(field)                       \
  do                                                          \
  {                                                           \
    if (!real_aig->field)                                     \
    {                                                         \
      assert (!real_clone->field);                            \
      break;                                                  \
    }                                                         \
    int clonetag = BTOR_GET_TAG_NODE (real_clone->field);     \
    assert (clonetag == BTOR_GET_TAG_NODE (real_aig->field)); \
    assert (BTOR_REAL_ADDR_NODE (real_aig->field)->id         \
            == BTOR_REAL_ADDR_NODE (real_clone->field)->id);  \
  } while (0)

static void
btor_chkclone_aig (BtorAIG *aig, BtorAIG *clone)
{
#ifndef NDEBUG
  assert (aig);
  assert (clone);

  int i;
  BtorAIG *real_aig, *real_clone;

  real_aig   = BTOR_REAL_ADDR_AIG (aig);
  real_clone = BTOR_REAL_ADDR_AIG (clone);

  BTOR_CHKCLONE_AIG (id);
  BTOR_CHKCLONE_AIG (refs);

  for (i = 0; i < 2; i++) BTOR_CHKCLONE_AIG_TAGGED (children[i]);

  BTOR_CHKCLONE_AIGPID (next);

  BTOR_CHKCLONE_AIG (cnf_id);
  BTOR_CHKCLONE_AIG (mark);
  BTOR_CHKCLONE_AIG (clone_mark);
  BTOR_CHKCLONE_AIG (local);
#endif
}

#define BTOR_CHKCLONE_EXP(field)                   \
  do                                               \
  {                                                \
    assert (real_clone->field == real_exp->field); \
  } while (0)

#define BTOR_CHKCLONE_EXPPID(field)                \
  do                                               \
  {                                                \
    if (!real_exp->field)                          \
    {                                              \
      assert (!real_clone->field);                 \
      break;                                       \
    }                                              \
    assert (real_clone->field == real_exp->field); \
  } while (0)

/* Note: used for both children and parent pointers */
#define BTOR_CHKCLONE_EXP_TAGGED(field)                       \
  do                                                          \
  {                                                           \
    if (!real_exp->field)                                     \
    {                                                         \
      assert (!real_clone->field);                            \
      break;                                                  \
    }                                                         \
    int clonetag = BTOR_GET_TAG_NODE (real_clone->field);     \
    assert (clonetag == BTOR_GET_TAG_NODE (real_exp->field)); \
    assert (BTOR_REAL_ADDR_NODE (real_exp->field)->id         \
            == BTOR_REAL_ADDR_NODE (real_clone->field)->id);  \
  } while (0)

static void
btor_chkclone_exp (BtorNode *exp, BtorNode *clone)
{
#ifndef NDEBUG
  assert (exp);
  assert (clone);

  assert ((!BTOR_IS_INVERTED_NODE (exp) && !BTOR_IS_INVERTED_NODE (clone))
          || (BTOR_IS_INVERTED_NODE (exp) && BTOR_IS_INVERTED_NODE (clone)));

  int i, len;
  BtorNode *real_exp, *real_clone;

  real_exp   = BTOR_REAL_ADDR_NODE (exp);
  real_clone = BTOR_REAL_ADDR_NODE (clone);

  BTOR_CHKCLONE_EXP (kind);
  BTOR_CHKCLONE_EXP (mark);
  BTOR_CHKCLONE_EXP (aux_mark);
  BTOR_CHKCLONE_EXP (array_mark);
  BTOR_CHKCLONE_EXP (beta_mark);
  BTOR_CHKCLONE_EXP (clone_mark);
  BTOR_CHKCLONE_EXP (eval_mark);
  BTOR_CHKCLONE_EXP (synth_mark);
  BTOR_CHKCLONE_EXP (reachable);
  BTOR_CHKCLONE_EXP (tseitin);
  BTOR_CHKCLONE_EXP (vread);
  BTOR_CHKCLONE_EXP (vread_index);
  BTOR_CHKCLONE_EXP (constraint);
  BTOR_CHKCLONE_EXP (erased);
  BTOR_CHKCLONE_EXP (disconnected);
  BTOR_CHKCLONE_EXP (unique);
  BTOR_CHKCLONE_EXP (bytes);
  BTOR_CHKCLONE_EXP (arity);
  BTOR_CHKCLONE_EXP (parameterized);
  BTOR_CHKCLONE_EXP (lambda_below);
  BTOR_CHKCLONE_EXP (no_synth);

  if (real_exp->bits)
  {
    len = strlen (real_exp->bits);
    assert ((size_t) len == strlen (real_clone->bits));
    for (i = 0; i < len; i++) assert (real_exp->bits[i] == real_clone->bits[i]);
  }
  else
    assert (real_exp->bits == real_clone->bits);

  BTOR_CHKCLONE_EXP (id);
  BTOR_CHKCLONE_EXP (len);
  BTOR_CHKCLONE_EXP (refs);

  /* rho is not cloned, hence not checked */
  if (!BTOR_IS_ARRAY_NODE (real_exp))
  {
    if (real_exp->av)
    {
      assert (real_exp->av->len == real_clone->av->len);
      for (i = 0; i < real_exp->len; i++)
        btor_chkclone_aig (real_exp->av->aigs[i], real_clone->av->aigs[i]);
    }
    else
      assert (real_exp->av == real_clone->av);
  }

  BTOR_CHKCLONE_EXPPID (next);
  BTOR_CHKCLONE_EXPPID (parent);
  BTOR_CHKCLONE_EXPPID (simplified);
  BTOR_CHKCLONE_EXPPID (first_parent);
  BTOR_CHKCLONE_EXPPID (last_parent);

  if (!BTOR_IS_BV_CONST_NODE (real_exp) && !BTOR_IS_BV_VAR_NODE (real_exp)
      && !BTOR_IS_ARRAY_VAR_NODE (real_exp) && !BTOR_IS_PARAM_NODE (real_exp))
  {
    assert (real_exp->arity == real_clone->arity);
    if (real_exp->arity)
    {
      for (i = 0; i < real_exp->arity; i++) BTOR_CHKCLONE_EXP_TAGGED (e[i]);
    }
    else
    {
      assert (!strcmp (real_exp->symbol, real_clone->symbol));
      BTOR_CHKCLONE_EXP (upper);
      if (!BTOR_IS_ARRAY_EQ_NODE (real_exp))
        BTOR_CHKCLONE_EXP (lower);
      else
      {
        assert (real_exp->vreads->exp1->id == real_clone->vreads->exp1->id);
        assert (real_exp->vreads->exp2->id == real_clone->vreads->exp2->id);
      }
    }

    for (i = 0; i < real_exp->arity; i++)
    {
      BTOR_CHKCLONE_EXP_TAGGED (prev_parent[i]);
      BTOR_CHKCLONE_EXP_TAGGED (next_parent[i]);
    }
  }

  if (BTOR_IS_ARRAY_NODE (real_exp))
  {
    BTOR_CHKCLONE_EXP (index_len);
    BTOR_CHKCLONE_EXP_TAGGED (first_aeq_acond_parent);
    BTOR_CHKCLONE_EXP_TAGGED (last_aeq_acond_parent);

    if (!BTOR_IS_ARRAY_VAR_NODE (real_exp))
    {
      for (i = 0; i < real_exp->arity; i++)
      {
        BTOR_CHKCLONE_EXP_TAGGED (prev_aeq_acond_parent[i]);
        BTOR_CHKCLONE_EXP_TAGGED (next_aeq_acond_parent[i]);
      }
    }
  }
#endif
}

#define BTOR_CHKCLONENORES(fun, args...) \
  do                                     \
  {                                      \
    if (!btor->clone) break;             \
    fun (btor->clone, ##args);           \
    BTOR_CHKCLONE ();                    \
  } while (0)

#define BTOR_CHKCLONERES(fun, res, args...)   \
  do                                          \
  {                                           \
    if (!btor->clone) break;                  \
    int cloneres = fun (btor->clone, ##args); \
    assert (cloneres == res);                 \
    BTOR_CHKCLONE ();                         \
  } while (0)

#define BTOR_CHKCLONERESP(fun, res, args...)        \
  do                                                \
  {                                                 \
    if (!btor->clone) break;                        \
    BtorNode *cloneres = fun (btor->clone, ##args); \
    btor_chkclone_exp (res, cloneres);              \
    BTOR_CHKCLONE ();                               \
  } while (0)

#define BTOR_CHKCLONERESS(fun, res, args...)          \
  do                                                  \
  {                                                   \
    if (!btor->clone) break;                          \
    const char *cloneres = fun (btor->clone, ##args); \
    assert (!strcmp (cloneres, res));                 \
    BTOR_CHKCLONE ();                                 \
  } while (0)

/*------------------------------------------------------------------------*/

#define BTOR_TRAPI_RETURN(fun, res, args...) \
  do                                         \
  {                                          \
    BTOR_TRAPI ("return %d", res);           \
    BTOR_CHKCLONERES (fun, res, ##args);     \
  } while (0)

#define BTOR_TRAPI_RETURNP(fun, res, args...) \
  do                                          \
  {                                           \
    BTOR_TRAPI ("return %p", res);            \
    BTOR_CHKCLONERESP (fun, res, ##args);     \
  } while (0)

#define BTOR_TRAPI_RETURNS(fun, res, args...) \
  do                                          \
  {                                           \
    BTOR_TRAPI ("return %s", res);            \
    BTOR_CHKCLONERESS (fun, res, ##args);     \
  } while (0)

/*------------------------------------------------------------------------*/

Btor *
boolector_new (void)
{
  char *trname;
  Btor *btor;

  btor = btor_new_btor ();
  if ((trname = getenv ("BTORAPITRACE"))) btor_open_apitrace (btor, trname);
  BTOR_TRAPI ("new");
  return btor;
}

Btor *
boolector_clone (Btor *btor)
{
  BTOR_TRAPI ("clone"); /* just log, do nothing else */
  return btor_clone_btor (btor);
}

void
boolector_set_rewrite_level (Btor *btor, int rewrite_level)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("set_rewrite_level %d", rewrite_level);
  BTOR_ABORT_BOOLECTOR (rewrite_level < 0 || rewrite_level > 3,
                        "'rewrite_level' has to be in [0,3]");
  BTOR_ABORT_BOOLECTOR (
      BTOR_COUNT_STACK (btor->nodes_id_table) > 2,
      "setting rewrite level must be done before creating expressions");
  btor_set_rewrite_level_btor (btor, rewrite_level);
  BTOR_CHKCLONENORES (boolector_set_rewrite_level, rewrite_level);
}

void
boolector_enable_model_gen (Btor *btor)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("enable_model_gen");
  BTOR_ABORT_BOOLECTOR (
      BTOR_COUNT_STACK (btor->nodes_id_table) > 2,
      "enabling model generation must be done before creating expressions");
  btor_enable_model_gen (btor);
  BTOR_CHKCLONENORES (boolector_enable_model_gen);
}

void
boolector_generate_model_for_all_reads (Btor *btor)
{
  // TODO TRAPI
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  btor_generate_model_for_all_reads (btor);
  BTOR_CHKCLONENORES (boolector_generate_model_for_all_reads);
}

void
boolector_enable_inc_usage (Btor *btor)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("enable_inc_usage");
  BTOR_ABORT_BOOLECTOR (
      btor->btor_sat_btor_called > 0,
      "enabling incremental usage must be done before calling 'boolector_sat'");
  btor_enable_inc_usage (btor);
  BTOR_CHKCLONENORES (boolector_enable_inc_usage);
}

int
boolector_set_sat_solver (Btor *btor, const char *solver)
{
  int res;
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("set_sat_solver %d", solver);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (solver);
  BTOR_ABORT_BOOLECTOR (
      btor->btor_sat_btor_called > 0,
      "setting the SAT solver must be done before calling 'boolector_sat'");
  res = btor_set_sat_solver (btor, solver);
  BTOR_TRAPI_RETURN (boolector_set_sat_solver, res, solver);
  return res;
}

int
boolector_get_refs (Btor *btor)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("get_refs");
  res = btor->external_refs;
  BTOR_TRAPI_RETURN (boolector_get_refs, res);
  return res;
}

void
boolector_delete (Btor *btor)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("delete");
  if (btor->closeapitrace == 1)
    fclose (btor->apitrace);
  else if (btor->closeapitrace == 2)
    pclose (btor->apitrace);
  btor_delete_btor (btor);
  BTOR_CHKCLONENORES (boolector_delete);
}

BtorNode *
boolector_const (Btor *btor, const char *bits)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("const %s", bits);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (bits);
  BTOR_ABORT_BOOLECTOR (*bits == '\0', "'bits' must not be empty");
  btor->external_refs++;
  res = btor_const_exp (btor, bits);
  BTOR_TRAPI_RETURNP (boolector_const, res, bits);
  return res;
}

BtorNode *
boolector_zero (Btor *btor, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("zero %d", width);
  BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
  btor->external_refs++;
  res = btor_zero_exp (btor, width);
  BTOR_TRAPI_RETURNP (boolector_zero, res, width);
  return res;
}

BtorNode *
boolector_false (Btor *btor)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("false");
  btor->external_refs++;
  res = btor_false_exp (btor);
  BTOR_TRAPI_RETURNP (boolector_false, res);
  return res;
}

BtorNode *
boolector_ones (Btor *btor, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ones %d", width);
  BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
  btor->external_refs++;
  res = btor_ones_exp (btor, width);
  BTOR_TRAPI_RETURNP (boolector_ones, res, width);
  return res;
}

BtorNode *
boolector_true (Btor *btor)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("true");
  btor->external_refs++;
  res = btor_true_exp (btor);
  BTOR_TRAPI_RETURNP (boolector_true, res);
  return res;
}

BtorNode *
boolector_one (Btor *btor, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("one %d", width);
  BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
  btor->external_refs++;
  res = btor_one_exp (btor, width);
  BTOR_TRAPI_RETURNP (boolector_one, res, width);
  return res;
}

BtorNode *
boolector_unsigned_int (Btor *btor, unsigned int u, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("unsigned_int %u %d", u, width);
  BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
  btor->external_refs++;
  res = btor_unsigned_to_exp (btor, u, width);
  BTOR_TRAPI_RETURNP (boolector_unsigned_int, res, u, width);
  return res;
}

BtorNode *
boolector_int (Btor *btor, int i, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("int %d %u", i, width);
  BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
  btor->external_refs++;
  res = btor_int_to_exp (btor, i, width);
  BTOR_TRAPI_RETURNP (boolector_int, res, i, width);
  return res;
}

BtorNode *
boolector_var (Btor *btor, int width, const char *symbol)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);

  BtorNode *res;
  char *symb;

  if ((symb = (char *) symbol) == NULL)
  {
    BTOR_NEWN (btor->mm, symb, 20);
    sprintf (symb, "DVN%d", btor->dvn_id++);
    BTOR_TRAPI ("var %d %s", width, symb);
    BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
    btor->external_refs++;
    res = btor_var_exp (btor, width, symb);
  }
  else
  {
    BTOR_TRAPI ("var %d %s", width, symbol);
    BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
    btor->external_refs++;
    res = btor_var_exp (btor, width, symbol);
  }

  if (symbol == NULL) BTOR_DELETEN (btor->mm, symb, 20);
  BTOR_TRAPI_RETURNP (boolector_var, res, width, symbol);
  return res;
}

BtorNode *
boolector_array (Btor *btor,
                 int elem_width,
                 int index_width,
                 const char *symbol)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);

  BtorNode *res;
  char *symb;

  if ((symb = (char *) symbol) == NULL)
  {
    BTOR_NEWN (btor->mm, symb, 20);
    sprintf (symb, "DAN%d", btor->dan_id++);
    BTOR_TRAPI ("array %d %d %s", elem_width, index_width, symb);
    BTOR_ABORT_BOOLECTOR (elem_width < 1, "'elem_width' must not be < 1");
    BTOR_ABORT_BOOLECTOR (index_width < 1, "'index_width' must not be < 1");
    btor->external_refs++;
    res = btor_array_exp (btor, elem_width, index_width, symb);
  }
  else
  {
    BTOR_TRAPI ("array %d %d %s", elem_width, index_width, symbol);
    BTOR_ABORT_BOOLECTOR (elem_width < 1, "'elem_width' must not be < 1");
    BTOR_ABORT_BOOLECTOR (index_width < 1, "'index_width' must not be < 1");
    btor->external_refs++;
    res = btor_array_exp (btor, elem_width, index_width, symbol);
  }

  if (symbol == NULL) BTOR_DELETEN (btor->mm, symb, 20);
  BTOR_TRAPI_RETURNP (boolector_array, res, elem_width, index_width, symbol);
  return res;
}

BtorNode *
boolector_not (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("not %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  btor->external_refs++;
  res = btor_not_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_not, res, exp);
  return res;
}

BtorNode *
boolector_neg (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("neg %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  btor->external_refs++;
  res = btor_neg_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_neg, res, exp);
  return res;
}

BtorNode *
boolector_redor (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("redor %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  btor->external_refs++;
  res = btor_redor_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_redor, res, exp);
  return res;
}

BtorNode *
boolector_redxor (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("redxor %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  btor->external_refs++;
  res = btor_redxor_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_redxor, res, exp);
  return res;
}

BtorNode *
boolector_redand (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("redand %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  btor->external_refs++;
  res = btor_redand_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_redand, res, exp);
  return res;
}

BtorNode *
boolector_slice (Btor *btor, BtorNode *exp, int upper, int lower)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("slice %p %d %d", exp, upper, lower);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (lower < 0, "'lower' must not be negative");
  BTOR_ABORT_BOOLECTOR (upper < lower, "'upper' must not be < 'lower'");
  BTOR_ABORT_BOOLECTOR (upper >= BTOR_REAL_ADDR_NODE (exp)->len,
                        "'upper' must not be >= width of 'exp'");
  btor->external_refs++;
  res = btor_slice_exp (btor, exp, upper, lower);
  BTOR_TRAPI_RETURNP (boolector_slice, res, exp, upper, lower);
  return res;
}

BtorNode *
boolector_uext (Btor *btor, BtorNode *exp, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("uext %p %d", exp, width);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (width < 0, "'width' must not be negative");
  btor->external_refs++;
  res = btor_uext_exp (btor, exp, width);
  BTOR_TRAPI_RETURNP (boolector_uext, res, exp, width);
  return res;
}

BtorNode *
boolector_sext (Btor *btor, BtorNode *exp, int width)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sext %p %d", exp, width);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (width < 0, "'width' must not be negative");
  btor->external_refs++;
  res = btor_sext_exp (btor, exp, width);
  BTOR_TRAPI_RETURNP (boolector_sext, res, exp, width);
  return res;
}

BtorNode *
boolector_implies (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("implies %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_BOOLECTOR (
      BTOR_REAL_ADDR_NODE (e0)->len != 1 || BTOR_REAL_ADDR_NODE (e1)->len != 1,
      "bit-width of 'e0' and 'e1' have be 1");
  btor->external_refs++;
  res = btor_implies_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_implies, res, e0, e1);
  return res;
}

BtorNode *
boolector_iff (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("iff %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_BOOLECTOR (
      BTOR_REAL_ADDR_NODE (e0)->len != 1 || BTOR_REAL_ADDR_NODE (e1)->len != 1,
      "bit-width of 'e0' and 'e1' must not be unequal to 1");
  btor->external_refs++;
  res = btor_iff_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_iff, res, e0, e1);
  return res;
}

BtorNode *
boolector_xor (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("xor %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_xor_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_xor, res, e0, e1);
  return res;
}

BtorNode *
boolector_xnor (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("xnor %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_xnor_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_xnor, res, e0, e1);
  return res;
}

BtorNode *
boolector_and (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("and %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_and_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_and, res, e0, e1);
  return res;
}

BtorNode *
boolector_nand (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("nand %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_nand_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_nand, res, e0, e1);
  return res;
}

BtorNode *
boolector_or (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("or %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_or_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_or, res, e0, e1);
  return res;
}

BtorNode *
boolector_nor (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("nor %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_nor_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_nor, res, e0, e1);
  return res;
}

BtorNode *
boolector_eq (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *real_e0, *real_e1, *res;
  int is_array_e0, is_array_e1;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("eq %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0          = btor_simplify_exp (btor, e0);
  e1          = btor_simplify_exp (btor, e1);
  real_e0     = BTOR_REAL_ADDR_NODE (e0);
  real_e1     = BTOR_REAL_ADDR_NODE (e1);
  is_array_e0 = BTOR_IS_ARRAY_NODE (real_e0);
  is_array_e1 = BTOR_IS_ARRAY_NODE (real_e1);
  BTOR_ABORT_BOOLECTOR (is_array_e0 != is_array_e1,
                        "array must not be compared to bit-vector");
  BTOR_ABORT_BOOLECTOR (
      !is_array_e0 && real_e0 && real_e1 && real_e0->len != real_e1->len,
      "bit-vectors must not have unequal bit-width");
  BTOR_ABORT_BOOLECTOR (
      is_array_e0 && real_e0 && real_e1 && real_e0->len != real_e1->len,
      "arrays must not have unequal element bit-width");
  BTOR_ABORT_BOOLECTOR (is_array_e0 && real_e0 && real_e1
                            && real_e0->index_len != real_e1->index_len,
                        "arrays must not have unequal index bit-width");
  btor->external_refs++;
  res = btor_eq_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_eq, res, e0, e1);
  return res;
}

BtorNode *
boolector_ne (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *real_e0, *real_e1, *res;
  int is_array_e0, is_array_e1;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ne %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0          = btor_simplify_exp (btor, e0);
  e1          = btor_simplify_exp (btor, e1);
  real_e0     = BTOR_REAL_ADDR_NODE (e0);
  real_e1     = BTOR_REAL_ADDR_NODE (e1);
  is_array_e0 = BTOR_IS_ARRAY_NODE (real_e0);
  is_array_e1 = BTOR_IS_ARRAY_NODE (real_e1);
  BTOR_ABORT_BOOLECTOR (is_array_e0 != is_array_e1,
                        "array must not be compared to bit-vector");
  BTOR_ABORT_BOOLECTOR (is_array_e0 && real_e0->len != real_e1->len,
                        "arrays must not have unequal element bit-width");
  BTOR_ABORT_BOOLECTOR (is_array_e0 && real_e0->index_len != real_e1->index_len,
                        "arrays must not have unequal index bit-width");
  btor->external_refs++;
  res = btor_ne_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ne, res, e0, e1);
  return res;
}

BtorNode *
boolector_add (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("add %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_add_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_add, res, e0, e1);
  return res;
}

BtorNode *
boolector_uaddo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("uaddo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_uaddo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_uaddo, res, e0, e1);
  return res;
}

BtorNode *
boolector_saddo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("saddo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_saddo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_saddo, res, e0, e1);
  return res;
}

BtorNode *
boolector_mul (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("mul %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_mul_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_mul, res, e0, e1);
  return res;
}

BtorNode *
boolector_umulo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("umulo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_umulo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_umulo, res, e0, e1);
  return res;
}

BtorNode *
boolector_smulo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("smulo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_smulo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_smulo, res, e0, e1);
  return res;
}

BtorNode *
boolector_ult (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ult %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_ult_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ult, res, e0, e1);
  return res;
}

BtorNode *
boolector_slt (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("slt %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_slt_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_slt, res, e0, e1);
  return res;
}

BtorNode *
boolector_ulte (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ulte %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_ulte_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ulte, res, e0, e1);
  return res;
}

BtorNode *
boolector_slte (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("slte %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_slte_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_slte, res, e0, e1);
  return res;
}

BtorNode *
boolector_ugt (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ugt %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_ugt_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ugt, res, e0, e1);
  return res;
}

BtorNode *
boolector_sgt (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sgt %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_sgt_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sgt, res, e0, e1);
  return res;
}

BtorNode *
boolector_ugte (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ugte %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_ugte_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ugte, res, e0, e1);
  return res;
}

BtorNode *
boolector_sgte (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sgte %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_sgte_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sgte, res, e0, e1);
  return res;
}

BtorNode *
boolector_sll (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  int len;
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sll %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  len = BTOR_REAL_ADDR_NODE (e0)->len;
  BTOR_ABORT_BOOLECTOR (!btor_is_power_of_2_util (len),
                        "bit-width of 'e0' must be a power of 2");
  BTOR_ABORT_BOOLECTOR (
      btor_log_2_util (len) != BTOR_REAL_ADDR_NODE (e1)->len,
      "bit-width of 'e1' must be equal to log2(bit-width of 'e0')");
  btor->external_refs++;
  res = btor_sll_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sll, res, e0, e1);
  return res;
}

BtorNode *
boolector_srl (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  int len;
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("srl %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  len = BTOR_REAL_ADDR_NODE (e0)->len;
  BTOR_ABORT_BOOLECTOR (!btor_is_power_of_2_util (len),
                        "bit-width of 'e0' must be a power of 2");
  BTOR_ABORT_BOOLECTOR (
      btor_log_2_util (len) != BTOR_REAL_ADDR_NODE (e1)->len,
      "bit-width of 'e1' must be equal to log2(bit-width of 'e0')");
  btor->external_refs++;
  res = btor_srl_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_srl, res, e0, e1);
  return res;
}

BtorNode *
boolector_sra (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  int len;
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sra %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  len = BTOR_REAL_ADDR_NODE (e0)->len;
  BTOR_ABORT_BOOLECTOR (!btor_is_power_of_2_util (len),
                        "bit-width of 'e0' must be a power of 2");
  BTOR_ABORT_BOOLECTOR (
      btor_log_2_util (len) != BTOR_REAL_ADDR_NODE (e1)->len,
      "bit-width of 'e1' must be equal to log2(bit-width of 'e0')");
  btor->external_refs++;
  res = btor_sra_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sra, res, e0, e1);
  return res;
}

BtorNode *
boolector_rol (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  int len;
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("rol %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  len = BTOR_REAL_ADDR_NODE (e0)->len;
  BTOR_ABORT_BOOLECTOR (!btor_is_power_of_2_util (len),
                        "bit-width of 'e0' must be a power of 2");
  BTOR_ABORT_BOOLECTOR (
      btor_log_2_util (len) != BTOR_REAL_ADDR_NODE (e1)->len,
      "bit-width of 'e1' must be equal to log2(bit-width of 'e0')");
  btor->external_refs++;
  res = btor_rol_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_rol, res, e0, e1);
  return res;
}

BtorNode *
boolector_ror (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  int len;
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ror %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  len = BTOR_REAL_ADDR_NODE (e0)->len;
  BTOR_ABORT_BOOLECTOR (!btor_is_power_of_2_util (len),
                        "bit-width of 'e0' must be a power of 2");
  BTOR_ABORT_BOOLECTOR (
      btor_log_2_util (len) != BTOR_REAL_ADDR_NODE (e1)->len,
      "bit-width of 'e1' must be equal to log2(bit-width of 'e0')");
  btor->external_refs++;
  res = btor_ror_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ror, res, e0, e1);
  return res;
}

BtorNode *
boolector_sub (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sub %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_sub_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sub, res, e0, e1);
  return res;
}

BtorNode *
boolector_usubo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("usubo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_usubo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_usubo, res, e0, e1);
  return res;
}

BtorNode *
boolector_ssubo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("ssubo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_ssubo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_ssubo, res, e0, e1);
  return res;
}

BtorNode *
boolector_udiv (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("udiv %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_udiv_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_udiv, res, e0, e1);
  return res;
}

BtorNode *
boolector_sdiv (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sdiv %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_sdiv_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sdiv, res, e0, e1);
  return res;
}

BtorNode *
boolector_sdivo (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sdivo %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_sdivo_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_sdivo, res, e0, e1);
  return res;
}

BtorNode *
boolector_urem (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("urem %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_urem_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_urem, res, e0, e1);
  return res;
}

BtorNode *
boolector_srem (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("srem %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_srem_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_srem, res, e0, e1);
  return res;
}

BtorNode *
boolector_smod (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("smod %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_NE_BW (e0, e1);
  btor->external_refs++;
  res = btor_smod_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_smod, res, e0, e1);
  return res;
}

BtorNode *
boolector_concat (Btor *btor, BtorNode *e0, BtorNode *e1)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("concat %p %p", e0, e1);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e0);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e1);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e0);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e1);
  e0 = btor_simplify_exp (btor, e0);
  e1 = btor_simplify_exp (btor, e1);
  BTOR_ABORT_ARRAY_BOOLECTOR (e0);
  BTOR_ABORT_ARRAY_BOOLECTOR (e1);
  BTOR_ABORT_BOOLECTOR (
      BTOR_REAL_ADDR_NODE (e0)->len > INT_MAX - BTOR_REAL_ADDR_NODE (e1)->len,
      "bit-width of result is too large");
  btor->external_refs++;
  res = btor_concat_exp (btor, e0, e1);
  BTOR_TRAPI_RETURNP (boolector_concat, res, e0, e1);
  return res;
}

BtorNode *
boolector_read (Btor *btor, BtorNode *e_array, BtorNode *e_index)
{
  BtorNode *res;

  BTOR_TRAPI ("read %p %p", e_array, e_index);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_array);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_index);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_array);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_index);
  e_array = btor_simplify_exp (btor, e_array);
  e_index = btor_simplify_exp (btor, e_index);
  BTOR_ABORT_BV_BOOLECTOR (e_array);
  BTOR_ABORT_ARRAY_BOOLECTOR (e_index);
  BTOR_ABORT_BOOLECTOR (
      e_array->index_len != BTOR_REAL_ADDR_NODE (e_index)->len,
      "index bit-width of 'e_array' and bit-width of 'e_index' must not be "
      "unequal");
  btor->external_refs++;
  res = btor_read_exp (btor, e_array, e_index);
  BTOR_TRAPI_RETURNP (boolector_read, res, e_array, e_index);
  return res;
}

BtorNode *
boolector_write (Btor *btor,
                 BtorNode *e_array,
                 BtorNode *e_index,
                 BtorNode *e_value)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("write %p %p %p", e_array, e_index, e_value);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_array);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_index);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_value);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_array);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_index);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_value);
  e_array = btor_simplify_exp (btor, e_array);
  e_index = btor_simplify_exp (btor, e_index);
  e_value = btor_simplify_exp (btor, e_value);
  BTOR_ABORT_BV_BOOLECTOR (e_array);
  BTOR_ABORT_ARRAY_BOOLECTOR (e_index);
  BTOR_ABORT_ARRAY_BOOLECTOR (e_value);
  BTOR_ABORT_BOOLECTOR (
      e_array->index_len != BTOR_REAL_ADDR_NODE (e_index)->len,
      "index bit-width of 'e_array' and bit-width of 'e_index' must not be "
      "unequal");
  BTOR_ABORT_BOOLECTOR (e_array->len != BTOR_REAL_ADDR_NODE (e_value)->len,
                        "element bit-width of 'e_array' and bit-width of "
                        "'e_value' must not be unequal");
  btor->external_refs++;
  res = btor_write_exp (btor, e_array, e_index, e_value);
  BTOR_TRAPI_RETURNP (boolector_write, res, e_array, e_index, e_value);
  return res;
}

BtorNode *
boolector_cond (Btor *btor, BtorNode *e_cond, BtorNode *e_if, BtorNode *e_else)
{
  BtorNode *real_e_if, *real_e_else, *res;
  int is_array_e_if, is_array_e_else;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("cond %p %p %p", e_cond, e_if, e_else);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_cond);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_if);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_else);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_cond);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_if);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_else);
  e_cond = btor_simplify_exp (btor, e_cond);
  e_if   = btor_simplify_exp (btor, e_if);
  e_else = btor_simplify_exp (btor, e_else);
  BTOR_ABORT_ARRAY_BOOLECTOR (e_cond);
  BTOR_ABORT_BOOLECTOR (BTOR_REAL_ADDR_NODE (e_cond)->len != 1,
                        "bit-width of 'e_cond' must be equal to 1");
  real_e_if       = BTOR_REAL_ADDR_NODE (e_if);
  real_e_else     = BTOR_REAL_ADDR_NODE (e_else);
  is_array_e_if   = BTOR_IS_ARRAY_NODE (real_e_if);
  is_array_e_else = BTOR_IS_ARRAY_NODE (real_e_else);
  BTOR_ABORT_BOOLECTOR (is_array_e_if != is_array_e_else,
                        "array must not be combined with bit-vector");
  BTOR_ABORT_BOOLECTOR (!is_array_e_if && real_e_if && real_e_else
                            && real_e_if->len != real_e_else->len,
                        "bit-vectors must not have unequal bit-width");
  BTOR_ABORT_BOOLECTOR (is_array_e_if && real_e_if && real_e_else
                            && real_e_if->len != real_e_else->len,
                        "arrays must not have unequal element bit-width");
  BTOR_ABORT_BOOLECTOR (is_array_e_if && real_e_if && real_e_else
                            && real_e_if->index_len != real_e_else->index_len,
                        "arrays must not have unequal index bit-width");
  btor->external_refs++;
  res = btor_cond_exp (btor, e_cond, e_if, e_else);
  BTOR_TRAPI_RETURNP (boolector_cond, res, e_cond, e_if, e_else);
  return res;
}

BtorNode *
boolector_lambda (Btor *btor, BtorNode *param, BtorNode *exp)
{
  // TODO TRAPI
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (param);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (param);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (!BTOR_IS_PARAM_NODE (BTOR_REAL_ADDR_NODE (param)),
                        "'param' must be a parameter");
  btor->external_refs++;
  return btor_lambda_exp (btor, param, exp);
}

BtorNode *
boolector_param (Btor *btor, int width, const char *symbol)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);

  BtorNode *res;
  char *symb;

  if ((symb = (char *) symbol) == NULL)
  {
    BTOR_NEWN (btor->mm, symb, 20);
    sprintf (symb, "DPN%d", btor->dpn_id++);
    BTOR_TRAPI ("param %d %s", width, symb);
    BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
    btor->external_refs++;
    res = btor_param_exp (btor, width, symb);
  }
  else
  {
    BTOR_TRAPI ("param %d %s", width, symbol);
    BTOR_ABORT_BOOLECTOR (width < 1, "'width' must not be < 1");
    btor->external_refs++;
    res = btor_param_exp (btor, width, symbol);
  }

  if (symbol == NULL) BTOR_DELETEN (btor->mm, symb, 20);
  BTOR_TRAPI_RETURNP (boolector_param, res, width, symbol);
  return res;
}

BtorNode *
boolector_fun (Btor *btor, int paramc, BtorNode **params, BtorNode *exp)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (params);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (paramc < 1, "'paramc' must not be < 1");

  int i, len;
  char *strtrapi;
  BtorNode *res;

  len = 5 + 10 + paramc * 20 + 20;
  BTOR_NEWN (btor->mm, strtrapi, len);
  sprintf (strtrapi, "fun %d", paramc);

  for (i = 0; i < paramc; i++)
  {
    BTOR_ABORT_BOOLECTOR (
        !params[i] || !BTOR_IS_PARAM_NODE (BTOR_REAL_ADDR_NODE (params[i])),
        "'params[%d]' is not a parameter",
        i);
    BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (params[i]);
    sprintf (strtrapi + strlen (strtrapi), " %p", params[i]);
  }
  sprintf (strtrapi + strlen (strtrapi), " %p", exp);
  BTOR_TRAPI (strtrapi);
  BTOR_DELETEN (btor->mm, strtrapi, len);
  btor->external_refs++;
  res = btor_fun_exp (btor, paramc, params, exp);
  BTOR_TRAPI_RETURNP (boolector_fun, res, paramc, params, exp);
  return res;
}

// TODO: allow partial application?
BtorNode *
boolector_apply (Btor *btor, int argc, BtorNode **args, BtorNode *fun)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (fun);
  BTOR_ABORT_BOOLECTOR (argc < 1, "'argc' must not be < 1");
  BTOR_ABORT_BOOLECTOR (argc >= 1 && !args,
                        "no arguments given but argc defined > 0");

  // TODO: get arity of function
  int i, len;
  char *strtrapi;
  BtorNode *res, *cur;

  len = 7 + 10 + argc * 20 + 20;
  BTOR_NEWN (btor->mm, strtrapi, len);
  sprintf (strtrapi, "apply %d", argc);

  cur = BTOR_REAL_ADDR_NODE (fun);
  for (i = 0; i < argc; i++)
  {
    BTOR_ABORT_BOOLECTOR (
        !BTOR_IS_LAMBDA_NODE (cur),
        "number of arguments muste be <= number of parameters in 'fun'");
    sprintf (strtrapi + strlen (strtrapi), " %p", args[i]);
    cur = BTOR_REAL_ADDR_NODE (cur->e[1]);
  }
  sprintf (strtrapi + strlen (strtrapi), " %p", fun);
  BTOR_TRAPI (strtrapi);
  BTOR_DELETEN (btor->mm, strtrapi, len);
  btor->external_refs++;
  res = btor_apply_exp (btor, argc, args, fun);
  BTOR_TRAPI_RETURNP (boolector_apply, res, argc, args, fun);
  return res;
}

BtorNode *
boolector_inc (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("inc %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);

  btor->external_refs++;
  res = btor_inc_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_inc, res, exp);
  return res;
}

BtorNode *
boolector_dec (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("dec %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);

  btor->external_refs++;
  res = btor_dec_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_dec, res, exp);
  return res;
}

int
boolector_get_width (Btor *btor, BtorNode *exp)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("get_width %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  res = btor_get_exp_len (btor, exp);
  BTOR_TRAPI_RETURN (boolector_get_width, res, exp);
  return res;
}

int
boolector_is_array (Btor *btor, BtorNode *exp)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("is_array %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  res = btor_is_array_exp (btor, exp);
  BTOR_TRAPI_RETURN (boolector_is_array, res, exp);
  return res;
}

int
boolector_is_fun (Btor *btor, BtorNode *exp)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("is_fun %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  res = btor_is_lambda_exp (btor, exp);
  BTOR_TRAPI_RETURN (boolector_is_fun, res, exp);
  return res;
}

int
boolector_get_fun_arity (Btor *btor, BtorNode *exp)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("get_fun_arity %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  res = btor_get_lambda_arity (btor, exp);
  BTOR_TRAPI_RETURN (boolector_get_fun_arity, res, exp);
  return res;
}

int
boolector_get_index_width (Btor *btor, BtorNode *e_array)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("get_index_width %p", e_array);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_array);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_array);
  e_array = btor_simplify_exp (btor, e_array);
  BTOR_ABORT_BV_BOOLECTOR (e_array);
  res = btor_get_index_exp_len (btor, e_array);
  BTOR_TRAPI_RETURN (boolector_get_index_width, res, e_array);
  return res;
}

int
boolector_fun_sort_check (Btor *btor, int argc, BtorNode **args, BtorNode *fun)
{
  // TODO TRAPI
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (fun);
  BTOR_ABORT_BOOLECTOR (argc < 1, "'argc' must not be < 1");
  BTOR_ABORT_BOOLECTOR (argc >= 1 && !args,
                        "no arguments given but argc defined > 0");
  fun = btor_simplify_exp (btor, fun);
  return btor_fun_sort_check (btor, argc, args, fun);
}

const char *
boolector_get_symbol_of_var (Btor *btor, BtorNode *exp)
{
  const char *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("get_symbol_of_var %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  res = (const char *) btor_get_symbol_exp (btor, exp);
  BTOR_TRAPI_RETURNS (boolector_get_symbol_of_var, res, exp);
  return res;
}

BtorNode *
boolector_copy (Btor *btor, BtorNode *exp)
{
  BtorNode *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("copy %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  btor->external_refs++;
  res = btor_copy_exp (btor, exp);
  BTOR_TRAPI_RETURNP (boolector_copy, res, exp);
  return res;
}

void
boolector_release (Btor *btor, BtorNode *exp)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("release %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  btor->external_refs--;
  btor_release_exp (btor, exp);
  BTOR_CHKCLONENORES (boolector_release, exp);
}

void
boolector_dump_btor (Btor *btor, FILE *file, BtorNode *exp)
{
  // TODO TRAPI
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (file);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  btor_dump_exp (btor, file, exp);
  BTOR_CHKCLONENORES (boolector_dump_btor, file, exp);
}

void
boolector_dump_smt (Btor *btor, FILE *file, BtorNode *exp)
{
  // TODO TRAPI
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (file);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  btor_dump_smt1 (btor, file, &exp, 1);
  BTOR_CHKCLONENORES (boolector_dump_smt, file, exp);
}

void
boolector_dump_smt2 (Btor *btor, FILE *file, BtorNode *exp)
{
  // TODO TRAPI
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (file);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  btor_dump_smt2 (btor, file, &exp, 1);
  BTOR_CHKCLONENORES (boolector_dump_smt2, file, exp);
}

void
boolector_assert (Btor *btor, BtorNode *exp)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("assert %p", exp);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (BTOR_REAL_ADDR_NODE (exp)->len != 1,
                        "'exp' must have bit-width one");
  btor_add_constraint_exp (btor, exp);
  BTOR_CHKCLONENORES (boolector_assert, exp);
}

void
boolector_assume (Btor *btor, BtorNode *exp)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("assume %p", exp);
  BTOR_ABORT_BOOLECTOR (!btor->inc_enabled,
                        "incremental usage has not been enabled");
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (BTOR_REAL_ADDR_NODE (exp)->len != 1,
                        "'exp' must have bit-width one");
  btor_add_assumption_exp (btor, exp);
  BTOR_CHKCLONENORES (boolector_assume, exp);
}

int
boolector_sat (Btor *btor)
{
  int res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("sat");
  BTOR_ABORT_BOOLECTOR (!btor->inc_enabled && btor->btor_sat_btor_called > 0,
                        "incremental usage has not been enabled."
                        "'boolector_sat' may only be called once");
  res = btor_sat_btor (btor);
  BTOR_TRAPI_RETURN (boolector_sat, res);
  return res;
}

char *
boolector_bv_assignment (Btor *btor, BtorNode *exp)
{
  char *res;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("bv_assignment %p", exp);
  BTOR_ABORT_BOOLECTOR (
      btor->last_sat_result != BTOR_SAT,
      "cannot retrieve assignment if input formula is not SAT");
  BTOR_ABORT_ARG_NULL_BOOLECTOR (exp);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (exp);
  exp = btor_simplify_exp (btor, exp);
  BTOR_ABORT_ARRAY_BOOLECTOR (exp);
  BTOR_ABORT_BOOLECTOR (!btor->model_gen,
                        "model generation has not been enabled");
  res = btor_bv_assignment_exp (btor, exp);
  BTOR_TRAPI_RETURNS (boolector_bv_assignment, res, exp);
  return res;
}

void
boolector_free_bv_assignment (Btor *btor, char *assignment)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("free_bv_assignment %p", assignment);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (assignment);
  btor_free_bv_assignment_exp (btor, assignment);
  BTOR_CHKCLONENORES (boolector_free_bv_assignment, assignment);
}

void
boolector_array_assignment (
    Btor *btor, BtorNode *e_array, char ***indices, char ***values, int *size)
{
  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_ABORT_BOOLECTOR (
      btor->last_sat_result != BTOR_SAT,
      "cannot retrieve assignment if input formula is not SAT");
  BTOR_ABORT_ARG_NULL_BOOLECTOR (e_array);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (indices);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (values);
  BTOR_ABORT_ARG_NULL_BOOLECTOR (size);
  BTOR_TRAPI ("array_assignment %p", e_array);
  BTOR_ABORT_REFS_NOT_POS_BOOLECTOR (e_array);
  e_array = btor_simplify_exp (btor, e_array);
  BTOR_ABORT_BV_BOOLECTOR (e_array);
  BTOR_ABORT_BOOLECTOR (!btor->model_gen,
                        "model generation has not been enabled");
  btor_array_assignment_exp (btor, e_array, indices, values, size);
  /* special case: we treat out parameters as return values for btoruntrace */
  BTOR_TRAPI ("return %p %p %d", *indices, *values, *size);
  BTOR_CHKCLONENORES (
      boolector_array_assignment, e_array, indices, values, size);
}

void
boolector_free_array_assignment (Btor *btor,
                                 char **indices,
                                 char **values,
                                 int size)
{
  int i;

  BTOR_ABORT_ARG_NULL_BOOLECTOR (btor);
  BTOR_TRAPI ("free_array_assignment %p %p %d", indices, values, size);
  BTOR_ABORT_BOOLECTOR (size < 0, "negative size");
  if (size)
  {
    BTOR_ABORT_ARG_NULL_BOOLECTOR (indices);
    BTOR_ABORT_ARG_NULL_BOOLECTOR (values);
  }
  else
  {
    BTOR_ABORT_BOOLECTOR (indices, "non zero 'indices' but 'size == 0'");
    BTOR_ABORT_BOOLECTOR (values, "non zero 'values' but 'size == 0'");
  }

  for (i = 0; i < size; i++) btor_free_bv_assignment_exp (btor, indices[i]);
  btor_free (btor->mm, indices, size * sizeof *indices);

  for (i = 0; i < size; i++) btor_free_bv_assignment_exp (btor, values[i]);
  btor_free (btor->mm, values, size * sizeof *values);
  BTOR_CHKCLONENORES (boolector_free_array_assignment, indices, values, size);
}
