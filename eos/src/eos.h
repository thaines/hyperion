#ifndef EOS_H
#define EOS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file eos.h
/// Include eos.h to use the eos library, there are no other files to be
/// included. Whilst somewhat inefficient it makes life easier, and the extra
/// compilation time is forgettable with modern systems. In addition you will
/// often want to make use of the <dfn>using</dfn> keyword, i.e.
/// <dfn>using eos;</dfn> to save getting anal with namespaces.re

//------------------------------------------------------------------------------

/// \mainpage
/// \section intro Introduction
/// eos is a general purpose library, covering a fairly arbitary set of
/// functions, ranging from generic data structures to very specific algorithms.
/// Its contents is basically defined as everything general I have ever coded
/// which I expect to use in multiple programs. Specifically, projects helios
/// and selene which are the main users of this library.
///
/// \section name The Name
/// eos is the greek godess of the dawn. The name is chosen to tie in with the
/// two other projects this library is meant to serve, helios and selene. These
/// respectivly equate to the sun and the moon, therefore the library they both
/// share obviously has to be either dawn or dusk. dawn simply has a
/// conveniantly short name. (hespera being the alternative, which is neither
/// spellable nor pronouncable without effort.)
///
/// \section use Usage
/// eos is provided as a single dll, <dfn>eos.so.1.0/eos.dll</dfn>, its (for
/// windows) import library, <dfn>eos.lib</dfn> and a bunch of headers.
/// To use it include the header file <dfn>eos.h</dfn>, which automatically
/// makes the compiler link against the relevent stuff. It is structured
/// arround namespaces, so the <dfn>using</dfn> keyword should see regular
/// usage.
///
/// \section goals Design Goals
/// eos is designed to provide lots of little tools to acheive certain tasks
/// easilly and abstract from the dreary work of low level code. It is however
/// primarly meant to be fast, and does not subscribe to the logic of saving the
/// programer time by creating a slower program. A logical bases for the the
/// functionality provided barely exists, though the goals of selene and helios
/// provide much of this. It is meant to fully abstract the OS, so only the
/// library will ever need porting, not the programs it serves. In terms of
/// actual consistancy in the system little exists, as speed is allways primary.
/// Templates are however extensivly used, and most of the system complies with
/// its own stream interface.

//------------------------------------------------------------------------------
// All the include files that make up the eos library...

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/version.h"

#include "eos/mem/functions.h"
#include "eos/mem/alloc.h"
#include "eos/mem/safety.h"
#include "eos/mem/preempt.h"
#include "eos/mem/managers.h"
#include "eos/mem/packer.h"

#include "eos/io/base.h"
#include "eos/io/in.h"
#include "eos/io/out.h"
#include "eos/io/inout.h"
#include "eos/io/seekable.h"
#include "eos/io/to_virt.h"
#include "eos/io/parser.h"
#include "eos/io/counter.h"
#include "eos/io/functions.h"

#include "eos/log/logs.h"

#include "eos/bs/colours.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/geo3d.h"
#include "eos/bs/geo_algs.h"
#include "eos/bs/dom.h"

#include "eos/ds/sorting.h"
#include "eos/ds/iteration.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/ds/stacks.h"
#include "eos/ds/queues.h"
#include "eos/ds/lists.h"
#include "eos/ds/sort_lists.h"
#include "eos/ds/priority_queues.h"
#include "eos/ds/sparse_hash.h"
#include "eos/ds/dense_hash.h"
#include "eos/ds/graphs.h"
#include "eos/ds/voronoi.h"
#include "eos/ds/kd_tree.h"
#include "eos/ds/scheduling.h"
#include "eos/ds/windows.h"
#include "eos/ds/arrays_resize.h"
#include "eos/ds/arrays_ns.h"
#include "eos/ds/sparse_bit_array.h"
#include "eos/ds/falloff.h"
#include "eos/ds/nth.h"
#include "eos/ds/dialler.h"
#include "eos/ds/layered_graphs.h"
#include "eos/ds/collectors.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/math/mat_ops.h"
#include "eos/math/eigen.h"
#include "eos/math/iter_min.h"
#include "eos/math/stats.h"
#include "eos/math/complex.h"
#include "eos/math/quaternions.h"
#include "eos/math/gaussian_mix.h"
#include "eos/math/interpolation.h"
#include "eos/math/distance.h"
#include "eos/math/svd.h"
#include "eos/math/func.h"
#include "eos/math/bessel.h"
#include "eos/math/stats_dir.h"

#include "eos/time/times.h"
#include "eos/time/progress.h"
#include "eos/time/format.h"

#include "eos/data/blocks.h"
#include "eos/data/buffers.h"
#include "eos/data/giants.h"
#include "eos/data/checksums.h"
#include "eos/data/randoms.h"
#include "eos/data/property.h"

#include "eos/str/functions.h"
#include "eos/str/strings.h"
#include "eos/str/tokens.h"

#include "eos/file/dirs.h"
#include "eos/file/files.h"
#include "eos/file/dlls.h"
#include "eos/file/images.h"
#include "eos/file/wavefront.h"
#include "eos/file/xml.h"
#include "eos/file/csv.h"
#include "eos/file/stereo_helpers.h"
#include "eos/file/ply.h"
#include "eos/file/meshes.h"
#include "eos/file/exif.h"

#include "eos/svt/core.h"
#include "eos/svt/node.h"
#include "eos/svt/meta.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/svt/type.h"
#include "eos/svt/file.h"
#include "eos/svt/calculation.h"
#include "eos/svt/sample.h"

#include "eos/alg/mean_shift.h"
#include "eos/alg/fitting.h"
#include "eos/alg/bp2d.h"
#include "eos/alg/shapes.h"
#include "eos/alg/genetic.h"
#include "eos/alg/greedy_merge.h"
#include "eos/alg/solvers.h"
#include "eos/alg/nearest.h"
#include "eos/alg/multigrid.h"

#include "eos/filter/image_io.h"
#include "eos/filter/conversion.h"
#include "eos/filter/segmentation.h"
#include "eos/filter/render_segs.h"
#include "eos/filter/kernel.h"
#include "eos/filter/grad_angle.h"
#include "eos/filter/edge_confidence.h"
#include "eos/filter/synergism.h"
#include "eos/filter/seg_graph.h"
#include "eos/filter/normalise.h"
#include "eos/filter/pyramid.h"
#include "eos/filter/dog_pyramid.h"
#include "eos/filter/dir_pyramid.h"
#include "eos/filter/sift.h"
#include "eos/filter/shape_index.h"
#include "eos/filter/corner_harris.h"
#include "eos/filter/matching.h"
#include "eos/filter/mser.h"
#include "eos/filter/specular.h"
#include "eos/filter/scaling.h"
#include "eos/filter/colour_matching.h"
#include "eos/filter/grad_walk.h"
#include "eos/filter/grad_bilateral.h"
#include "eos/filter/smoothing.h"
#include "eos/filter/mscr.h"
#include "eos/filter/seg_k_mean_grid.h"

#include "eos/stereo/sad.h"
#include "eos/stereo/sad_seg_stereo.h"
#include "eos/stereo/disp_post.h"
#include "eos/stereo/visualize.h"
#include "eos/stereo/warp.h"
#include "eos/stereo/plane_seg.h"
#include "eos/stereo/layer_maker.h"
#include "eos/stereo/layer_select.h"
#include "eos/stereo/bleyer04.h"
#include "eos/stereo/simpleBP.h"
#include "eos/stereo/sfg_stereo.h"
#include "eos/stereo/orient_stereo.h"
#include "eos/stereo/dsi_ms.h"
#include "eos/stereo/surface_fit_refine.h"
#include "eos/stereo/sfs_refine.h"
#include "eos/stereo/refine_orient.h"
#include "eos/stereo/refine_norm.h"
#include "eos/stereo/dsi_ms_2.h"
#include "eos/stereo/bp_clean.h"
#include "eos/stereo/ebp.h"
#include "eos/stereo/simple.h"
#include "eos/stereo/dsr.h"
#include "eos/stereo/hebp.h"

#include "eos/mya/surfaces.h"
#include "eos/mya/ied.h"
#include "eos/mya/layers.h"
#include "eos/mya/planes.h"
#include "eos/mya/spheres.h"
#include "eos/mya/disparity.h"
#include "eos/mya/layer_score.h"
#include "eos/mya/layer_merge.h"
#include "eos/mya/layer_grow.h"
#include "eos/mya/needle_int.h"

#include "eos/rend/functions.h"
#include "eos/rend/pixels.h"
#include "eos/rend/rerender.h"
#include "eos/rend/visualise.h"
#include "eos/rend/renderer.h"
#include "eos/rend/databases.h"
#include "eos/rend/renderers.h"
#include "eos/rend/backgrounds.h"
#include "eos/rend/viewers.h"
#include "eos/rend/samplers.h"
#include "eos/rend/tone_mappers.h"
#include "eos/rend/lights.h"
#include "eos/rend/objects.h"
#include "eos/rend/materials.h"
#include "eos/rend/textures.h"
#include "eos/rend/scenes.h"

#include "eos/cam/cameras.h"
#include "eos/cam/homography.h"
#include "eos/cam/calibration.h"
#include "eos/cam/fundamental.h"
#include "eos/cam/triangulation.h"
#include "eos/cam/files.h"
#include "eos/cam/rectification.h"
#include "eos/cam/disparity_converter.h"
#include "eos/cam/resectioning.h"
#include "eos/cam/make_disp.h"
#include "eos/cam/cam_render.h"

#include "eos/gui/base.h"
#include "eos/gui/callbacks.h"
#include "eos/gui/widgets.h"
#include "eos/gui/gtk_widgets.h"

#include "eos/inf/fg_types.h"
#include "eos/inf/fg_funcs.h"
#include "eos/inf/fg_vars.h"
#include "eos/inf/factor_graphs.h"
#include "eos/inf/field_graphs.h"
#include "eos/inf/fig_variables.h"
#include "eos/inf/fig_factors.h"
#include "eos/inf/gauss_integration.h"
#include "eos/inf/model_seg.h"
#include "eos/inf/gauss_integration_hier.h"
#include "eos/inf/bin_bp_2d.h"

#include "eos/os/cameras.h"
#include "eos/os/console.h"
#include "eos/os/command.h"

#include "eos/mt/threads.h"
#include "eos/mt/locks.h"

#include "eos/sur/mesh.h"
#include "eos/sur/mesh_iter.h"
#include "eos/sur/mesh_sup.h"
#include "eos/sur/catmull_clark.h"
#include "eos/sur/intersection.h"
#include "eos/sur/subdivide.h"
#include "eos/sur/simplify.h"

#include "eos/sfs/worthington.h"
#include "eos/sfs/lambertian_fit.h"
#include "eos/sfs/lambertian_segs.h"
#include "eos/sfs/lambertian_pp.h"
#include "eos/sfs/lambertian_hough.h"
#include "eos/sfs/lambertian_segment.h"
#include "eos/sfs/sfsao_gd.h"
#include "eos/sfs/sfs_bp.h"
#include "eos/sfs/zheng.h"
#include "eos/sfs/lee.h"
#include "eos/sfs/albedo_est.h"

#include "eos/fit/disp_fish.h"
#include "eos/fit/disp_norm.h"
#include "eos/fit/light_dir.h"
#include "eos/fit/sphere_sample.h"
#include "eos/fit/light_ambient.h"
#include "eos/fit/image_sphere.h"

//------------------------------------------------------------------------------
#endif

