#######################
# The Eos Makefile... #
#######################

RMDIR	= rm -R -f
MKDIR	= mkdir -p



###################
# Entry points... #
###################

PLATFORM	= lin
TYPE		= release


lin_release:
	@make -j 2 PLATFORM=lin TYPE=release all

lin_relbug:
	@make -j 2 PLATFORM=lin TYPE=relbug all

lin_debug:
	@make -j 2 PLATFORM=lin TYPE=debug all

lin_docs:
	@make PLATFORM=lin docs

lin_line_count:
	@make -j 2 PLATFORM=lin line_count

win_release:
	@mingw32-make -j 2 PLATFORM=win TYPE=release all

win_relbug:
	@mingw32-make -j 2 PLATFORM=win TYPE=relbug all

win_debug:
	@mingw32-make -j 2 PLATFORM=win TYPE=debug all

win_docs:
	@mingw32-make PLATFORM=win docs

win_line_count:
	@mingw32-make -j 2 PLATFORM=win line_count


clean:
	-$(RMDIR) obj
	-$(RMDIR) bin
	-rm out.txt


all: eos prep_final
	@echo Done



######################
# Directorys used... #
######################

OUT_BASE	= bin
OUT   		= $(OUT_BASE)/$(TYPE)
OBJ_BASE	= obj
OBJ		= $(OBJ_BASE)/$(TYPE)
SRC		= src
DEP             = dep

DOC             = doc
DOC_FINAL       = $(DOC)/eos.chm

DEVIL		= $(DEP)/devil
ZLIB		= $(DEP)/zlib

PRE_COMP	= $(OUT)/eos.h.gch

INCLUDE         = -I$(SRC) -I$(DEVIL) -I$(ZLIB)\include

DIRS		= $(OBJ)/.wibble



#################################
# Compiler & linker etc setup.. #
#################################

ifeq ($(PLATFORM),win)
 EXTRA_OPT	= # No sse optimisation for windows, v2 makes a bad executable with mingw whilst v1 produces really bad output, switch on with future compilers when not broken.
endif

ifeq ($(PLATFORM),lin)
 EXTRA_OPT	= #-msse2 -mfpmath=sse # Disabled for now out of general paranoia at the running differences.
endif

COMMON_C	= -mmmx -m3dnow $(EXTRA_OPT) -ffast-math -fno-strict-aliasing -c -Wall -Wfloat-equal $(INCLUDE)
DEFINE_C	= -DEOS_DLL -DEOS_X86

ifeq ($(TYPE),release)
 C_PRE		= g++ $(COMMON_C) $(DEFINE_C) -O3 -ftracer -DEOS_ASM -DEOS_RELEASE
 C_PRE_AO	= $(C_PRE)
 C_PRE_INC	= g++ $(COMMON_C) -O3 -ftracer
 L_EXE		= g++
 L_DLL		= g++ -shared
endif

ifeq ($(TYPE),relbug)
 C_PRE		= g++ $(COMMON_C) $(DEFINE_C) -ggdb -O3 -ftracer -DEOS_DEBUG -DEOS_ASM -DEOS_LOG_BLOCKS -DEOS_LOG_MINOR
 C_PRE_AO	= $(C_PRE)
 C_PRE_INC	= g++ $(COMMON_C) -ggdb -O3 -ftracer
 L_EXE		= g++ -rdynamic -ggdb
 L_DLL		= g++ -rdynamic -shared -ggdb
endif

ifeq ($(TYPE),debug)
 C_PRE		= g++ $(COMMON_C) $(DEFINE_C) -ggdb -DEOS_DEBUG -DEOS_LOG_BLOCKS -DEOS_LOG_MINOR
 C_PRE_AO	= $(C_PRE) -O3 -ftracer
 C_PRE_INC	= g++ $(COMMON_C) -ggdb
 L_EXE		= g++ -rdynamic -ggdb
 L_DLL		= g++ -rdynamic -shared -ggdb
endif

ifeq ($(PLATFORM),win)
 C 		= $(C_PRE) -DEOS_WIN32
 C_AO 		= $(C_PRE_AO) -DEOS_WIN32
 C_INC		= $(C_PRE_INC) -DEOS_WIN32
 FINAL		= $(OUT)/eos.dll
 FINAL_NAME	= $(OUT)/libeos.a
endif

ifeq ($(PLATFORM),lin)
 C 		= $(C_PRE) -DEOS_LINUX -fpic
 C_AO 		= $(C_PRE_AO) -DEOS_LINUX -fpic
 C_INC		= $(C_PRE_INC) -DEOS_LINUX
 FINAL		= $(OUT)/libeos.so.1.0
 FINAL_NAME	= libeos.so.1
endif



###########
# eos.dll #
###########

OBJS_BASIC      = $(OBJ)/types.o $(OBJ)/version.o $(OBJ)/typestring.o
OBJS_MEMORY     = $(OBJ)/mem_functions.o $(OBJ)/mem_alloc.o $(OBJ)/mem_safety.o $(OBJ)/mem_preempt.o $(OBJ)/mem_managers.o $(OBJ)/mem_packer.o
OBJS_IO         = $(OBJ)/io_base.o $(OBJ)/io_in.o $(OBJ)/io_out.o $(OBJ)/io_inout.o $(OBJ)/io_seekable.o $(OBJ)/io_to_virt.o $(OBJ)/io_parser.o $(OBJ)/io_counter.o $(OBJ)/io_functions.o $(OBJ)/io_conversion.o
OBJS_LOG	= $(OBJ)/log_logs.o
OBJS_BS		= $(OBJ)/bs_colours.o $(OBJ)/bs_geo2d.o $(OBJ)/bs_geo3d.o $(OBJ)/bs_geo_algs.o $(OBJ)/bs_dom.o $(OBJ)/bs_luv_range.o
OBJS_DS         = $(OBJ)/ds_sorting.o $(OBJ)/ds_iteration.o $(OBJ)/ds_arrays.o $(OBJ)/ds_arrays2d.o $(OBJ)/ds_stacks.o $(OBJ)/ds_queues.o $(OBJ)/ds_lists.o $(OBJ)/ds_sort_lists.o $(OBJ)/ds_priority_queues.o $(OBJ)/ds_sparse_hash.o $(OBJ)/ds_dense_hash.o $(OBJ)/ds_graphs.o $(OBJ)/ds_voronoi.o $(OBJ)/ds_kd_tree.o $(OBJ)/ds_scheduling.o $(OBJ)/ds_windows.o $(OBJ)/ds_arrays_resize.o $(OBJ)/ds_arrays_ns.o $(OBJ)/ds_sparse_bit_array.o $(OBJ)/ds_falloff.o $(OBJ)/ds_nth.o $(OBJ)/ds_dialler.o $(OBJ)/ds_layered_graphs.o $(OBJ)/ds_collectors.o
OBJS_MATH       = $(OBJ)/math_constants.o $(OBJ)/math_functions.o $(OBJ)/math_vectors.o $(OBJ)/math_matrices.o $(OBJ)/math_mat_ops.o $(OBJ)/math_eigen.o $(OBJ)/math_iter_min.o $(OBJ)/math_stats.o $(OBJ)/math_complex.o $(OBJ)/math_quaternions.o $(OBJ)/math_gaussian_mix.o $(OBJ)/math_interpolation.o $(OBJ)/math_distance.o $(OBJ)/math_svd.o $(OBJ)/math_func.o $(OBJ)/math_bessel.o $(OBJ)/math_stats_dir.o
OBJS_TIME       = $(OBJ)/time_times.o $(OBJ)/time_progress.o $(OBJ)/time_format.o
OBJS_DATA	= $(OBJ)/data_blocks.o $(OBJ)/data_buffers.o $(OBJ)/data_giants.o $(OBJ)/data_checksums.o $(OBJ)/data_randoms.o $(OBJ)/data_property.o
OBJS_STR	= $(OBJ)/str_functions.o $(OBJ)/str_strings.o $(OBJ)/str_tokens.o $(OBJ)/str_tokenize.o
OBJS_FILE	= $(OBJ)/file_dirs.o $(OBJ)/file_files.o $(OBJ)/file_dlls.o $(OBJ)/file_images.o $(OBJ)/file_wavefront.o $(OBJ)/file_xml.o $(OBJ)/file_csv.o $(OBJ)/file_stereo_helpers.o $(OBJ)/file_ply.o $(OBJ)/file_devil_funcs.o $(OBJ)/file_meshes.o $(OBJ)/file_exif.o
OBJS_SVT	= $(OBJ)/svt_core.o $(OBJ)/svt_node.o $(OBJ)/svt_meta.o $(OBJ)/svt_var.o $(OBJ)/svt_field.o $(OBJ)/svt_type.o $(OBJ)/svt_file.o $(OBJ)/svt_calculation.o $(OBJ)/svt_sample.o
OBJS_ALG	= $(OBJ)/alg_mean_shift.o $(OBJ)/alg_fitting.o $(OBJ)/alg_bp2d.o $(OBJ)/alg_shapes.o $(OBJ)/alg_genetic.o $(OBJ)/alg_local_plane.o $(OBJ)/alg_depth_plane.o $(OBJ)/alg_greedy_merge.o $(OBJ)/alg_solvers.o $(OBJ)/alg_nearest.o $(OBJ)/alg_multigrid.o
OBJS_FILTER	= $(OBJ)/filter_image_io.o $(OBJ)/filter_conversion.o $(OBJ)/filter_segmentation.o $(OBJ)/filter_render_segs.o $(OBJ)/filter_kernel.o $(OBJ)/filter_grad_angle.o $(OBJ)/filter_edge_confidence.o $(OBJ)/filter_synergism.o $(OBJ)/filter_seg_graph.o $(OBJ)/filter_normalise.o $(OBJ)/filter_pyramid.o $(OBJ)/filter_dog_pyramid.o $(OBJ)/filter_dir_pyramid.o $(OBJ)/filter_sift.o $(OBJ)/filter_shape_index.o $(OBJ)/filter_corner_harris.o $(OBJ)/filter_matching.o $(OBJ)/filter_mser.o $(OBJ)/filter_specular.o $(OBJ)/filter_scaling.o $(OBJ)/filter_colour_matching.o $(OBJ)/filter_grad_walk.o $(OBJ)/filter_grad_bilateral.o $(OBJ)/filter_smoothing.o $(OBJ)/filter_mscr.o $(OBJ)/filter_seg_k_mean_grid.o
OBJS_STEREO	= $(OBJ)/stereo_sad.o $(OBJ)/stereo_sad_seg_stereo.o $(OBJ)/stereo_disp_post.o $(OBJ)/stereo_visualize.o $(OBJ)/stereo_warp.o $(OBJ)/stereo_plane_seg.o $(OBJ)/stereo_layer_maker.o $(OBJ)/stereo_layer_select.o $(OBJ)/stereo_bleyer04.o $(OBJ)/stereo_simpleBP.o $(OBJ)/stereo_sfg_stereo.o $(OBJ)/stereo_orient_stereo.o $(OBJ)/stereo_dsi_ms.o $(OBJ)/stereo_surface_fit_refine.o $(OBJ)/stereo_sfs_refine.o $(OBJ)/stereo_dsi.o $(OBJ)/stereo_refine_orient.o $(OBJ)/stereo_refine_norm.o $(OBJ)/stereo_dsi_ms_2.o $(OBJ)/stereo_bp_clean.o $(OBJ)/stereo_ebp.o $(OBJ)/stereo_simple.o $(OBJ)/stereo_dsr.o $(OBJ)/stereo_hebp.o $(OBJ)/stereo_diffuse_correlation.o
OBJS_MYA	= $(OBJ)/mya_surfaces.o $(OBJ)/mya_ied.o $(OBJ)/mya_layers.o $(OBJ)/mya_planes.o $(OBJ)/mya_spheres.o $(OBJ)/mya_disparity.o $(OBJ)/mya_needles.o $(OBJ)/mya_layer_score.o $(OBJ)/mya_layer_merge.o $(OBJ)/mya_layer_grow.o $(OBJ)/mya_needle_int.o
OBJS_REND	= $(OBJ)/rend_functions.o $(OBJ)/rend_pixels.o $(OBJ)/rend_rerender.o $(OBJ)/rend_visualise.o $(OBJ)/rend_renderer.o $(OBJ)/rend_databases.o $(OBJ)/rend_renderers.o $(OBJ)/rend_backgrounds.o $(OBJ)/rend_viewers.o $(OBJ)/rend_samplers.o $(OBJ)/rend_tone_mappers.o $(OBJ)/rend_lights.o $(OBJ)/rend_objects.o $(OBJ)/rend_materials.o $(OBJ)/rend_textures.o $(OBJ)/rend_scenes.o $(OBJ)/rend_graphs.o
OBJS_CAM	= $(OBJ)/cam_cameras.o $(OBJ)/cam_homography.o $(OBJ)/cam_calibration.o $(OBJ)/cam_fundamental.o $(OBJ)/cam_triangulation.o $(OBJ)/cam_files.o $(OBJ)/cam_rectification.o $(OBJ)/cam_disparity_converter.o $(OBJ)/cam_resectioning.o $(OBJ)/cam_make_disp.o $(OBJ)/cam_cam_render.o
OBJS_GUI	= $(OBJ)/gui_base.o $(OBJ)/gui_callbacks.o $(OBJ)/gui_widgets.o $(OBJ)/gui_gtk_funcs.o $(OBJ)/gui_gtk_widgets.o
OBJS_INF	= $(OBJ)/inf_fg_types.o $(OBJ)/inf_fg_funcs.o $(OBJ)/inf_fg_vars.o $(OBJ)/inf_factor_graphs.o $(OBJ)/inf_field_graphs.o $(OBJ)/inf_fig_variables.o $(OBJ)/inf_fig_factors.o $(OBJ)/inf_gauss_integration.o $(OBJ)/inf_model_seg.o $(OBJ)/inf_gauss_integration_hier.o $(OBJ)/inf_bin_bp_2d.o
OBJS_OS		= $(OBJ)/os_cameras.o $(OBJ)/os_gphoto2_funcs.o $(OBJ)/os_console.o $(OBJ)/os_command.o
OBJS_MT		= $(OBJ)/mt_threads.o $(OBJ)/mt_locks.o
OBJS_SUR	= $(OBJ)/sur_mesh.o $(OBJ)/sur_mesh_iter.o $(OBJ)/sur_mesh_sup.o $(OBJ)/sur_catmull_clark.o $(OBJ)/sur_intersection.o $(OBJ)/sur_subdivide.o $(OBJ)/sur_simplify.o
OBJS_SFS	= $(OBJ)/sfs_worthington.o $(OBJ)/sfs_lambertian_fit.o $(OBJ)/sfs_lambertian_segs.o $(OBJ)/sfs_lambertian_pp.o $(OBJ)/sfs_lambertian_hough.o $(OBJ)/sfs_lambertian_segment.o $(OBJ)/sfs_sfsao_gd.o $(OBJ)/sfs_sfs_bp.o $(OBJ)/sfs_zheng.o $(OBJ)/sfs_lee.o $(OBJ)/sfs_albedo_est.o
OBJS_FIT	= $(OBJ)/fit_disp_fish.o $(OBJ)/fit_disp_norm.o $(OBJ)/fit_light_dir.o $(OBJ)/fit_sphere_sample.o $(OBJ)/fit_light_ambient.o $(OBJ)/fit_image_sphere.o $(OBJ)/fit_disp_norm_fish.o
OBJS            = $(OBJS_BASIC) $(OBJS_MEMORY) $(OBJS_IO) $(OBJS_LOG) $(OBJS_BS) $(OBJS_DS) $(OBJS_MATH) $(OBJS_TIME) $(OBJS_DATA) $(OBJS_STR) $(OBJS_FILE) $(OBJS_SVT) $(OBJS_ALG) $(OBJS_FILTER) $(OBJS_STEREO) $(OBJS_MYA) $(OBJS_REND) $(OBJS_CAM) $(OBJS_GUI) $(OBJS_INF) $(OBJS_OS) $(OBJS_MT) $(OBJS_SUR) $(OBJS_SFS) $(OBJS_FIT)


eos: $(FINAL) # $(PRE_COMP)


ifeq ($(PLATFORM),win)
$(FINAL): $(OBJS)
	$(L_DLL) -Wl,--enable-runtime-pseudo-reloc -shared -Wl,--out-implib,$(FINAL_NAME) -o $(FINAL) $(OBJS)
endif

ifeq ($(PLATFORM),lin)
$(FINAL): $(OBJS)
	$(L_DLL) -Wl,-export-dynamic,-soname,$(FINAL_NAME) -o $(FINAL) $(OBJS) -lc -ldl -lncurses
endif


$(PRE_COMP):
	$(C_INC) -o $(PRE_COMP) $(SRC)/eos.h


$(OBJ)/types.o: $(DIRS) $(SRC)/eos/types.h $(SRC)/eos/types.cpp
	$(C) -o $(OBJ)/types.o $(SRC)/eos/types.cpp

$(OBJ)/version.o: $(DIRS) $(SRC)/eos/version.h $(SRC)/eos/version.cpp
	$(C) -o $(OBJ)/version.o $(SRC)/eos/version.cpp

$(OBJ)/typestring.o: $(DIRS) $(SRC)/eos/typestring.h $(SRC)/eos/typestring.cpp
	$(C) -o $(OBJ)/typestring.o $(SRC)/eos/typestring.cpp


$(OBJ)/mem_functions.o: $(DIRS) $(SRC)/eos/mem/functions.h $(SRC)/eos/mem/functions.cpp
	$(C) -o $(OBJ)/mem_functions.o $(SRC)/eos/mem/functions.cpp

$(OBJ)/mem_alloc.o: $(DIRS) $(SRC)/eos/mem/alloc.h $(SRC)/eos/mem/alloc.cpp
	$(C) -o $(OBJ)/mem_alloc.o $(SRC)/eos/mem/alloc.cpp

$(OBJ)/mem_safety.o: $(DIRS) $(SRC)/eos/mem/safety.h $(SRC)/eos/mem/safety.cpp
	$(C) -o $(OBJ)/mem_safety.o $(SRC)/eos/mem/safety.cpp

$(OBJ)/mem_preempt.o: $(DIRS) $(SRC)/eos/mem/preempt.h $(SRC)/eos/mem/preempt.cpp
	$(C) -o $(OBJ)/mem_preempt.o $(SRC)/eos/mem/preempt.cpp

$(OBJ)/mem_managers.o: $(DIRS) $(SRC)/eos/mem/managers.h $(SRC)/eos/mem/managers.cpp
	$(C) -o $(OBJ)/mem_managers.o $(SRC)/eos/mem/managers.cpp

$(OBJ)/mem_packer.o: $(DIRS) $(SRC)/eos/mem/packer.h $(SRC)/eos/mem/packer.cpp
	$(C) -o $(OBJ)/mem_packer.o $(SRC)/eos/mem/packer.cpp


$(OBJ)/io_base.o: $(DIRS) $(SRC)/eos/io/base.h $(SRC)/eos/io/base.cpp
	$(C) -o $(OBJ)/io_base.o $(SRC)/eos/io/base.cpp

$(OBJ)/io_in.o: $(DIRS) $(SRC)/eos/io/in.h $(SRC)/eos/io/in.cpp
	$(C) -o $(OBJ)/io_in.o $(SRC)/eos/io/in.cpp

$(OBJ)/io_out.o: $(DIRS) $(SRC)/eos/io/out.h $(SRC)/eos/io/out.cpp
	$(C) -o $(OBJ)/io_out.o $(SRC)/eos/io/out.cpp

$(OBJ)/io_inout.o: $(DIRS) $(SRC)/eos/io/inout.h $(SRC)/eos/io/inout.cpp
	$(C) -o $(OBJ)/io_inout.o $(SRC)/eos/io/inout.cpp

$(OBJ)/io_seekable.o: $(DIRS) $(SRC)/eos/io/seekable.h $(SRC)/eos/io/seekable.cpp
	$(C) -o $(OBJ)/io_seekable.o $(SRC)/eos/io/seekable.cpp

$(OBJ)/io_to_virt.o: $(DIRS) $(SRC)/eos/io/to_virt.h $(SRC)/eos/io/to_virt.cpp
	$(C) -o $(OBJ)/io_to_virt.o $(SRC)/eos/io/to_virt.cpp

$(OBJ)/io_parser.o: $(DIRS) $(SRC)/eos/io/parser.h $(SRC)/eos/io/parser.cpp
	$(C) -o $(OBJ)/io_parser.o $(SRC)/eos/io/parser.cpp

$(OBJ)/io_counter.o: $(DIRS) $(SRC)/eos/io/counter.h $(SRC)/eos/io/counter.cpp
	$(C) -o $(OBJ)/io_counter.o $(SRC)/eos/io/counter.cpp

$(OBJ)/io_functions.o: $(DIRS) $(SRC)/eos/io/functions.h $(SRC)/eos/io/functions.cpp
	$(C) -o $(OBJ)/io_functions.o $(SRC)/eos/io/functions.cpp

$(OBJ)/io_conversion.o: $(DIRS) $(SRC)/eos/io/conversion.h $(SRC)/eos/io/conversion.cpp
	$(C) -o $(OBJ)/io_conversion.o $(SRC)/eos/io/conversion.cpp


$(OBJ)/log_logs.o: $(DIRS) $(SRC)/eos/log/logs.h $(SRC)/eos/log/logs.cpp
	$(C) -o $(OBJ)/log_logs.o $(SRC)/eos/log/logs.cpp


$(OBJ)/bs_colours.o: $(DIRS) $(SRC)/eos/bs/colours.h $(SRC)/eos/bs/colours.cpp
	$(C) -o $(OBJ)/bs_colours.o $(SRC)/eos/bs/colours.cpp

$(OBJ)/bs_geo2d.o: $(DIRS) $(SRC)/eos/bs/geo2d.h $(SRC)/eos/bs/geo2d.cpp
	$(C) -o $(OBJ)/bs_geo2d.o $(SRC)/eos/bs/geo2d.cpp

$(OBJ)/bs_geo3d.o: $(DIRS) $(SRC)/eos/bs/geo3d.h $(SRC)/eos/bs/geo3d.cpp
	$(C) -o $(OBJ)/bs_geo3d.o $(SRC)/eos/bs/geo3d.cpp

$(OBJ)/bs_geo_algs.o: $(DIRS) $(SRC)/eos/bs/geo_algs.h $(SRC)/eos/bs/geo_algs.cpp
	$(C) -o $(OBJ)/bs_geo_algs.o $(SRC)/eos/bs/geo_algs.cpp

$(OBJ)/bs_dom.o: $(DIRS) $(SRC)/eos/bs/dom.h $(SRC)/eos/bs/dom.cpp
	$(C) -o $(OBJ)/bs_dom.o $(SRC)/eos/bs/dom.cpp

$(OBJ)/bs_luv_range.o: $(DIRS) $(SRC)/eos/bs/luv_range.h $(SRC)/eos/bs/luv_range.cpp
	$(C) -o $(OBJ)/bs_luv_range.o $(SRC)/eos/bs/luv_range.cpp


$(OBJ)/ds_sorting.o: $(DIRS) $(SRC)/eos/ds/sorting.h $(SRC)/eos/ds/sorting.cpp
	$(C) -o $(OBJ)/ds_sorting.o $(SRC)/eos/ds/sorting.cpp

$(OBJ)/ds_iteration.o: $(DIRS) $(SRC)/eos/ds/iteration.h $(SRC)/eos/ds/iteration.cpp
	$(C) -o $(OBJ)/ds_iteration.o $(SRC)/eos/ds/iteration.cpp

$(OBJ)/ds_arrays.o: $(DIRS) $(SRC)/eos/ds/arrays.h $(SRC)/eos/ds/arrays.cpp
	$(C) -o $(OBJ)/ds_arrays.o $(SRC)/eos/ds/arrays.cpp

$(OBJ)/ds_arrays2d.o: $(DIRS) $(SRC)/eos/ds/arrays2d.h $(SRC)/eos/ds/arrays2d.cpp
	$(C) -o $(OBJ)/ds_arrays2d.o $(SRC)/eos/ds/arrays2d.cpp

$(OBJ)/ds_stacks.o: $(DIRS) $(SRC)/eos/ds/stacks.h $(SRC)/eos/ds/stacks.cpp
	$(C) -o $(OBJ)/ds_stacks.o $(SRC)/eos/ds/stacks.cpp

$(OBJ)/ds_queues.o: $(DIRS) $(SRC)/eos/ds/queues.h $(SRC)/eos/ds/queues.cpp
	$(C) -o $(OBJ)/ds_queues.o $(SRC)/eos/ds/queues.cpp

$(OBJ)/ds_lists.o: $(DIRS) $(SRC)/eos/ds/lists.h $(SRC)/eos/ds/lists.cpp
	$(C) -o $(OBJ)/ds_lists.o $(SRC)/eos/ds/lists.cpp

$(OBJ)/ds_sort_lists.o: $(DIRS) $(SRC)/eos/ds/sort_lists.h $(SRC)/eos/ds/sort_lists.cpp
	$(C) -o $(OBJ)/ds_sort_lists.o $(SRC)/eos/ds/sort_lists.cpp

$(OBJ)/ds_priority_queues.o: $(DIRS) $(SRC)/eos/ds/priority_queues.h $(SRC)/eos/ds/priority_queues.cpp
	$(C) -o $(OBJ)/ds_priority_queues.o $(SRC)/eos/ds/priority_queues.cpp

$(OBJ)/ds_sparse_hash.o: $(DIRS) $(SRC)/eos/ds/sparse_hash.h $(SRC)/eos/ds/sparse_hash.cpp
	$(C) -o $(OBJ)/ds_sparse_hash.o $(SRC)/eos/ds/sparse_hash.cpp

$(OBJ)/ds_dense_hash.o: $(DIRS) $(SRC)/eos/ds/dense_hash.h $(SRC)/eos/ds/dense_hash.cpp
	$(C) -o $(OBJ)/ds_dense_hash.o $(SRC)/eos/ds/dense_hash.cpp

$(OBJ)/ds_graphs.o: $(DIRS) $(SRC)/eos/ds/graphs.h $(SRC)/eos/ds/graphs.cpp
	$(C) -o $(OBJ)/ds_graphs.o $(SRC)/eos/ds/graphs.cpp

$(OBJ)/ds_voronoi.o: $(DIRS) $(SRC)/eos/ds/voronoi.h $(SRC)/eos/ds/voronoi.cpp
	$(C) -o $(OBJ)/ds_voronoi.o $(SRC)/eos/ds/voronoi.cpp

$(OBJ)/ds_kd_tree.o: $(DIRS) $(SRC)/eos/ds/kd_tree.h $(SRC)/eos/ds/kd_tree.cpp
	$(C) -o $(OBJ)/ds_kd_tree.o $(SRC)/eos/ds/kd_tree.cpp

$(OBJ)/ds_scheduling.o: $(DIRS) $(SRC)/eos/ds/scheduling.h $(SRC)/eos/ds/scheduling.cpp
	$(C) -o $(OBJ)/ds_scheduling.o $(SRC)/eos/ds/scheduling.cpp

$(OBJ)/ds_windows.o: $(DIRS) $(SRC)/eos/ds/windows.h $(SRC)/eos/ds/windows.cpp
	$(C) -o $(OBJ)/ds_windows.o $(SRC)/eos/ds/windows.cpp

$(OBJ)/ds_arrays_resize.o: $(DIRS) $(SRC)/eos/ds/arrays_resize.h $(SRC)/eos/ds/arrays_resize.cpp
	$(C) -o $(OBJ)/ds_arrays_resize.o $(SRC)/eos/ds/arrays_resize.cpp

$(OBJ)/ds_arrays_ns.o: $(DIRS) $(SRC)/eos/ds/arrays_ns.h $(SRC)/eos/ds/arrays_ns.cpp
	$(C) -o $(OBJ)/ds_arrays_ns.o $(SRC)/eos/ds/arrays_ns.cpp

$(OBJ)/ds_sparse_bit_array.o: $(DIRS) $(SRC)/eos/ds/sparse_bit_array.h $(SRC)/eos/ds/sparse_bit_array.cpp
	$(C) -o $(OBJ)/ds_sparse_bit_array.o $(SRC)/eos/ds/sparse_bit_array.cpp

$(OBJ)/ds_falloff.o: $(DIRS) $(SRC)/eos/ds/falloff.h $(SRC)/eos/ds/falloff.cpp
	$(C) -o $(OBJ)/ds_falloff.o $(SRC)/eos/ds/falloff.cpp

$(OBJ)/ds_nth.o: $(DIRS) $(SRC)/eos/ds/nth.h $(SRC)/eos/ds/nth.cpp
	$(C) -o $(OBJ)/ds_nth.o $(SRC)/eos/ds/nth.cpp

$(OBJ)/ds_dialler.o: $(DIRS) $(SRC)/eos/ds/dialler.h $(SRC)/eos/ds/dialler.cpp
	$(C) -o $(OBJ)/ds_dialler.o $(SRC)/eos/ds/dialler.cpp

$(OBJ)/ds_layered_graphs.o: $(DIRS) $(SRC)/eos/ds/layered_graphs.h $(SRC)/eos/ds/layered_graphs.cpp
	$(C) -o $(OBJ)/ds_layered_graphs.o $(SRC)/eos/ds/layered_graphs.cpp

$(OBJ)/ds_collectors.o: $(DIRS) $(SRC)/eos/ds/collectors.h $(SRC)/eos/ds/collectors.cpp
	$(C) -o $(OBJ)/ds_collectors.o $(SRC)/eos/ds/collectors.cpp


$(OBJ)/math_constants.o: $(DIRS) $(SRC)/eos/math/constants.h $(SRC)/eos/math/constants.cpp
	$(C) -o $(OBJ)/math_constants.o $(SRC)/eos/math/constants.cpp

$(OBJ)/math_functions.o: $(DIRS) $(SRC)/eos/math/functions.h $(SRC)/eos/math/functions.cpp
	$(C) -o $(OBJ)/math_functions.o $(SRC)/eos/math/functions.cpp

$(OBJ)/math_vectors.o: $(DIRS) $(SRC)/eos/math/vectors.h $(SRC)/eos/math/vectors.cpp
	$(C) -o $(OBJ)/math_vectors.o $(SRC)/eos/math/vectors.cpp

$(OBJ)/math_matrices.o: $(DIRS) $(SRC)/eos/math/matrices.h $(SRC)/eos/math/matrices.cpp
	$(C) -o $(OBJ)/math_matrices.o $(SRC)/eos/math/matrices.cpp

$(OBJ)/math_mat_ops.o: $(DIRS) $(SRC)/eos/math/mat_ops.h $(SRC)/eos/math/mat_ops.cpp
	$(C) -o $(OBJ)/math_mat_ops.o $(SRC)/eos/math/mat_ops.cpp

$(OBJ)/math_eigen.o: $(DIRS) $(SRC)/eos/math/eigen.h $(SRC)/eos/math/eigen.cpp
	$(C) -o $(OBJ)/math_eigen.o $(SRC)/eos/math/eigen.cpp

$(OBJ)/math_iter_min.o: $(DIRS) $(SRC)/eos/math/iter_min.h $(SRC)/eos/math/iter_min.cpp
	$(C) -o $(OBJ)/math_iter_min.o $(SRC)/eos/math/iter_min.cpp

$(OBJ)/math_stats.o: $(DIRS) $(SRC)/eos/math/stats.h $(SRC)/eos/math/stats.cpp
	$(C) -o $(OBJ)/math_stats.o $(SRC)/eos/math/stats.cpp

$(OBJ)/math_complex.o: $(DIRS) $(SRC)/eos/math/complex.h $(SRC)/eos/math/complex.cpp
	$(C) -o $(OBJ)/math_complex.o $(SRC)/eos/math/complex.cpp

$(OBJ)/math_quaternions.o: $(DIRS) $(SRC)/eos/math/quaternions.h $(SRC)/eos/math/quaternions.cpp
	$(C) -o $(OBJ)/math_quaternions.o $(SRC)/eos/math/quaternions.cpp

$(OBJ)/math_gaussian_mix.o: $(DIRS) $(SRC)/eos/math/gaussian_mix.h $(SRC)/eos/math/gaussian_mix.cpp
	$(C) -o $(OBJ)/math_gaussian_mix.o $(SRC)/eos/math/gaussian_mix.cpp

$(OBJ)/math_interpolation.o: $(DIRS) $(SRC)/eos/math/interpolation.h $(SRC)/eos/math/interpolation.cpp
	$(C) -o $(OBJ)/math_interpolation.o $(SRC)/eos/math/interpolation.cpp

$(OBJ)/math_distance.o: $(DIRS) $(SRC)/eos/math/distance.h $(SRC)/eos/math/distance.cpp
	$(C) -o $(OBJ)/math_distance.o $(SRC)/eos/math/distance.cpp

$(OBJ)/math_svd.o: $(DIRS) $(SRC)/eos/math/svd.h $(SRC)/eos/math/svd.cpp
	$(C) -o $(OBJ)/math_svd.o $(SRC)/eos/math/svd.cpp

$(OBJ)/math_func.o: $(DIRS) $(SRC)/eos/math/func.h $(SRC)/eos/math/func.cpp
	$(C) -o $(OBJ)/math_func.o $(SRC)/eos/math/func.cpp

$(OBJ)/math_bessel.o: $(DIRS) $(SRC)/eos/math/bessel.h $(SRC)/eos/math/bessel.cpp
	$(C) -o $(OBJ)/math_bessel.o $(SRC)/eos/math/bessel.cpp

$(OBJ)/math_stats_dir.o: $(DIRS) $(SRC)/eos/math/stats_dir.h $(SRC)/eos/math/stats_dir.cpp
	$(C) -o $(OBJ)/math_stats_dir.o $(SRC)/eos/math/stats_dir.cpp


$(OBJ)/time_times.o: $(DIRS) $(SRC)/eos/time/times.h $(SRC)/eos/time/times.cpp
	$(C) -o $(OBJ)/time_times.o $(SRC)/eos/time/times.cpp

$(OBJ)/time_progress.o: $(DIRS) $(SRC)/eos/time/progress.h $(SRC)/eos/time/progress.cpp
	$(C) -o $(OBJ)/time_progress.o $(SRC)/eos/time/progress.cpp

$(OBJ)/time_format.o: $(DIRS) $(SRC)/eos/time/format.h $(SRC)/eos/time/format.cpp
	$(C) -o $(OBJ)/time_format.o $(SRC)/eos/time/format.cpp


$(OBJ)/data_blocks.o: $(DIRS) $(SRC)/eos/data/blocks.h $(SRC)/eos/data/blocks.cpp
	$(C) -o $(OBJ)/data_blocks.o $(SRC)/eos/data/blocks.cpp

$(OBJ)/data_buffers.o: $(DIRS) $(SRC)/eos/data/buffers.h $(SRC)/eos/data/buffers.cpp
	$(C) -o $(OBJ)/data_buffers.o $(SRC)/eos/data/buffers.cpp

$(OBJ)/data_giants.o: $(DIRS) $(SRC)/eos/data/giants.h $(SRC)/eos/data/giants.cpp
	$(C) -o $(OBJ)/data_giants.o $(SRC)/eos/data/giants.cpp

$(OBJ)/data_checksums.o: $(DIRS) $(SRC)/eos/data/checksums.h $(SRC)/eos/data/checksums.cpp
	$(C) -o $(OBJ)/data_checksums.o $(SRC)/eos/data/checksums.cpp

$(OBJ)/data_randoms.o: $(DIRS) $(SRC)/eos/data/randoms.h $(SRC)/eos/data/randoms.cpp
	$(C) -o $(OBJ)/data_randoms.o $(SRC)/eos/data/randoms.cpp

$(OBJ)/data_property.o: $(DIRS) $(SRC)/eos/data/property.h $(SRC)/eos/data/property.cpp
	$(C) -o $(OBJ)/data_property.o $(SRC)/eos/data/property.cpp


$(OBJ)/str_functions.o: $(DIRS) $(SRC)/eos/str/functions.h $(SRC)/eos/str/functions.cpp
	$(C) -o $(OBJ)/str_functions.o $(SRC)/eos/str/functions.cpp

$(OBJ)/str_strings.o: $(DIRS) $(SRC)/eos/str/strings.h $(SRC)/eos/str/strings.cpp
	$(C) -o $(OBJ)/str_strings.o $(SRC)/eos/str/strings.cpp

$(OBJ)/str_tokens.o: $(DIRS) $(SRC)/eos/str/tokens.h $(SRC)/eos/str/tokens.cpp
	$(C) -o $(OBJ)/str_tokens.o $(SRC)/eos/str/tokens.cpp

$(OBJ)/str_tokenize.o: $(DIRS) $(SRC)/eos/str/tokenize.h $(SRC)/eos/str/tokenize.cpp
	$(C) -o $(OBJ)/str_tokenize.o $(SRC)/eos/str/tokenize.cpp


$(OBJ)/file_dirs.o: $(DIRS) $(SRC)/eos/file/dirs.h $(SRC)/eos/file/dirs.cpp
	$(C) -o $(OBJ)/file_dirs.o $(SRC)/eos/file/dirs.cpp

$(OBJ)/file_files.o: $(DIRS) $(SRC)/eos/file/files.h $(SRC)/eos/file/files.cpp
	$(C) -o $(OBJ)/file_files.o $(SRC)/eos/file/files.cpp

$(OBJ)/file_dlls.o: $(DIRS) $(SRC)/eos/file/dlls.h $(SRC)/eos/file/dlls.cpp
	$(C) -o $(OBJ)/file_dlls.o $(SRC)/eos/file/dlls.cpp

$(OBJ)/file_images.o: $(DIRS) $(SRC)/eos/file/images.h $(SRC)/eos/file/images.cpp
	$(C) -o $(OBJ)/file_images.o $(SRC)/eos/file/images.cpp

$(OBJ)/file_wavefront.o: $(DIRS) $(SRC)/eos/file/wavefront.h $(SRC)/eos/file/wavefront.cpp
	$(C) -o $(OBJ)/file_wavefront.o $(SRC)/eos/file/wavefront.cpp

$(OBJ)/file_xml.o: $(DIRS) $(SRC)/eos/file/xml.h $(SRC)/eos/file/xml.cpp
	$(C) -o $(OBJ)/file_xml.o $(SRC)/eos/file/xml.cpp

$(OBJ)/file_csv.o: $(DIRS) $(SRC)/eos/file/csv.h $(SRC)/eos/file/csv.cpp
	$(C) -o $(OBJ)/file_csv.o $(SRC)/eos/file/csv.cpp

$(OBJ)/file_stereo_helpers.o: $(DIRS) $(SRC)/eos/file/stereo_helpers.h $(SRC)/eos/file/stereo_helpers.cpp
	$(C) -o $(OBJ)/file_stereo_helpers.o $(SRC)/eos/file/stereo_helpers.cpp

$(OBJ)/file_ply.o: $(DIRS) $(SRC)/eos/file/ply.h $(SRC)/eos/file/ply.cpp
	$(C) -o $(OBJ)/file_ply.o $(SRC)/eos/file/ply.cpp

$(OBJ)/file_devil_funcs.o: $(DIRS) $(SRC)/eos/file/devil_funcs.h $(SRC)/eos/file/devil_funcs.cpp
	$(C) -o $(OBJ)/file_devil_funcs.o $(SRC)/eos/file/devil_funcs.cpp

$(OBJ)/file_meshes.o: $(DIRS) $(SRC)/eos/file/meshes.h $(SRC)/eos/file/meshes.cpp
	$(C) -o $(OBJ)/file_meshes.o $(SRC)/eos/file/meshes.cpp

$(OBJ)/file_exif.o: $(DIRS) $(SRC)/eos/file/exif.h $(SRC)/eos/file/exif.cpp
	$(C) -o $(OBJ)/file_exif.o $(SRC)/eos/file/exif.cpp


$(OBJ)/svt_core.o: $(DIRS) $(SRC)/eos/svt/core.h $(SRC)/eos/svt/core.cpp
	$(C) -o $(OBJ)/svt_core.o $(SRC)/eos/svt/core.cpp

$(OBJ)/svt_node.o: $(DIRS) $(SRC)/eos/svt/node.h $(SRC)/eos/svt/node.cpp
	$(C) -o $(OBJ)/svt_node.o $(SRC)/eos/svt/node.cpp

$(OBJ)/svt_meta.o: $(DIRS) $(SRC)/eos/svt/meta.h $(SRC)/eos/svt/meta.cpp
	$(C) -o $(OBJ)/svt_meta.o $(SRC)/eos/svt/meta.cpp

$(OBJ)/svt_var.o: $(DIRS) $(SRC)/eos/svt/var.h $(SRC)/eos/svt/var.cpp
	$(C) -o $(OBJ)/svt_var.o $(SRC)/eos/svt/var.cpp

$(OBJ)/svt_field.o: $(DIRS) $(SRC)/eos/svt/field.h $(SRC)/eos/svt/field.cpp
	$(C) -o $(OBJ)/svt_field.o $(SRC)/eos/svt/field.cpp

$(OBJ)/svt_type.o: $(DIRS) $(SRC)/eos/svt/type.h $(SRC)/eos/svt/type.cpp
	$(C) -o $(OBJ)/svt_type.o $(SRC)/eos/svt/type.cpp

$(OBJ)/svt_file.o: $(DIRS) $(SRC)/eos/svt/file.h $(SRC)/eos/svt/file.cpp
	$(C) -o $(OBJ)/svt_file.o $(SRC)/eos/svt/file.cpp

$(OBJ)/svt_calculation.o: $(DIRS) $(SRC)/eos/svt/calculation.h $(SRC)/eos/svt/calculation.cpp
	$(C) -o $(OBJ)/svt_calculation.o $(SRC)/eos/svt/calculation.cpp

$(OBJ)/svt_sample.o: $(DIRS) $(SRC)/eos/svt/sample.h $(SRC)/eos/svt/sample.cpp
	$(C) -o $(OBJ)/svt_sample.o $(SRC)/eos/svt/sample.cpp


$(OBJ)/alg_mean_shift.o: $(DIRS) $(SRC)/eos/alg/mean_shift.h $(SRC)/eos/alg/mean_shift.cpp
	$(C) -o $(OBJ)/alg_mean_shift.o $(SRC)/eos/alg/mean_shift.cpp

$(OBJ)/alg_fitting.o: $(DIRS) $(SRC)/eos/alg/fitting.h $(SRC)/eos/alg/fitting.cpp
	$(C) -o $(OBJ)/alg_fitting.o $(SRC)/eos/alg/fitting.cpp

$(OBJ)/alg_bp2d.o: $(DIRS) $(SRC)/eos/alg/bp2d.h $(SRC)/eos/alg/bp2d.cpp
	$(C) -o $(OBJ)/alg_bp2d.o $(SRC)/eos/alg/bp2d.cpp

$(OBJ)/alg_shapes.o: $(DIRS) $(SRC)/eos/alg/shapes.h $(SRC)/eos/alg/shapes.cpp
	$(C) -o $(OBJ)/alg_shapes.o $(SRC)/eos/alg/shapes.cpp

$(OBJ)/alg_genetic.o: $(DIRS) $(SRC)/eos/alg/genetic.h $(SRC)/eos/alg/genetic.cpp
	$(C) -o $(OBJ)/alg_genetic.o $(SRC)/eos/alg/genetic.cpp

$(OBJ)/alg_local_plane.o: $(DIRS) $(SRC)/eos/alg/local_plane.h $(SRC)/eos/alg/local_plane.cpp
	$(C) -o $(OBJ)/alg_local_plane.o $(SRC)/eos/alg/local_plane.cpp

$(OBJ)/alg_depth_plane.o: $(DIRS) $(SRC)/eos/alg/depth_plane.h $(SRC)/eos/alg/depth_plane.cpp
	$(C) -o $(OBJ)/alg_depth_plane.o $(SRC)/eos/alg/depth_plane.cpp

$(OBJ)/alg_greedy_merge.o: $(DIRS) $(SRC)/eos/alg/greedy_merge.h $(SRC)/eos/alg/greedy_merge.cpp
	$(C) -o $(OBJ)/alg_greedy_merge.o $(SRC)/eos/alg/greedy_merge.cpp

$(OBJ)/alg_solvers.o: $(DIRS) $(SRC)/eos/alg/solvers.h $(SRC)/eos/alg/solvers.cpp
	$(C) -o $(OBJ)/alg_solvers.o $(SRC)/eos/alg/solvers.cpp

$(OBJ)/alg_nearest.o: $(DIRS) $(SRC)/eos/alg/nearest.h $(SRC)/eos/alg/nearest.cpp
	$(C) -o $(OBJ)/alg_nearest.o $(SRC)/eos/alg/nearest.cpp

$(OBJ)/alg_multigrid.o: $(DIRS) $(SRC)/eos/alg/multigrid.h $(SRC)/eos/alg/multigrid.cpp
	$(C) -o $(OBJ)/alg_multigrid.o $(SRC)/eos/alg/multigrid.cpp


$(OBJ)/filter_image_io.o: $(DIRS) $(SRC)/eos/filter/image_io.h $(SRC)/eos/filter/image_io.cpp
	$(C) -o $(OBJ)/filter_image_io.o $(SRC)/eos/filter/image_io.cpp

$(OBJ)/filter_conversion.o: $(DIRS) $(SRC)/eos/filter/conversion.h $(SRC)/eos/filter/conversion.cpp
	$(C) -o $(OBJ)/filter_conversion.o $(SRC)/eos/filter/conversion.cpp

$(OBJ)/filter_segmentation.o: $(DIRS) $(SRC)/eos/filter/segmentation.h $(SRC)/eos/filter/segmentation.cpp
	$(C) -o $(OBJ)/filter_segmentation.o $(SRC)/eos/filter/segmentation.cpp

$(OBJ)/filter_render_segs.o: $(DIRS) $(SRC)/eos/filter/render_segs.h $(SRC)/eos/filter/render_segs.cpp
	$(C) -o $(OBJ)/filter_render_segs.o $(SRC)/eos/filter/render_segs.cpp

$(OBJ)/filter_kernel.o: $(DIRS) $(SRC)/eos/filter/kernel.h $(SRC)/eos/filter/kernel.cpp
	$(C) -o $(OBJ)/filter_kernel.o $(SRC)/eos/filter/kernel.cpp

$(OBJ)/filter_grad_angle.o: $(DIRS) $(SRC)/eos/filter/grad_angle.h $(SRC)/eos/filter/grad_angle.cpp
	$(C) -o $(OBJ)/filter_grad_angle.o $(SRC)/eos/filter/grad_angle.cpp

$(OBJ)/filter_edge_confidence.o: $(DIRS) $(SRC)/eos/filter/edge_confidence.h $(SRC)/eos/filter/edge_confidence.cpp
	$(C) -o $(OBJ)/filter_edge_confidence.o $(SRC)/eos/filter/edge_confidence.cpp

$(OBJ)/filter_synergism.o: $(DIRS) $(SRC)/eos/filter/synergism.h $(SRC)/eos/filter/synergism.cpp
	$(C) -o $(OBJ)/filter_synergism.o $(SRC)/eos/filter/synergism.cpp

$(OBJ)/filter_seg_graph.o: $(DIRS) $(SRC)/eos/filter/seg_graph.h $(SRC)/eos/filter/seg_graph.cpp
	$(C) -o $(OBJ)/filter_seg_graph.o $(SRC)/eos/filter/seg_graph.cpp

$(OBJ)/filter_normalise.o: $(DIRS) $(SRC)/eos/filter/normalise.h $(SRC)/eos/filter/normalise.cpp
	$(C) -o $(OBJ)/filter_normalise.o $(SRC)/eos/filter/normalise.cpp

$(OBJ)/filter_pyramid.o: $(DIRS) $(SRC)/eos/filter/pyramid.h $(SRC)/eos/filter/pyramid.cpp
	$(C) -o $(OBJ)/filter_pyramid.o $(SRC)/eos/filter/pyramid.cpp

$(OBJ)/filter_dog_pyramid.o: $(DIRS) $(SRC)/eos/filter/dog_pyramid.h $(SRC)/eos/filter/dog_pyramid.cpp
	$(C) -o $(OBJ)/filter_dog_pyramid.o $(SRC)/eos/filter/dog_pyramid.cpp

$(OBJ)/filter_dir_pyramid.o: $(DIRS) $(SRC)/eos/filter/dir_pyramid.h $(SRC)/eos/filter/dir_pyramid.cpp
	$(C) -o $(OBJ)/filter_dir_pyramid.o $(SRC)/eos/filter/dir_pyramid.cpp

$(OBJ)/filter_sift.o: $(DIRS) $(SRC)/eos/filter/sift.h $(SRC)/eos/filter/sift.cpp
	$(C) -o $(OBJ)/filter_sift.o $(SRC)/eos/filter/sift.cpp

$(OBJ)/filter_shape_index.o: $(DIRS) $(SRC)/eos/filter/shape_index.h $(SRC)/eos/filter/shape_index.cpp
	$(C) -o $(OBJ)/filter_shape_index.o $(SRC)/eos/filter/shape_index.cpp

$(OBJ)/filter_corner_harris.o: $(DIRS) $(SRC)/eos/filter/corner_harris.h $(SRC)/eos/filter/corner_harris.cpp
	$(C) -o $(OBJ)/filter_corner_harris.o $(SRC)/eos/filter/corner_harris.cpp

$(OBJ)/filter_matching.o: $(DIRS) $(SRC)/eos/filter/matching.h $(SRC)/eos/filter/matching.cpp
	$(C) -o $(OBJ)/filter_matching.o $(SRC)/eos/filter/matching.cpp

$(OBJ)/filter_mser.o: $(DIRS) $(SRC)/eos/filter/mser.h $(SRC)/eos/filter/mser.cpp
	$(C) -o $(OBJ)/filter_mser.o $(SRC)/eos/filter/mser.cpp

$(OBJ)/filter_specular.o: $(DIRS) $(SRC)/eos/filter/specular.h $(SRC)/eos/filter/specular.cpp
	$(C) -o $(OBJ)/filter_specular.o $(SRC)/eos/filter/specular.cpp

$(OBJ)/filter_scaling.o: $(DIRS) $(SRC)/eos/filter/scaling.h $(SRC)/eos/filter/scaling.cpp
	$(C) -o $(OBJ)/filter_scaling.o $(SRC)/eos/filter/scaling.cpp

$(OBJ)/filter_colour_matching.o: $(DIRS) $(SRC)/eos/filter/colour_matching.h $(SRC)/eos/filter/colour_matching.cpp
	$(C) -o $(OBJ)/filter_colour_matching.o $(SRC)/eos/filter/colour_matching.cpp

$(OBJ)/filter_grad_walk.o: $(DIRS) $(SRC)/eos/filter/grad_walk.h $(SRC)/eos/filter/grad_walk.cpp
	$(C) -o $(OBJ)/filter_grad_walk.o $(SRC)/eos/filter/grad_walk.cpp

$(OBJ)/filter_grad_bilateral.o: $(DIRS) $(SRC)/eos/filter/grad_bilateral.h $(SRC)/eos/filter/grad_bilateral.cpp
	$(C) -o $(OBJ)/filter_grad_bilateral.o $(SRC)/eos/filter/grad_bilateral.cpp

$(OBJ)/filter_smoothing.o: $(DIRS) $(SRC)/eos/filter/smoothing.h $(SRC)/eos/filter/smoothing.cpp
	$(C) -o $(OBJ)/filter_smoothing.o $(SRC)/eos/filter/smoothing.cpp

$(OBJ)/filter_mscr.o: $(DIRS) $(SRC)/eos/filter/mscr.h $(SRC)/eos/filter/mscr.cpp
	$(C) -o $(OBJ)/filter_mscr.o $(SRC)/eos/filter/mscr.cpp

$(OBJ)/filter_seg_k_mean_grid.o: $(DIRS) $(SRC)/eos/filter/seg_k_mean_grid.h $(SRC)/eos/filter/seg_k_mean_grid.cpp
	$(C) -o $(OBJ)/filter_seg_k_mean_grid.o $(SRC)/eos/filter/seg_k_mean_grid.cpp


$(OBJ)/stereo_sad.o: $(DIRS) $(SRC)/eos/stereo/sad.h $(SRC)/eos/stereo/sad.cpp
	$(C) -o $(OBJ)/stereo_sad.o $(SRC)/eos/stereo/sad.cpp

$(OBJ)/stereo_sad_seg_stereo.o: $(DIRS) $(SRC)/eos/stereo/sad_seg_stereo.h $(SRC)/eos/stereo/sad_seg_stereo.cpp
	$(C) -o $(OBJ)/stereo_sad_seg_stereo.o $(SRC)/eos/stereo/sad_seg_stereo.cpp

$(OBJ)/stereo_disp_post.o: $(DIRS) $(SRC)/eos/stereo/disp_post.h $(SRC)/eos/stereo/disp_post.cpp
	$(C) -o $(OBJ)/stereo_disp_post.o $(SRC)/eos/stereo/disp_post.cpp

$(OBJ)/stereo_visualize.o: $(DIRS) $(SRC)/eos/stereo/visualize.h $(SRC)/eos/stereo/visualize.cpp
	$(C) -o $(OBJ)/stereo_visualize.o $(SRC)/eos/stereo/visualize.cpp

$(OBJ)/stereo_warp.o: $(DIRS) $(SRC)/eos/stereo/warp.h $(SRC)/eos/stereo/warp.cpp
	$(C) -o $(OBJ)/stereo_warp.o $(SRC)/eos/stereo/warp.cpp

$(OBJ)/stereo_plane_seg.o: $(DIRS) $(SRC)/eos/stereo/plane_seg.h $(SRC)/eos/stereo/plane_seg.cpp
	$(C) -o $(OBJ)/stereo_plane_seg.o $(SRC)/eos/stereo/plane_seg.cpp

$(OBJ)/stereo_layer_maker.o: $(DIRS) $(SRC)/eos/stereo/layer_maker.h $(SRC)/eos/stereo/layer_maker.cpp
	$(C) -o $(OBJ)/stereo_layer_maker.o $(SRC)/eos/stereo/layer_maker.cpp

$(OBJ)/stereo_layer_select.o: $(DIRS) $(SRC)/eos/stereo/layer_select.h $(SRC)/eos/stereo/layer_select.cpp
	$(C) -o $(OBJ)/stereo_layer_select.o $(SRC)/eos/stereo/layer_select.cpp

$(OBJ)/stereo_bleyer04.o: $(DIRS) $(SRC)/eos/stereo/bleyer04.h $(SRC)/eos/stereo/bleyer04.cpp
	$(C) -o $(OBJ)/stereo_bleyer04.o $(SRC)/eos/stereo/bleyer04.cpp

$(OBJ)/stereo_simpleBP.o: $(DIRS) $(SRC)/eos/stereo/simpleBP.h $(SRC)/eos/stereo/simpleBP.cpp
	$(C) -o $(OBJ)/stereo_simpleBP.o $(SRC)/eos/stereo/simpleBP.cpp

$(OBJ)/stereo_sfg_stereo.o: $(DIRS) $(SRC)/eos/stereo/sfg_stereo.h $(SRC)/eos/stereo/sfg_stereo.cpp
	$(C) -o $(OBJ)/stereo_sfg_stereo.o $(SRC)/eos/stereo/sfg_stereo.cpp

$(OBJ)/stereo_orient_stereo.o: $(DIRS) $(SRC)/eos/stereo/orient_stereo.h $(SRC)/eos/stereo/orient_stereo.cpp
	$(C) -o $(OBJ)/stereo_orient_stereo.o $(SRC)/eos/stereo/orient_stereo.cpp

$(OBJ)/stereo_dsi_ms.o: $(DIRS) $(SRC)/eos/stereo/dsi_ms.h $(SRC)/eos/stereo/dsi_ms.cpp
	$(C) -o $(OBJ)/stereo_dsi_ms.o $(SRC)/eos/stereo/dsi_ms.cpp

$(OBJ)/stereo_surface_fit_refine.o: $(DIRS) $(SRC)/eos/stereo/surface_fit_refine.h $(SRC)/eos/stereo/surface_fit_refine.cpp
	$(C) -o $(OBJ)/stereo_surface_fit_refine.o $(SRC)/eos/stereo/surface_fit_refine.cpp

$(OBJ)/stereo_sfs_refine.o: $(DIRS) $(SRC)/eos/stereo/sfs_refine.h $(SRC)/eos/stereo/sfs_refine.cpp
	$(C) -o $(OBJ)/stereo_sfs_refine.o $(SRC)/eos/stereo/sfs_refine.cpp

$(OBJ)/stereo_dsi.o: $(DIRS) $(SRC)/eos/stereo/dsi.h $(SRC)/eos/stereo/dsi.cpp
	$(C) -o $(OBJ)/stereo_dsi.o $(SRC)/eos/stereo/dsi.cpp

$(OBJ)/stereo_refine_orient.o: $(DIRS) $(SRC)/eos/stereo/refine_orient.h $(SRC)/eos/stereo/refine_orient.cpp
	$(C) -o $(OBJ)/stereo_refine_orient.o $(SRC)/eos/stereo/refine_orient.cpp

$(OBJ)/stereo_refine_norm.o: $(DIRS) $(SRC)/eos/stereo/refine_norm.h $(SRC)/eos/stereo/refine_norm.cpp
	$(C) -o $(OBJ)/stereo_refine_norm.o $(SRC)/eos/stereo/refine_norm.cpp

$(OBJ)/stereo_dsi_ms_2.o: $(DIRS) $(SRC)/eos/stereo/dsi_ms_2.h $(SRC)/eos/stereo/dsi_ms_2.cpp
	$(C) -o $(OBJ)/stereo_dsi_ms_2.o $(SRC)/eos/stereo/dsi_ms_2.cpp

$(OBJ)/stereo_bp_clean.o: $(DIRS) $(SRC)/eos/stereo/bp_clean.h $(SRC)/eos/stereo/bp_clean.cpp
	$(C) -o $(OBJ)/stereo_bp_clean.o $(SRC)/eos/stereo/bp_clean.cpp

$(OBJ)/stereo_ebp.o: $(DIRS) $(SRC)/eos/stereo/ebp.h $(SRC)/eos/stereo/ebp.cpp
	$(C) -o $(OBJ)/stereo_ebp.o $(SRC)/eos/stereo/ebp.cpp

$(OBJ)/stereo_simple.o: $(DIRS) $(SRC)/eos/stereo/simple.h $(SRC)/eos/stereo/simple.cpp
	$(C) -o $(OBJ)/stereo_simple.o $(SRC)/eos/stereo/simple.cpp

$(OBJ)/stereo_dsr.o: $(DIRS) $(SRC)/eos/stereo/dsr.h $(SRC)/eos/stereo/dsr.cpp
	$(C) -o $(OBJ)/stereo_dsr.o $(SRC)/eos/stereo/dsr.cpp

$(OBJ)/stereo_hebp.o: $(DIRS) $(SRC)/eos/stereo/hebp.h $(SRC)/eos/stereo/hebp.cpp
	$(C) -o $(OBJ)/stereo_hebp.o $(SRC)/eos/stereo/hebp.cpp

$(OBJ)/stereo_diffuse_correlation.o: $(DIRS) $(SRC)/eos/stereo/diffuse_correlation.h $(SRC)/eos/stereo/diffuse_correlation.cpp
	$(C) -o $(OBJ)/stereo_diffuse_correlation.o $(SRC)/eos/stereo/diffuse_correlation.cpp


$(OBJ)/mya_surfaces.o: $(DIRS) $(SRC)/eos/mya/surfaces.h $(SRC)/eos/mya/surfaces.cpp
	$(C) -o $(OBJ)/mya_surfaces.o $(SRC)/eos/mya/surfaces.cpp

$(OBJ)/mya_ied.o: $(DIRS) $(SRC)/eos/mya/ied.h $(SRC)/eos/mya/ied.cpp
	$(C) -o $(OBJ)/mya_ied.o $(SRC)/eos/mya/ied.cpp

$(OBJ)/mya_layers.o: $(DIRS) $(SRC)/eos/mya/layers.h $(SRC)/eos/mya/layers.cpp
	$(C) -o $(OBJ)/mya_layers.o $(SRC)/eos/mya/layers.cpp

$(OBJ)/mya_planes.o: $(DIRS) $(SRC)/eos/mya/planes.h $(SRC)/eos/mya/planes.cpp
	$(C) -o $(OBJ)/mya_planes.o $(SRC)/eos/mya/planes.cpp

$(OBJ)/mya_spheres.o: $(DIRS) $(SRC)/eos/mya/spheres.h $(SRC)/eos/mya/spheres.cpp
	$(C) -o $(OBJ)/mya_spheres.o $(SRC)/eos/mya/spheres.cpp

$(OBJ)/mya_disparity.o: $(DIRS) $(SRC)/eos/mya/disparity.h $(SRC)/eos/mya/disparity.cpp
	$(C) -o $(OBJ)/mya_disparity.o $(SRC)/eos/mya/disparity.cpp

$(OBJ)/mya_needles.o: $(DIRS) $(SRC)/eos/mya/needles.h $(SRC)/eos/mya/needles.cpp
	$(C) -o $(OBJ)/mya_needles.o $(SRC)/eos/mya/needles.cpp

$(OBJ)/mya_layer_score.o: $(DIRS) $(SRC)/eos/mya/layer_score.h $(SRC)/eos/mya/layer_score.cpp
	$(C) -o $(OBJ)/mya_layer_score.o $(SRC)/eos/mya/layer_score.cpp

$(OBJ)/mya_layer_merge.o: $(DIRS) $(SRC)/eos/mya/layer_merge.h $(SRC)/eos/mya/layer_merge.cpp
	$(C) -o $(OBJ)/mya_layer_merge.o $(SRC)/eos/mya/layer_merge.cpp

$(OBJ)/mya_layer_grow.o: $(DIRS) $(SRC)/eos/mya/layer_grow.h $(SRC)/eos/mya/layer_grow.cpp
	$(C) -o $(OBJ)/mya_layer_grow.o $(SRC)/eos/mya/layer_grow.cpp

$(OBJ)/mya_sfs.o: $(DIRS) $(SRC)/eos/mya/sfs.h $(SRC)/eos/mya/sfs.cpp
	$(C) -o $(OBJ)/mya_sfs.o $(SRC)/eos/mya/sfs.cpp

$(OBJ)/mya_needle_int.o: $(DIRS) $(SRC)/eos/mya/needle_int.h $(SRC)/eos/mya/needle_int.cpp
	$(C) -o $(OBJ)/mya_needle_int.o $(SRC)/eos/mya/needle_int.cpp


$(OBJ)/rend_functions.o: $(DIRS) $(SRC)/eos/rend/functions.h $(SRC)/eos/rend/functions.cpp
	$(C) -o $(OBJ)/rend_functions.o $(SRC)/eos/rend/functions.cpp

$(OBJ)/rend_pixels.o: $(DIRS) $(SRC)/eos/rend/pixels.h $(SRC)/eos/rend/pixels.cpp
	$(C) -o $(OBJ)/rend_pixels.o $(SRC)/eos/rend/pixels.cpp

$(OBJ)/rend_rerender.o: $(DIRS) $(SRC)/eos/rend/rerender.h $(SRC)/eos/rend/rerender.cpp
	$(C) -o $(OBJ)/rend_rerender.o $(SRC)/eos/rend/rerender.cpp

$(OBJ)/rend_visualise.o: $(DIRS) $(SRC)/eos/rend/visualise.h $(SRC)/eos/rend/visualise.cpp
	$(C) -o $(OBJ)/rend_visualise.o $(SRC)/eos/rend/visualise.cpp

$(OBJ)/rend_renderer.o: $(DIRS) $(SRC)/eos/rend/renderer.h $(SRC)/eos/rend/renderer.cpp
	$(C) -o $(OBJ)/rend_renderer.o $(SRC)/eos/rend/renderer.cpp

$(OBJ)/rend_databases.o: $(DIRS) $(SRC)/eos/rend/databases.h $(SRC)/eos/rend/databases.cpp
	$(C) -o $(OBJ)/rend_databases.o $(SRC)/eos/rend/databases.cpp

$(OBJ)/rend_renderers.o: $(DIRS) $(SRC)/eos/rend/renderers.h $(SRC)/eos/rend/renderers.cpp
	$(C) -o $(OBJ)/rend_renderers.o $(SRC)/eos/rend/renderers.cpp

$(OBJ)/rend_backgrounds.o: $(DIRS) $(SRC)/eos/rend/backgrounds.h $(SRC)/eos/rend/backgrounds.cpp
	$(C) -o $(OBJ)/rend_backgrounds.o $(SRC)/eos/rend/backgrounds.cpp

$(OBJ)/rend_viewers.o: $(DIRS) $(SRC)/eos/rend/viewers.h $(SRC)/eos/rend/viewers.cpp
	$(C) -o $(OBJ)/rend_viewers.o $(SRC)/eos/rend/viewers.cpp

$(OBJ)/rend_samplers.o: $(DIRS) $(SRC)/eos/rend/samplers.h $(SRC)/eos/rend/samplers.cpp
	$(C) -o $(OBJ)/rend_samplers.o $(SRC)/eos/rend/samplers.cpp

$(OBJ)/rend_tone_mappers.o: $(DIRS) $(SRC)/eos/rend/tone_mappers.h $(SRC)/eos/rend/tone_mappers.cpp
	$(C) -o $(OBJ)/rend_tone_mappers.o $(SRC)/eos/rend/tone_mappers.cpp

$(OBJ)/rend_lights.o: $(DIRS) $(SRC)/eos/rend/lights.h $(SRC)/eos/rend/lights.cpp
	$(C) -o $(OBJ)/rend_lights.o $(SRC)/eos/rend/lights.cpp

$(OBJ)/rend_objects.o: $(DIRS) $(SRC)/eos/rend/objects.h $(SRC)/eos/rend/objects.cpp
	$(C) -o $(OBJ)/rend_objects.o $(SRC)/eos/rend/objects.cpp

$(OBJ)/rend_materials.o: $(DIRS) $(SRC)/eos/rend/materials.h $(SRC)/eos/rend/materials.cpp
	$(C) -o $(OBJ)/rend_materials.o $(SRC)/eos/rend/materials.cpp

$(OBJ)/rend_textures.o: $(DIRS) $(SRC)/eos/rend/textures.h $(SRC)/eos/rend/textures.cpp
	$(C) -o $(OBJ)/rend_textures.o $(SRC)/eos/rend/textures.cpp

$(OBJ)/rend_scenes.o: $(DIRS) $(SRC)/eos/rend/scenes.h $(SRC)/eos/rend/scenes.cpp
	$(C) -o $(OBJ)/rend_scenes.o $(SRC)/eos/rend/scenes.cpp

$(OBJ)/rend_graphs.o: $(DIRS) $(SRC)/eos/rend/graphs.h $(SRC)/eos/rend/graphs.cpp
	$(C) -o $(OBJ)/rend_graphs.o $(SRC)/eos/rend/graphs.cpp


$(OBJ)/cam_cameras.o: $(DIRS) $(SRC)/eos/cam/cameras.h $(SRC)/eos/cam/cameras.cpp
	$(C) -o $(OBJ)/cam_cameras.o $(SRC)/eos/cam/cameras.cpp

$(OBJ)/cam_homography.o: $(DIRS) $(SRC)/eos/cam/homography.h $(SRC)/eos/cam/homography.cpp
	$(C) -o $(OBJ)/cam_homography.o $(SRC)/eos/cam/homography.cpp

$(OBJ)/cam_calibration.o: $(DIRS) $(SRC)/eos/cam/calibration.h $(SRC)/eos/cam/calibration.cpp
	$(C) -o $(OBJ)/cam_calibration.o $(SRC)/eos/cam/calibration.cpp

$(OBJ)/cam_fundamental.o: $(DIRS) $(SRC)/eos/cam/fundamental.h $(SRC)/eos/cam/fundamental.cpp
	$(C) -o $(OBJ)/cam_fundamental.o $(SRC)/eos/cam/fundamental.cpp

$(OBJ)/cam_triangulation.o: $(DIRS) $(SRC)/eos/cam/triangulation.h $(SRC)/eos/cam/triangulation.cpp
	$(C) -o $(OBJ)/cam_triangulation.o $(SRC)/eos/cam/triangulation.cpp

$(OBJ)/cam_files.o: $(DIRS) $(SRC)/eos/cam/files.h $(SRC)/eos/cam/files.cpp
	$(C) -o $(OBJ)/cam_files.o $(SRC)/eos/cam/files.cpp

$(OBJ)/cam_rectification.o: $(DIRS) $(SRC)/eos/cam/rectification.h $(SRC)/eos/cam/rectification.cpp
	$(C) -o $(OBJ)/cam_rectification.o $(SRC)/eos/cam/rectification.cpp

$(OBJ)/cam_disparity_converter.o: $(DIRS) $(SRC)/eos/cam/disparity_converter.h $(SRC)/eos/cam/disparity_converter.cpp
	$(C_AO) -o $(OBJ)/cam_disparity_converter.o $(SRC)/eos/cam/disparity_converter.cpp

$(OBJ)/cam_resectioning.o: $(DIRS) $(SRC)/eos/cam/resectioning.h $(SRC)/eos/cam/resectioning.cpp
	$(C) -o $(OBJ)/cam_resectioning.o $(SRC)/eos/cam/resectioning.cpp

$(OBJ)/cam_make_disp.o: $(DIRS) $(SRC)/eos/cam/make_disp.h $(SRC)/eos/cam/make_disp.cpp
	$(C) -o $(OBJ)/cam_make_disp.o $(SRC)/eos/cam/make_disp.cpp

$(OBJ)/cam_cam_render.o: $(DIRS) $(SRC)/eos/cam/cam_render.h $(SRC)/eos/cam/cam_render.cpp
	$(C) -o $(OBJ)/cam_cam_render.o $(SRC)/eos/cam/cam_render.cpp


$(OBJ)/gui_base.o: $(DIRS) $(SRC)/eos/gui/base.h $(SRC)/eos/gui/base.cpp
	$(C) -o $(OBJ)/gui_base.o $(SRC)/eos/gui/base.cpp

$(OBJ)/gui_callbacks.o: $(DIRS) $(SRC)/eos/gui/callbacks.h $(SRC)/eos/gui/callbacks.cpp
	$(C) -o $(OBJ)/gui_callbacks.o $(SRC)/eos/gui/callbacks.cpp

$(OBJ)/gui_widgets.o: $(DIRS) $(SRC)/eos/gui/widgets.h $(SRC)/eos/gui/widgets.cpp
	$(C) -o $(OBJ)/gui_widgets.o $(SRC)/eos/gui/widgets.cpp

$(OBJ)/gui_gtk_funcs.o: $(DIRS) $(SRC)/eos/gui/gtk_funcs.h $(SRC)/eos/gui/gtk_funcs.cpp
	$(C) -o $(OBJ)/gui_gtk_funcs.o $(SRC)/eos/gui/gtk_funcs.cpp

$(OBJ)/gui_gtk_widgets.o: $(DIRS) $(SRC)/eos/gui/gtk_widgets.h $(SRC)/eos/gui/gtk_widgets.cpp
	$(C) -o $(OBJ)/gui_gtk_widgets.o $(SRC)/eos/gui/gtk_widgets.cpp


$(OBJ)/inf_fg_types.o: $(DIRS) $(SRC)/eos/inf/fg_types.h $(SRC)/eos/inf/fg_types.cpp
	$(C) -o $(OBJ)/inf_fg_types.o $(SRC)/eos/inf/fg_types.cpp

$(OBJ)/inf_fg_funcs.o: $(DIRS) $(SRC)/eos/inf/fg_funcs.h $(SRC)/eos/inf/fg_funcs.cpp
	$(C) -o $(OBJ)/inf_fg_funcs.o $(SRC)/eos/inf/fg_funcs.cpp

$(OBJ)/inf_fg_vars.o: $(DIRS) $(SRC)/eos/inf/fg_vars.h $(SRC)/eos/inf/fg_vars.cpp
	$(C) -o $(OBJ)/inf_fg_vars.o $(SRC)/eos/inf/fg_vars.cpp

$(OBJ)/inf_factor_graphs.o: $(DIRS) $(SRC)/eos/inf/factor_graphs.h $(SRC)/eos/inf/factor_graphs.cpp
	$(C) -o $(OBJ)/inf_factor_graphs.o $(SRC)/eos/inf/factor_graphs.cpp

$(OBJ)/inf_field_graphs.o: $(DIRS) $(SRC)/eos/inf/field_graphs.h $(SRC)/eos/inf/field_graphs.cpp
	$(C) -o $(OBJ)/inf_field_graphs.o $(SRC)/eos/inf/field_graphs.cpp

$(OBJ)/inf_fig_variables.o: $(DIRS) $(SRC)/eos/inf/fig_variables.h $(SRC)/eos/inf/fig_variables.cpp
	$(C) -o $(OBJ)/inf_fig_variables.o $(SRC)/eos/inf/fig_variables.cpp

$(OBJ)/inf_fig_factors.o: $(DIRS) $(SRC)/eos/inf/fig_factors.h $(SRC)/eos/inf/fig_factors.cpp
	$(C) -o $(OBJ)/inf_fig_factors.o $(SRC)/eos/inf/fig_factors.cpp

$(OBJ)/inf_gauss_integration.o: $(DIRS) $(SRC)/eos/inf/gauss_integration.h $(SRC)/eos/inf/gauss_integration.cpp
	$(C) -o $(OBJ)/inf_gauss_integration.o $(SRC)/eos/inf/gauss_integration.cpp

$(OBJ)/inf_model_seg.o: $(DIRS) $(SRC)/eos/inf/model_seg.h $(SRC)/eos/inf/model_seg.cpp
	$(C) -o $(OBJ)/inf_model_seg.o $(SRC)/eos/inf/model_seg.cpp

$(OBJ)/inf_gauss_integration_hier.o: $(DIRS) $(SRC)/eos/inf/gauss_integration_hier.h $(SRC)/eos/inf/gauss_integration_hier.cpp
	$(C) -o $(OBJ)/inf_gauss_integration_hier.o $(SRC)/eos/inf/gauss_integration_hier.cpp

$(OBJ)/inf_bin_bp_2d.o: $(DIRS) $(SRC)/eos/inf/bin_bp_2d.h $(SRC)/eos/inf/bin_bp_2d.cpp
	$(C) -o $(OBJ)/inf_bin_bp_2d.o $(SRC)/eos/inf/bin_bp_2d.cpp


$(OBJ)/os_cameras.o: $(DIRS) $(SRC)/eos/os/cameras.h $(SRC)/eos/os/cameras.cpp
	$(C) -o $(OBJ)/os_cameras.o $(SRC)/eos/os/cameras.cpp

$(OBJ)/os_gphoto2_funcs.o: $(DIRS) $(SRC)/eos/os/gphoto2_funcs.h $(SRC)/eos/os/gphoto2_funcs.cpp
	$(C) -o $(OBJ)/os_gphoto2_funcs.o $(SRC)/eos/os/gphoto2_funcs.cpp

$(OBJ)/os_console.o: $(DIRS) $(SRC)/eos/os/console.h $(SRC)/eos/os/console.cpp
	$(C) -o $(OBJ)/os_console.o $(SRC)/eos/os/console.cpp

$(OBJ)/os_command.o: $(DIRS) $(SRC)/eos/os/command.h $(SRC)/eos/os/command.cpp
	$(C) -o $(OBJ)/os_command.o $(SRC)/eos/os/command.cpp


$(OBJ)/mt_threads.o: $(DIRS) $(SRC)/eos/mt/threads.h $(SRC)/eos/mt/threads.cpp
	$(C) -o $(OBJ)/mt_threads.o $(SRC)/eos/mt/threads.cpp

$(OBJ)/mt_locks.o: $(DIRS) $(SRC)/eos/mt/locks.h $(SRC)/eos/mt/locks.cpp
	$(C) -o $(OBJ)/mt_locks.o $(SRC)/eos/mt/locks.cpp


$(OBJ)/sur_mesh.o: $(DIRS) $(SRC)/eos/sur/mesh.h $(SRC)/eos/sur/mesh.cpp
	$(C) -o $(OBJ)/sur_mesh.o $(SRC)/eos/sur/mesh.cpp

$(OBJ)/sur_mesh_iter.o: $(DIRS) $(SRC)/eos/sur/mesh_iter.h $(SRC)/eos/sur/mesh_iter.cpp
	$(C) -o $(OBJ)/sur_mesh_iter.o $(SRC)/eos/sur/mesh_iter.cpp

$(OBJ)/sur_mesh_sup.o: $(DIRS) $(SRC)/eos/sur/mesh_sup.h $(SRC)/eos/sur/mesh_sup.cpp
	$(C) -o $(OBJ)/sur_mesh_sup.o $(SRC)/eos/sur/mesh_sup.cpp

$(OBJ)/sur_catmull_clark.o: $(DIRS) $(SRC)/eos/sur/catmull_clark.h $(SRC)/eos/sur/catmull_clark.cpp
	$(C) -o $(OBJ)/sur_catmull_clark.o $(SRC)/eos/sur/catmull_clark.cpp

$(OBJ)/sur_intersection.o: $(DIRS) $(SRC)/eos/sur/intersection.h $(SRC)/eos/sur/intersection.cpp
	$(C) -o $(OBJ)/sur_intersection.o $(SRC)/eos/sur/intersection.cpp

$(OBJ)/sur_subdivide.o: $(DIRS) $(SRC)/eos/sur/subdivide.h $(SRC)/eos/sur/subdivide.cpp
	$(C) -o $(OBJ)/sur_subdivide.o $(SRC)/eos/sur/subdivide.cpp

$(OBJ)/sur_simplify.o: $(DIRS) $(SRC)/eos/sur/simplify.h $(SRC)/eos/sur/simplify.cpp
	$(C) -o $(OBJ)/sur_simplify.o $(SRC)/eos/sur/simplify.cpp


$(OBJ)/sfs_worthington.o: $(DIRS) $(SRC)/eos/sfs/worthington.h $(SRC)/eos/sfs/worthington.cpp
	$(C) -o $(OBJ)/sfs_worthington.o $(SRC)/eos/sfs/worthington.cpp

$(OBJ)/sfs_lambertian_fit.o: $(DIRS) $(SRC)/eos/sfs/lambertian_fit.h $(SRC)/eos/sfs/lambertian_fit.cpp
	$(C) -o $(OBJ)/sfs_lambertian_fit.o $(SRC)/eos/sfs/lambertian_fit.cpp

$(OBJ)/sfs_lambertian_segs.o: $(DIRS) $(SRC)/eos/sfs/lambertian_segs.h $(SRC)/eos/sfs/lambertian_segs.cpp
	$(C) -o $(OBJ)/sfs_lambertian_segs.o $(SRC)/eos/sfs/lambertian_segs.cpp

$(OBJ)/sfs_lambertian_pp.o: $(DIRS) $(SRC)/eos/sfs/lambertian_pp.h $(SRC)/eos/sfs/lambertian_pp.cpp
	$(C) -o $(OBJ)/sfs_lambertian_pp.o $(SRC)/eos/sfs/lambertian_pp.cpp

$(OBJ)/sfs_lambertian_hough.o: $(DIRS) $(SRC)/eos/sfs/lambertian_hough.h $(SRC)/eos/sfs/lambertian_hough.cpp
	$(C) -o $(OBJ)/sfs_lambertian_hough.o $(SRC)/eos/sfs/lambertian_hough.cpp

$(OBJ)/sfs_lambertian_segment.o: $(DIRS) $(SRC)/eos/sfs/lambertian_segment.h $(SRC)/eos/sfs/lambertian_segment.cpp
	$(C) -o $(OBJ)/sfs_lambertian_segment.o $(SRC)/eos/sfs/lambertian_segment.cpp

$(OBJ)/sfs_sfsao_gd.o: $(DIRS) $(SRC)/eos/sfs/sfsao_gd.h $(SRC)/eos/sfs/sfsao_gd.cpp
	$(C) -o $(OBJ)/sfs_sfsao_gd.o $(SRC)/eos/sfs/sfsao_gd.cpp

$(OBJ)/sfs_sfs_bp.o: $(DIRS) $(SRC)/eos/sfs/sfs_bp.h $(SRC)/eos/sfs/sfs_bp.cpp
	$(C) -o $(OBJ)/sfs_sfs_bp.o $(SRC)/eos/sfs/sfs_bp.cpp

$(OBJ)/sfs_zheng.o: $(DIRS) $(SRC)/eos/sfs/zheng.h $(SRC)/eos/sfs/zheng.cpp
	$(C) -o $(OBJ)/sfs_zheng.o $(SRC)/eos/sfs/zheng.cpp

$(OBJ)/sfs_lee.o: $(DIRS) $(SRC)/eos/sfs/lee.h $(SRC)/eos/sfs/lee.cpp
	$(C) -o $(OBJ)/sfs_lee.o $(SRC)/eos/sfs/lee.cpp

$(OBJ)/sfs_albedo_est.o: $(DIRS) $(SRC)/eos/sfs/albedo_est.h $(SRC)/eos/sfs/albedo_est.cpp
	$(C) -o $(OBJ)/sfs_albedo_est.o $(SRC)/eos/sfs/albedo_est.cpp


$(OBJ)/fit_disp_fish.o: $(DIRS) $(SRC)/eos/fit/disp_fish.h $(SRC)/eos/fit/disp_fish.cpp
	$(C) -o $(OBJ)/fit_disp_fish.o $(SRC)/eos/fit/disp_fish.cpp

$(OBJ)/fit_disp_norm.o: $(DIRS) $(SRC)/eos/fit/disp_norm.h $(SRC)/eos/fit/disp_norm.cpp
	$(C) -o $(OBJ)/fit_disp_norm.o $(SRC)/eos/fit/disp_norm.cpp

$(OBJ)/fit_light_dir.o: $(DIRS) $(SRC)/eos/fit/light_dir.h $(SRC)/eos/fit/light_dir.cpp
	$(C) -o $(OBJ)/fit_light_dir.o $(SRC)/eos/fit/light_dir.cpp

$(OBJ)/fit_sphere_sample.o: $(DIRS) $(SRC)/eos/fit/sphere_sample.h $(SRC)/eos/fit/sphere_sample.cpp
	$(C) -o $(OBJ)/fit_sphere_sample.o $(SRC)/eos/fit/sphere_sample.cpp

$(OBJ)/fit_light_ambient.o: $(DIRS) $(SRC)/eos/fit/light_ambient.h $(SRC)/eos/fit/light_ambient.cpp
	$(C) -o $(OBJ)/fit_light_ambient.o $(SRC)/eos/fit/light_ambient.cpp

$(OBJ)/fit_image_sphere.o: $(DIRS) $(SRC)/eos/fit/image_sphere.h $(SRC)/eos/fit/image_sphere.cpp
	$(C) -o $(OBJ)/fit_image_sphere.o $(SRC)/eos/fit/image_sphere.cpp

$(OBJ)/fit_disp_norm_fish.o: $(DIRS) $(SRC)/eos/fit/disp_norm_fish.h $(SRC)/eos/fit/disp_norm_fish.cpp
	$(C) -o $(OBJ)/fit_disp_norm_fish.o $(SRC)/eos/fit/disp_norm_fish.cpp



###############
# Auxilary... #
###############

$(DIRS):
	-$(MKDIR) ${OUT_BASE}
	-$(MKDIR) ${OUT}
	-$(MKDIR) ${OBJ_BASE}
	-$(MKDIR) ${OBJ}
	touch $(DIRS)


ifeq ($(PLATFORM),win)
 HHC_LINE = -hhc doc\html\index.hhp
endif

ifeq ($(PLATFORM),lin)
 HHC_LINE =
endif

docs:
	-$(RMDIR) ${DOC}
	-$(MKDIR) ${DOC}
	doxygen doxymakefile
	cp $(DEP)/go.html $(DOC)/
	$(HHC_LINE)


ifeq ($(PLATFORM),win)
prep_final: $(DIRS)
	-cp $(DEVIL)/devil.dll $(OUT)
	#-cp $(ZLIB)/zlib1.dll $(OUT)

line_count:
	@echo Line Count:
	@find src -exec cat {} \; -name "*.h" -or -name "*.cpp" 2> /dev/null | grep -c ""
endif

ifeq ($(PLATFORM),lin)
prep_final: eos
	-cp $(DEVIL)/libIL.so $(OUT)
	ln -sf libeos.so.1.0 $(OUT)/libeos.so.1
	ln -sf libeos.so.1 $(OUT)/libeos.so

line_count:
	@echo Line Count:
	@find src -type f -and \( -name "*.h" -or -name "*.cpp" \) -print 2> /dev/null | xargs cat | wc -l
endif



###################
# End of makefile #
###################

