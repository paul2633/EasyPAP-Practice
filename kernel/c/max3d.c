
#include "easypap.h"

#include <omp.h>

void max3d_init (void)
{
  PRINT_DEBUG ('u', "Mesh size: %d\n", NB_CELLS);
  PRINT_DEBUG ('u', "#Patches: %d\n", NB_PATCHES);
  PRINT_DEBUG ('u', "Min cell neighbors: %d\n", min_neighbors ());
  PRINT_DEBUG ('u', "Max cell neighbors: %d\n", max_neighbors ());
}

// The Mesh is a one-dimension array of cells of size NB_CELLS. Each cell value
// is of type 'float' and should be kept between 0.0 and 1.0.

int max3d_do_patch_default (int start_cell, int end_cell)
{
  int change = 0;

  for (int c = start_cell; c < end_cell; c++) {
    // TODO
  }

  return change;
}

///////////////////////////// Simple sequential version (seq)
// Suggested cmdline(s):
// ./run -lm 2-torus.cgns -k max3d -v seq
//
unsigned max3d_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {
    unsigned change = 0;

    for (int p = 0; p < NB_PATCHES; p++)
      change |= do_patch (p);

    if (!change) // we stop if all patches are stable
      return it;
  }

  return 0;
}

///////////////////////////// Initial configuration

void max3d_draw_default (void)
{
  for (int c = 0; c < NB_CELLS; c++)
    cur_data (c) = (float)c / (float)(NB_CELLS - 1);
}

void max3d_draw (char *param)
{
  // Call function ${kernel}_draw_${param}, or default function (second
  // parameter) if symbol not found
  hooks_draw_helper (param, max3d_draw_default);
}

///////////// Config

void max3d_config (char *param)
{
  if (easypap_mesh_file == NULL)
    exit_with_error ("kernel %s needs a mesh (use --load-mesh <filename>)",
                     kernel_name);

  // Choose color palette
  mesh_data_set_palette_predefined (EZV_PALETTE_RAINBOW);
}
