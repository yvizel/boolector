#include <stdio.h>
#include <stdlib.h>
#include "../../boolector.h"
#include "../../btorutil.h"

int
main (int argc, char **argv)
{
  int num_elements, i, j;
  Btor *btor;
  BtorExp *mem, *ne, *ugte, *ulte, *ult, *ugt, *temp, *read1;
  BtorExp *read2, *cond1, *cond2, *sorted, *pos, *pos_p_1;
  BtorExp *no_diff_element, *formula, *index, *old_element;
  BtorExp *num_elements_exp, *start, *top, *one, *range_index;
  BtorExp *implies;
  if (argc != 2)
  {
    printf ("Usage: ./bubblesortmem <num-elements>\n");
    return 1;
  }
  num_elements = atoi (argv[1]);
  if (num_elements <= 1)
  {
    printf ("Number of elements must be greater than one\n");
    return 1;
  }

  btor = boolector_new ();
  boolector_set_rewrite_level (btor, 0);
  one = boolector_one (btor, 32);

  mem = boolector_array (btor, 8, 32, "mem");
  /* first index */
  start            = boolector_var (btor, 32, "start");
  num_elements_exp = boolector_unsigned_int (btor, num_elements, 32);
  /* last index + 1 */
  top = boolector_add (btor, start, num_elements_exp);

  /* read at an arbitrary index inside the sequence (needed later): */
  index       = boolector_var (btor, 32, "index");
  ugte        = boolector_ugte (btor, index, start);
  ult         = boolector_ult (btor, index, top);
  range_index = boolector_and (btor, ugte, ult);
  boolector_release (btor, ugte);
  boolector_release (btor, ult);
  old_element = boolector_read (btor, mem, index);

  /* bubble sort algorithm */
  for (i = 1; i < num_elements; i++)
  {
    pos     = boolector_copy (btor, start);
    pos_p_1 = boolector_add (btor, pos, one);
    for (j = 0; j < num_elements - i; j++)
    {
      read1 = boolector_read (btor, mem, pos);
      read2 = boolector_read (btor, mem, pos_p_1);
      ugt   = boolector_ugt (btor, read1, read2);
      /* swap ? */
      cond1 = boolector_cond (btor, ugt, read2, read1);
      cond2 = boolector_cond (btor, ugt, read1, read2);
      temp  = boolector_write (btor, mem, pos, cond1);
      boolector_release (btor, mem);
      mem  = temp;
      temp = boolector_write (btor, mem, pos_p_1, cond2);
      boolector_release (btor, mem);
      mem = temp;

      boolector_release (btor, read1);
      boolector_release (btor, read2);
      boolector_release (btor, ugt);
      boolector_release (btor, cond1);
      boolector_release (btor, cond2);

      boolector_release (btor, pos);
      pos = boolector_copy (btor, pos_p_1);
      boolector_release (btor, pos_p_1);
      pos_p_1 = boolector_add (btor, pos, one);
    }
    boolector_release (btor, pos);
    boolector_release (btor, pos_p_1);
  }

  /* show that sequence is sorted */
  sorted  = boolector_true (btor);
  pos     = boolector_copy (btor, start);
  pos_p_1 = boolector_add (btor, pos, one);
  for (i = 0; i < num_elements - 1; i++)
  {
    read1 = boolector_read (btor, mem, pos);
    read2 = boolector_read (btor, mem, pos_p_1);
    ulte  = boolector_ulte (btor, read1, read2);
    temp  = boolector_and (btor, sorted, ulte);
    boolector_release (btor, sorted);
    sorted = temp;
    boolector_release (btor, read1);
    boolector_release (btor, read2);
    boolector_release (btor, ulte);

    boolector_release (btor, pos);
    pos = boolector_copy (btor, pos_p_1);
    boolector_release (btor, pos_p_1);
    pos_p_1 = boolector_add (btor, pos, one);
  }
  boolector_release (btor, pos);
  boolector_release (btor, pos_p_1);

  formula = sorted;

  /* It is not the case that there exists an element in
   * the initial sequence which does not occur in the sorted
   * sequence.*/
  no_diff_element = boolector_true (btor);
  pos             = boolector_copy (btor, start);
  for (i = 0; i < num_elements; i++)
  {
    read1 = boolector_read (btor, mem, pos);
    ne    = boolector_ne (btor, read1, old_element);
    temp  = boolector_and (btor, no_diff_element, ne);
    boolector_release (btor, no_diff_element);
    no_diff_element = temp;
    boolector_release (btor, read1);
    boolector_release (btor, ne);
    temp = boolector_add (btor, pos, one);
    boolector_release (btor, pos);
    pos = temp;
  }
  boolector_release (btor, pos);

  temp = boolector_not (btor, no_diff_element);
  boolector_release (btor, no_diff_element);
  no_diff_element = temp;

  implies = boolector_implies (btor, range_index, no_diff_element);

  temp = boolector_and (btor, formula, implies);
  boolector_release (btor, formula);
  formula = temp;

  boolector_release (btor, implies);
  boolector_release (btor, no_diff_element);
  boolector_release (btor, range_index);

  /* we negate the formula and show that it is unsatisfiable */
  temp = boolector_not (btor, formula);
  boolector_release (btor, formula);
  formula = temp;
  boolector_dump_btor (btor, stdout, formula);

  /* clean up */
  boolector_release (btor, formula);
  boolector_release (btor, old_element);
  boolector_release (btor, index);
  boolector_release (btor, mem);
  boolector_release (btor, start);
  boolector_release (btor, top);
  boolector_release (btor, num_elements_exp);
  boolector_release (btor, one);
  boolector_delete (btor);
  return 0;
}
