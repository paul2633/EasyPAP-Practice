
#include "easypap.h"

#include <omp.h>

static int debug_hud = -1;

void heat3d_config (char *param)
{
  if (easypap_mesh_file == NULL)
    exit_with_error ("kernel %s needs a mesh (use --load-mesh <filename>)",
                     kernel_name);

  // Choose color palette
  mesh_data_set_palette_predefined (EZV_PALETTE_HEAT);

  if (picking_enabled) {
    debug_hud = ezv_hud_alloc (ctx [0]);
    ezv_hud_on (ctx [0], debug_hud);
  }
}

void heat3d_debug (int cell)
{
  if (cell == -1)
    ezv_hud_set (ctx [0], debug_hud, "No selection");
  else
    ezv_hud_set (ctx [0], debug_hud, "Temp: %f", cur_data (cell));
}

void heat3d_init (void)
{
  PRINT_DEBUG ('u', "Mesh size: %d\n", NB_CELLS);
  PRINT_DEBUG ('u', "#Patches: %d\n", NB_PATCHES);
  PRINT_DEBUG ('u', "Min cell neighbors: %d\n", min_neighbors ());
  PRINT_DEBUG ('u', "Max cell neighbors: %d\n", max_neighbors ());
}

// The Mesh is a one-dimension array of cells of size NB_CELLS. Each cell value
// is of type 'float' and should be kept between 0.0 and 1.0.

int heat3d_do_patch_default (int start_cell, int end_cell)
{
  for (int c = start_cell; c < end_cell; c++) {
    for (int n = neighbor_start (c); n < neighbor_end (c); n++)
      // TODO
      ;
  }

  return 0;
}

///////////////////////////// Simple sequential version (seq)
// Suggested cmdline(s):
// ./run -lm data/mesh/1-torus.obj -k heat3d -a fifty -d d -m -p
//
unsigned heat3d_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    for (int p = 0; p < NB_PATCHES; p++)
      do_patch (p);

    swap_data ();
  }

  return 0;
}

///////////////////////////// Initial configuration

void heat3d_draw_fifty (void)
{
  for (int c = 0; c < NB_CELLS >> 1; c++)
    cur_data (c) = 1.0;
}

void heat3d_draw_alt (void)
{
  for (int p = 0; p < NB_PATCHES; p++)
    for (int c = patch_start (p); c < patch_end (p); c++)
      cur_data (c) = (p % 2 == 0) ? 1.0 : 0.0;
}

static void draw_coord (int coord)
{
  const float min_val = easypap_mesh_desc.bbox.min [coord];
  const float max_val = easypap_mesh_desc.bbox.max [coord];
  const float half    = min_val + (max_val - min_val) / 2.0f;

  for (int c = 0; c < NB_CELLS; c++) {
    bbox_t box;
    mesh3d_obj_get_bbox_of_cell (&easypap_mesh_desc, c, &box);
    float center = (box.min [coord] + box.max [coord]) / 2.0f;
    cur_data (c) = (center <= half) ? 0.0f : 1.0f;
  }
}

void heat3d_draw_center (void)
{
  const float dim_x =
      easypap_mesh_desc.bbox.max [0] - easypap_mesh_desc.bbox.min [0];
  const float dim_y =
      easypap_mesh_desc.bbox.max [1] - easypap_mesh_desc.bbox.min [1];
  const float dim_z =
      easypap_mesh_desc.bbox.max [2] - easypap_mesh_desc.bbox.min [2];
  const float center [3] = {easypap_mesh_desc.bbox.min [0] + dim_x / 2.0f,
                            easypap_mesh_desc.bbox.min [1] + dim_y / 2.0f,
                            easypap_mesh_desc.bbox.min [2] + dim_z / 2.0f};
  float radius, threshold;

  radius = MAX (dim_x, dim_y);
  radius = MAX (radius, dim_z);
  // threshold = radius / 2
  threshold = (radius / 2.0f) / 2.0f;

  for (int c = 0; c < NB_CELLS; c++) {
    bbox_t box;
    mesh3d_obj_get_bbox_of_cell (&easypap_mesh_desc, c, &box);
    float cell_center [3] = {(box.min [0] + box.max [0]) / 2.0f,
                             (box.min [1] + box.max [1]) / 2.0f,
                             (box.min [2] + box.max [2]) / 2.0f};
    float square_dist =
        (cell_center [0] - center [0]) * (cell_center [0] - center [0]) +
        (cell_center [1] - center [1]) * (cell_center [1] - center [1]) +
        (cell_center [2] - center [2]) * (cell_center [2] - center [2]);
    cur_data (c) = (square_dist <= threshold * threshold) ? 1.0f : 0.0f;
  }
}

void heat3d_draw_corners (void)
{
  const float dim_x =
      easypap_mesh_desc.bbox.max [0] - easypap_mesh_desc.bbox.min [0];
  const float dim_y =
      easypap_mesh_desc.bbox.max [1] - easypap_mesh_desc.bbox.min [1];
  const float dim_z =
      easypap_mesh_desc.bbox.max [2] - easypap_mesh_desc.bbox.min [2];
  const float corner_x [2] = {easypap_mesh_desc.bbox.min [0],
                              easypap_mesh_desc.bbox.max [0]};
  const float corner_y [2] = {easypap_mesh_desc.bbox.min [1],
                              easypap_mesh_desc.bbox.max [1]};
  const float corner_z [2] = {easypap_mesh_desc.bbox.min [2],
                              easypap_mesh_desc.bbox.max [2]};
  float radius, threshold;

  radius = MAX (dim_x, dim_y);
  radius = MAX (radius, dim_z);
  threshold = radius / 3.0f;

  for (int c = 0; c < NB_CELLS; c++) {
    bbox_t box;
    mesh3d_obj_get_bbox_of_cell (&easypap_mesh_desc, c, &box);
    float cell_center [3] = {(box.min [0] + box.max [0]) / 2.0f,
                             (box.min [1] + box.max [1]) / 2.0f,
                             (box.min [2] + box.max [2]) / 2.0f};
    float dx0             = cell_center [0] - corner_x [0];
    float dy0             = cell_center [1] - corner_y [0];
    float dz0             = cell_center [2] - corner_z [0];
    float min_square_dist = dx0 * dx0 + dy0 * dy0 + dz0 * dz0;

    for (int ix = 0; ix < 2; ix++)
      for (int iy = 0; iy < 2; iy++)
        for (int iz = 0; iz < 2; iz++) {
          float dx = cell_center [0] - corner_x [ix];
          float dy = cell_center [1] - corner_y [iy];
          float dz = cell_center [2] - corner_z [iz];
          float square_dist = dx * dx + dy * dy + dz * dz;
          if (square_dist < min_square_dist)
            min_square_dist = square_dist;
        }

    cur_data (c) = (min_square_dist <= threshold * threshold) ? 1.0f : 0.0f;
  }
}

void heat3d_draw_x (void)
{
  draw_coord (0);
}

void heat3d_draw_y (void)
{
  draw_coord (1);
}

void heat3d_draw_z (void)
{
  draw_coord (2);
}

void heat3d_draw (char *param)
{
  // Call function ${kernel}_draw_${param}, or default function (second
  // parameter) if symbol not found
  hooks_draw_helper (param, heat3d_draw_fifty);
}

///////////////////////////// naive OpenCL
#ifdef ENABLE_OPENCL

static cl_mem neighbors_buffer = 0, index_buffer = 0;

void heat3d_init_ocl_naive (void)
{
  cl_int err;

  // Array of all neighbors
  const int sizen = easypap_mesh_desc.total_neighbors * sizeof (unsigned);

  neighbors_buffer =
      clCreateBuffer (context, CL_MEM_READ_WRITE, sizen, NULL, NULL);
  if (!neighbors_buffer)
    exit_with_error ("Failed to allocate neighbor buffer");

  err =
      clEnqueueWriteBuffer (ocl_queue (0), neighbors_buffer, CL_TRUE, 0, sizen,
                            easypap_mesh_desc.neighbors, 0, NULL, NULL);
  ocl_check (err, "Failed to write to neighbor buffer");

  // indexes
  const int sizei = (NB_CELLS + 1) * sizeof (unsigned);

  index_buffer = clCreateBuffer (context, CL_MEM_READ_WRITE, sizei, NULL, NULL);
  if (!index_buffer)
    exit_with_error ("Failed to allocate index buffer");

  err = clEnqueueWriteBuffer (ocl_queue (0), index_buffer, CL_TRUE, 0, sizei,
                              easypap_mesh_desc.index_first_neighbor, 0, NULL,
                              NULL);
  ocl_check (err, "Failed to write to index buffer");
}

unsigned heat3d_compute_ocl_naive (unsigned nb_iter)
{
  size_t global [1] = {GPU_SIZE}; // global domain size for our calculation
  size_t local [1]  = {TILE};     // local domain size for our calculation
  cl_int err;

  monitoring_start (easypap_gpu_lane (0));

  for (unsigned it = 1; it <= nb_iter; it++) {

    // Set kernel arguments
    //
    err = 0;
    err |= clSetKernelArg (ocl_compute_kernel (0), 0, sizeof (cl_mem),
                           &ocl_cur_buffer (0));
    err |= clSetKernelArg (ocl_compute_kernel (0), 1, sizeof (cl_mem),
                           &ocl_next_buffer (0));
    err |= clSetKernelArg (ocl_compute_kernel (0), 2, sizeof (cl_mem),
                           &neighbors_buffer);
    err |= clSetKernelArg (ocl_compute_kernel (0), 3, sizeof (cl_mem),
                           &index_buffer);
    ocl_check (err, "Failed to set kernel arguments");

    err = clEnqueueNDRangeKernel (ocl_queue (0), ocl_compute_kernel (0), 1,
                                  NULL, global, local, 0, NULL, NULL);
    ocl_check (err, "Failed to execute kernel");

    // Swap buffers
    {
      cl_mem tmp          = ocl_cur_buffer (0);
      ocl_cur_buffer (0)  = ocl_next_buffer (0);
      ocl_next_buffer (0) = tmp;
    }
  }

  clFinish (ocl_queue (0));

  monitoring_end_tile (0, 0, NB_CELLS, 0, easypap_gpu_lane (0));

  return 0;
}

#endif