########################
# The Ares Makefile... #
########################

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

win_release:
	@mingw32-make -j 2 PLATFORM=win TYPE=release all

win_relbug:
	@mingw32-make -j 2 PLATFORM=win TYPE=relbug all

win_debug:
	@mingw32-make -j 2 PLATFORM=win TYPE=debug all


clean:
	-$(RMDIR) obj
	-$(RMDIR) bin
	-rm out.txt


EXES	= cyclops svt mscr geo_test

EXES_S1	= sfgs colour sad_stereo ba_test mya sfs fitter sift spec_rem mser segs 
EXES_S2	= bleyer04 eos_test fg_test orient sur_test sur_stereo ds_test voronoi
EXES_S3 = bp_stereo sur_bp_stereo text_to_svt bessel sfs_bp bp_test
EXES_S4 = smes 

all: $(EXES) prep_final line_count
	@echo Done



######################
# Directorys used... #
######################

OUT_BASE	= bin
OUT   		= $(OUT_BASE)/$(TYPE)
OBJ_BASE	= obj
OBJ		= $(OBJ_BASE)/$(TYPE)
SRC		= src

DEP		= dep
EOS		= ../eos
EOS_LIB		= $(EOS)/bin/$(TYPE)/

INCLUDE         = -I$(SRC) -I$(EOS)/src

DIRS		= $(OBJ)/.wibble



#################################
# Compiler & linker etc setup.. #
#################################

ifeq ($(PLATFORM),win)
 EXTRA_OPT	= # No sse optimisation for windows, v2 makes a bad executable with mingw whilst v1 produces really bad output, switch on with future compilers when not broken.
endif

ifeq ($(PLATFORM),lin)
 EXTRA_OPT	= -msse2 -mfpmath=sse
endif

COMMON_C	= -mmmx -m3dnow $(EXTRA_OPT) -ffast-math -c -Wall -Wfloat-equal $(INCLUDE) -DEOS_DLL -DEOS_X86

ifeq ($(TYPE),release)
 C_PRE		= g++ $(COMMON_C) -O3 -ftracer -DEOS_ASM -DEOS_RELEASE
 L_EXE_PRE	= g++ -Wl,-S
 L_DLL		= g++ -shared
endif

ifeq ($(TYPE),relbug)
 C_PRE		= g++ $(COMMON_C) -ggdb -O3 -ftracer -DEOS_DEBUG -DEOS_LOG_BLOCKS -DEOS_LOG_MINOR -DEOS_ASM
 L_EXE_PRE	= g++ -ggdb -Wl,-S
 L_DLL		= g++ -shared -ggdb
endif

ifeq ($(TYPE),debug)
 C_PRE		= g++ $(COMMON_C) -ggdb -DEOS_DEBUG -DEOS_LOG_BLOCKS -DEOS_LOG_MINOR
 L_EXE_PRE	= g++ -ggdb
 L_DLL		= g++ -shared -ggdb
endif

ifeq ($(PLATFORM),win)
 C		= $(C_PRE) -DEOS_WIN32
 L_EXE		= $(L_EXE_PRE) -Wl,--enable-runtime-pseudo-reloc
 L_EXE_APP	= $(L_EXE) -mwindows
 PEXT		= .exe
endif

ifeq ($(PLATFORM),lin)
 C		= $(C_PRE) -DEOS_LINUX
 L_EXE		= $(L_EXE_PRE)
 L_EXE_APP	= $(L_EXE)
 PEXT		=
endif



############
# eos_test #
############

FINAL_EOS_TEST	= $(OUT)/eos_test$(PEXT)
OBJS_EOS_TEST	= $(OBJ)/eos_test_main.o


eos_test: $(FINAL_EOS_TEST)

$(FINAL_EOS_TEST): $(OBJS_EOS_TEST)
	$(L_EXE) -o $(FINAL_EOS_TEST) $(OBJS_EOS_TEST) -L$(EOS_LIB) -leos


$(OBJ)/eos_test_main.o: $(DIRS) $(SRC)/eos_test/main.h $(SRC)/eos_test/main.cpp
	$(C) -o $(OBJ)/eos_test_main.o $(SRC)/eos_test/main.cpp



#######
# sfs #
#######

FINAL_SFS	= $(OUT)/sfs$(PEXT)
OBJS_SFS	= $(OBJ)/sfs_main.o


sfs: $(FINAL_SFS)

$(FINAL_SFS): $(OBJS_SFS)
	$(L_EXE) -o $(FINAL_SFS) $(OBJS_SFS) -L$(EOS_LIB) -leos


$(OBJ)/sfs_main.o: $(DIRS) $(SRC)/sfs/main.h $(SRC)/sfs/main.cpp
	$(C) -o $(OBJ)/sfs_main.o $(SRC)/sfs/main.cpp



########
# segs #
########

FINAL_SEGS	= $(OUT)/segs$(PEXT)
OBJS_SEGS	= $(OBJ)/segs_main.o


segs: $(FINAL_SEGS)

$(FINAL_SEGS): $(OBJS_SEGS)
	$(L_EXE) -o $(FINAL_SEGS) $(OBJS_SEGS) -L$(EOS_LIB) -leos


$(OBJ)/segs_main.o: $(DIRS) $(SRC)/segs/main.h $(SRC)/segs/main.cpp
	$(C) -o $(OBJ)/segs_main.o $(SRC)/segs/main.cpp



############
# mat_test #
############

FINAL_MAT_TEST	= $(OUT)/mat_test$(PEXT)
OBJS_MAT_TEST	= $(OBJ)/mat_test_main.o


mat_test: $(FINAL_MAT_TEST)

$(FINAL_MAT_TEST): $(OBJS_MAT_TEST)
	$(L_EXE) -o $(FINAL_MAT_TEST) $(OBJS_MAT_TEST) -L$(EOS_LIB) -leos


$(OBJ)/mat_test_main.o: $(DIRS) $(SRC)/mat_test/main.h $(SRC)/mat_test/main.cpp
	$(C) -o $(OBJ)/mat_test_main.o $(SRC)/mat_test/main.cpp



##########
# colour #
##########

FINAL_COLOUR	= $(OUT)/colour$(PEXT)
OBJS_COLOUR	= $(OBJ)/colour_main.o


colour: $(FINAL_COLOUR)

$(FINAL_COLOUR): $(OBJS_COLOUR)
	$(L_EXE) -o $(FINAL_COLOUR) $(OBJS_COLOUR) -L$(EOS_LIB) -leos


$(OBJ)/colour_main.o: $(DIRS) $(SRC)/colour/main.h $(SRC)/colour/main.cpp
	$(C) -o $(OBJ)/colour_main.o $(SRC)/colour/main.cpp



##############
# sad_stereo #
##############

FINAL_SAD_STEREO	= $(OUT)/sad_stereo$(PEXT)
OBJS_SAD_STEREO		= $(OBJ)/sad_stereo_main.o


sad_stereo: $(FINAL_SAD_STEREO)

$(FINAL_SAD_STEREO): $(OBJS_SAD_STEREO)
	$(L_EXE) -o $(FINAL_SAD_STEREO) $(OBJS_SAD_STEREO) -L$(EOS_LIB) -leos


$(OBJ)/sad_stereo_main.o: $(DIRS) $(SRC)/sad_stereo/main.h $(SRC)/sad_stereo/main.cpp
	$(C) -o $(OBJ)/sad_stereo_main.o $(SRC)/sad_stereo/main.cpp



############
# bleyer04 #
############

FINAL_BLEYER04	= $(OUT)/bleyer04$(PEXT)
OBJS_BLEYER04	= $(OBJ)/bleyer04_main.o


bleyer04: $(FINAL_BLEYER04)

$(FINAL_BLEYER04): $(OBJS_BLEYER04)
	$(L_EXE) -o $(FINAL_BLEYER04) $(OBJS_BLEYER04) -L$(EOS_LIB) -leos


$(OBJ)/bleyer04_main.o: $(DIRS) $(SRC)/bleyer04/main.h $(SRC)/bleyer04/main.cpp
	$(C) -o $(OBJ)/bleyer04_main.o $(SRC)/bleyer04/main.cpp



#######
# mya #
#######

FINAL_MYA	= $(OUT)/mya$(PEXT)
OBJS_MYA	= $(OBJ)/mya_main.o


mya: $(FINAL_MYA)

$(FINAL_MYA): $(OBJS_MYA)
	$(L_EXE) -o $(FINAL_MYA) $(OBJS_MYA) -L$(EOS_LIB) -leos


$(OBJ)/mya_main.o: $(DIRS) $(SRC)/mya/main.h $(SRC)/mya/main.cpp
	$(C) -o $(OBJ)/mya_main.o $(SRC)/mya/main.cpp



########
# sift #
########

FINAL_SIFT	= $(OUT)/sift$(PEXT)
OBJS_SIFT	= $(OBJ)/sift_main.o


sift: $(FINAL_SIFT)

$(FINAL_SIFT): $(OBJS_SIFT)
	$(L_EXE) -o $(FINAL_SIFT) $(OBJS_SIFT) -L$(EOS_LIB) -leos


$(OBJ)/sift_main.o: $(DIRS) $(SRC)/sift/main.h $(SRC)/sift/main.cpp
	$(C) -o $(OBJ)/sift_main.o $(SRC)/sift/main.cpp



###########
# ba_test #
###########

FINAL_BA_TEST	= $(OUT)/ba_test$(PEXT)
OBJS_BA_TEST	= $(OBJ)/ba_test_main.o


ba_test: $(FINAL_BA_TEST)

$(FINAL_BA_TEST): $(OBJS_BA_TEST)
	$(L_EXE) -o $(FINAL_BA_TEST) $(OBJS_BA_TEST) -L$(EOS_LIB) -leos


$(OBJ)/ba_test_main.o: $(DIRS) $(SRC)/ba_test/main.h $(SRC)/ba_test/main.cpp
	$(C) -o $(OBJ)/ba_test_main.o $(SRC)/ba_test/main.cpp



##########
# fitter #
##########

FINAL_FITTER	= $(OUT)/fitter$(PEXT)
OBJS_FITTER	= $(OBJ)/fitter_main.o $(OBJ)/fitter_fitter.o $(OBJ)/fitter_fit_algs.o $(OBJ)/fitter_all_algs.o


fitter: $(FINAL_FITTER)

$(FINAL_FITTER): $(OBJS_FITTER)
	$(L_EXE) -o $(FINAL_FITTER) $(OBJS_FITTER) -L$(EOS_LIB) -leos


$(OBJ)/fitter_main.o: $(DIRS) $(SRC)/fitter/main.h $(SRC)/fitter/main.cpp
	$(C) -o $(OBJ)/fitter_main.o $(SRC)/fitter/main.cpp

$(OBJ)/fitter_fitter.o: $(DIRS) $(SRC)/fitter/fitter.h $(SRC)/fitter/fitter.cpp
	$(C) -o $(OBJ)/fitter_fitter.o $(SRC)/fitter/fitter.cpp

$(OBJ)/fitter_fit_algs.o: $(DIRS) $(SRC)/fitter/fit_algs.h $(SRC)/fitter/fit_algs.cpp
	$(C) -o $(OBJ)/fitter_fit_algs.o $(SRC)/fitter/fit_algs.cpp

$(OBJ)/fitter_all_algs.o: $(DIRS) $(SRC)/fitter/all_algs.h $(SRC)/fitter/all_algs.cpp
	$(C) -o $(OBJ)/fitter_all_algs.o $(SRC)/fitter/all_algs.cpp



###########
# cyclops #
###########

FINAL_CYCLOPS	= $(OUT)/cyclops$(PEXT)
OBJS_CYCLOPS	= $(OBJ)/cyclops_intrinsic.o $(OBJ)/cyclops_protractor.o $(OBJ)/cyclops_fundamental.o $(OBJ)/cyclops_triangulator.o $(OBJ)/cyclops_crop.o $(OBJ)/cyclops_crop_pair.o $(OBJ)/cyclops_rectification.o $(OBJ)/cyclops_capture.o $(OBJ)/cyclops_to_mesh.o $(OBJ)/cyclops_stereopsis.o $(OBJ)/cyclops_warp.o $(OBJ)/cyclops_scale_pair.o $(OBJ)/cyclops_homography.o $(OBJ)/cyclops_calibration.o $(OBJ)/cyclops_model_disp.o $(OBJ)/cyclops_comp_disp.o $(OBJ)/cyclops_file_info.o $(OBJ)/cyclops_disp_mask.o $(OBJ)/cyclops_colour_balance.o $(OBJ)/cyclops_undistorter.o $(OBJ)/cyclops_disp_scale.o $(OBJ)/cyclops_to_orient.o $(OBJ)/cyclops_model_render.o $(OBJ)/cyclops_anaglyph.o $(OBJ)/cyclops_disp_clean.o $(OBJ)/cyclops_scale.o $(OBJ)/cyclops_disp_crop.o $(OBJ)/cyclops_mesh_ops.o $(OBJ)/cyclops_comp_needle.o $(OBJ)/cyclops_sfs.o $(OBJ)/cyclops_integration.o $(OBJ)/cyclops_lighting.o $(OBJ)/cyclops_segmentation.o $(OBJ)/cyclops_light_est.o


cyclops: $(FINAL_CYCLOPS)

$(FINAL_CYCLOPS): $(OBJ)/cyclops_main.o $(OBJS_CYCLOPS)
	$(L_EXE_APP) -o $(FINAL_CYCLOPS) $(OBJ)/cyclops_main.o $(OBJS_CYCLOPS) -L$(EOS_LIB) -leos


$(OBJ)/cyclops_main.o: $(DIRS) $(SRC)/cyclops/main.h $(SRC)/cyclops/main.cpp $(OBJS_CYCLOPS)
	$(C) -o $(OBJ)/cyclops_main.o $(SRC)/cyclops/main.cpp

$(OBJ)/cyclops_intrinsic.o: $(DIRS) $(SRC)/cyclops/intrinsic.h $(SRC)/cyclops/intrinsic.cpp
	$(C) -o $(OBJ)/cyclops_intrinsic.o $(SRC)/cyclops/intrinsic.cpp

$(OBJ)/cyclops_protractor.o: $(DIRS) $(SRC)/cyclops/protractor.h $(SRC)/cyclops/protractor.cpp
	$(C) -o $(OBJ)/cyclops_protractor.o $(SRC)/cyclops/protractor.cpp

$(OBJ)/cyclops_fundamental.o: $(DIRS) $(SRC)/cyclops/fundamental.h $(SRC)/cyclops/fundamental.cpp
	$(C) -o $(OBJ)/cyclops_fundamental.o $(SRC)/cyclops/fundamental.cpp

$(OBJ)/cyclops_triangulator.o: $(DIRS) $(SRC)/cyclops/triangulator.h $(SRC)/cyclops/triangulator.cpp
	$(C) -o $(OBJ)/cyclops_triangulator.o $(SRC)/cyclops/triangulator.cpp

$(OBJ)/cyclops_crop.o: $(DIRS) $(SRC)/cyclops/crop.h $(SRC)/cyclops/crop.cpp
	$(C) -o $(OBJ)/cyclops_crop.o $(SRC)/cyclops/crop.cpp

$(OBJ)/cyclops_crop_pair.o: $(DIRS) $(SRC)/cyclops/crop_pair.h $(SRC)/cyclops/crop_pair.cpp
	$(C) -o $(OBJ)/cyclops_crop_pair.o $(SRC)/cyclops/crop_pair.cpp

$(OBJ)/cyclops_rectification.o: $(DIRS) $(SRC)/cyclops/rectification.h $(SRC)/cyclops/rectification.cpp
	$(C) -o $(OBJ)/cyclops_rectification.o $(SRC)/cyclops/rectification.cpp

$(OBJ)/cyclops_capture.o: $(DIRS) $(SRC)/cyclops/capture.h $(SRC)/cyclops/capture.cpp
	$(C) -o $(OBJ)/cyclops_capture.o $(SRC)/cyclops/capture.cpp

$(OBJ)/cyclops_to_mesh.o: $(DIRS) $(SRC)/cyclops/to_mesh.h $(SRC)/cyclops/to_mesh.cpp
	$(C) -o $(OBJ)/cyclops_to_mesh.o $(SRC)/cyclops/to_mesh.cpp

$(OBJ)/cyclops_stereopsis.o: $(DIRS) $(SRC)/cyclops/stereopsis.h $(SRC)/cyclops/stereopsis.cpp
	$(C) -o $(OBJ)/cyclops_stereopsis.o $(SRC)/cyclops/stereopsis.cpp

$(OBJ)/cyclops_warp.o: $(DIRS) $(SRC)/cyclops/warp.h $(SRC)/cyclops/warp.cpp
	$(C) -o $(OBJ)/cyclops_warp.o $(SRC)/cyclops/warp.cpp

$(OBJ)/cyclops_scale_pair.o: $(DIRS) $(SRC)/cyclops/scale_pair.h $(SRC)/cyclops/scale_pair.cpp
	$(C) -o $(OBJ)/cyclops_scale_pair.o $(SRC)/cyclops/scale_pair.cpp

$(OBJ)/cyclops_homography.o: $(DIRS) $(SRC)/cyclops/homography.h $(SRC)/cyclops/homography.cpp
	$(C) -o $(OBJ)/cyclops_homography.o $(SRC)/cyclops/homography.cpp

$(OBJ)/cyclops_calibration.o: $(DIRS) $(SRC)/cyclops/calibration.h $(SRC)/cyclops/calibration.cpp
	$(C) -o $(OBJ)/cyclops_calibration.o $(SRC)/cyclops/calibration.cpp

$(OBJ)/cyclops_model_disp.o: $(DIRS) $(SRC)/cyclops/model_disp.h $(SRC)/cyclops/model_disp.cpp
	$(C) -o $(OBJ)/cyclops_model_disp.o $(SRC)/cyclops/model_disp.cpp

$(OBJ)/cyclops_comp_disp.o: $(DIRS) $(SRC)/cyclops/comp_disp.h $(SRC)/cyclops/comp_disp.cpp
	$(C) -o $(OBJ)/cyclops_comp_disp.o $(SRC)/cyclops/comp_disp.cpp

$(OBJ)/cyclops_file_info.o: $(DIRS) $(SRC)/cyclops/file_info.h $(SRC)/cyclops/file_info.cpp
	$(C) -o $(OBJ)/cyclops_file_info.o $(SRC)/cyclops/file_info.cpp

$(OBJ)/cyclops_disp_mask.o: $(DIRS) $(SRC)/cyclops/disp_mask.h $(SRC)/cyclops/disp_mask.cpp
	$(C) -o $(OBJ)/cyclops_disp_mask.o $(SRC)/cyclops/disp_mask.cpp

$(OBJ)/cyclops_colour_balance.o: $(DIRS) $(SRC)/cyclops/colour_balance.h $(SRC)/cyclops/colour_balance.cpp
	$(C) -o $(OBJ)/cyclops_colour_balance.o $(SRC)/cyclops/colour_balance.cpp

$(OBJ)/cyclops_undistorter.o: $(DIRS) $(SRC)/cyclops/undistorter.h $(SRC)/cyclops/undistorter.cpp
	$(C) -o $(OBJ)/cyclops_undistorter.o $(SRC)/cyclops/undistorter.cpp

$(OBJ)/cyclops_disp_scale.o: $(DIRS) $(SRC)/cyclops/disp_scale.h $(SRC)/cyclops/disp_scale.cpp
	$(C) -o $(OBJ)/cyclops_disp_scale.o $(SRC)/cyclops/disp_scale.cpp

$(OBJ)/cyclops_to_orient.o: $(DIRS) $(SRC)/cyclops/to_orient.h $(SRC)/cyclops/to_orient.cpp
	$(C) -o $(OBJ)/cyclops_to_orient.o $(SRC)/cyclops/to_orient.cpp

$(OBJ)/cyclops_model_render.o: $(DIRS) $(SRC)/cyclops/model_render.h $(SRC)/cyclops/model_render.cpp
	$(C) -o $(OBJ)/cyclops_model_render.o $(SRC)/cyclops/model_render.cpp

$(OBJ)/cyclops_anaglyph.o: $(DIRS) $(SRC)/cyclops/anaglyph.h $(SRC)/cyclops/anaglyph.cpp
	$(C) -o $(OBJ)/cyclops_anaglyph.o $(SRC)/cyclops/anaglyph.cpp

$(OBJ)/cyclops_disp_clean.o: $(DIRS) $(SRC)/cyclops/disp_clean.h $(SRC)/cyclops/disp_clean.cpp
	$(C) -o $(OBJ)/cyclops_disp_clean.o $(SRC)/cyclops/disp_clean.cpp

$(OBJ)/cyclops_scale.o: $(DIRS) $(SRC)/cyclops/scale.h $(SRC)/cyclops/scale.cpp
	$(C) -o $(OBJ)/cyclops_scale.o $(SRC)/cyclops/scale.cpp

$(OBJ)/cyclops_disp_crop.o: $(DIRS) $(SRC)/cyclops/disp_crop.h $(SRC)/cyclops/disp_crop.cpp
	$(C) -o $(OBJ)/cyclops_disp_crop.o $(SRC)/cyclops/disp_crop.cpp

$(OBJ)/cyclops_mesh_ops.o: $(DIRS) $(SRC)/cyclops/mesh_ops.h $(SRC)/cyclops/mesh_ops.cpp
	$(C) -o $(OBJ)/cyclops_mesh_ops.o $(SRC)/cyclops/mesh_ops.cpp

$(OBJ)/cyclops_comp_needle.o: $(DIRS) $(SRC)/cyclops/comp_needle.h $(SRC)/cyclops/comp_needle.cpp
	$(C) -o $(OBJ)/cyclops_comp_needle.o $(SRC)/cyclops/comp_needle.cpp

$(OBJ)/cyclops_sfs.o: $(DIRS) $(SRC)/cyclops/sfs.h $(SRC)/cyclops/sfs.cpp
	$(C) -o $(OBJ)/cyclops_sfs.o $(SRC)/cyclops/sfs.cpp

$(OBJ)/cyclops_integration.o: $(DIRS) $(SRC)/cyclops/integration.h $(SRC)/cyclops/integration.cpp
	$(C) -o $(OBJ)/cyclops_integration.o $(SRC)/cyclops/integration.cpp

$(OBJ)/cyclops_lighting.o: $(DIRS) $(SRC)/cyclops/lighting.h $(SRC)/cyclops/lighting.cpp
	$(C) -o $(OBJ)/cyclops_lighting.o $(SRC)/cyclops/lighting.cpp

$(OBJ)/cyclops_segmentation.o: $(DIRS) $(SRC)/cyclops/segmentation.h $(SRC)/cyclops/segmentation.cpp
	$(C) -o $(OBJ)/cyclops_segmentation.o $(SRC)/cyclops/segmentation.cpp

$(OBJ)/cyclops_light_est.o: $(DIRS) $(SRC)/cyclops/light_est.h $(SRC)/cyclops/light_est.cpp
	$(C) -o $(OBJ)/cyclops_light_est.o $(SRC)/cyclops/light_est.cpp



#############
# bp_stereo #
#############

FINAL_BP_STEREO	= $(OUT)/bp_stereo$(PEXT)
OBJS_BP_STEREO	= $(OBJ)/bp_stereo_main.o


bp_stereo: $(FINAL_BP_STEREO)

$(FINAL_BP_STEREO): $(OBJS_BP_STEREO)
	$(L_EXE) -o $(FINAL_BP_STEREO) $(OBJS_BP_STEREO) -L$(EOS_LIB) -leos


$(OBJ)/bp_stereo_main.o: $(DIRS) $(SRC)/bp_stereo/main.h $(SRC)/bp_stereo/main.cpp
	$(C) -o $(OBJ)/bp_stereo_main.o $(SRC)/bp_stereo/main.cpp



###########
# voronoi #
###########

FINAL_VORONOI	= $(OUT)/voronoi$(PEXT)
OBJS_VORONOI	= $(OBJ)/voronoi_main.o


voronoi: $(FINAL_VORONOI)

$(FINAL_VORONOI): $(OBJS_VORONOI)
	$(L_EXE_APP) -o $(FINAL_VORONOI) $(OBJS_VORONOI) -L$(EOS_LIB) -leos


$(OBJ)/voronoi_main.o: $(DIRS) $(SRC)/voronoi/main.h $(SRC)/voronoi/main.cpp
	$(C) -o $(OBJ)/voronoi_main.o $(SRC)/voronoi/main.cpp



############
# fg_test #
############

FINAL_FG_TEST	= $(OUT)/fg_test$(PEXT)
OBJS_FG_TEST	= $(OBJ)/fg_test_main.o


fg_test: $(FINAL_FG_TEST)

$(FINAL_FG_TEST): $(OBJS_FG_TEST)
	$(L_EXE) -o $(FINAL_FG_TEST) $(OBJS_FG_TEST) -L$(EOS_LIB) -leos


$(OBJ)/fg_test_main.o: $(DIRS) $(SRC)/fg_test/main.h $(SRC)/fg_test/main.cpp
	$(C) -o $(OBJ)/fg_test_main.o $(SRC)/fg_test/main.cpp



########
# sfgs #
########

FINAL_SFGS	= $(OUT)/sfgs$(PEXT)
OBJS_SFGS	= $(OBJ)/sfgs_main.o


sfgs: $(FINAL_SFGS)

$(FINAL_SFGS): $(OBJS_SFGS)
	$(L_EXE) -o $(FINAL_SFGS) $(OBJS_SFGS) -L$(EOS_LIB) -leos


$(OBJ)/sfgs_main.o: $(DIRS) $(SRC)/sfgs/main.h $(SRC)/sfgs/main.cpp
	$(C) -o $(OBJ)/sfgs_main.o $(SRC)/sfgs/main.cpp



#######
# svt #
#######

FINAL_SVT	= $(OUT)/svt$(PEXT)
OBJS_SVT	= $(OBJ)/svt_main.o


svt: $(FINAL_SVT)

$(FINAL_SVT): $(OBJS_SVT)
	$(L_EXE) -o $(FINAL_SVT) $(OBJS_SVT) -L$(EOS_LIB) -leos


$(OBJ)/svt_main.o: $(DIRS) $(SRC)/svt/main.h $(SRC)/svt/main.cpp
	$(C) -o $(OBJ)/svt_main.o $(SRC)/svt/main.cpp



########
# mser #
########

FINAL_MSER	= $(OUT)/mser$(PEXT)
OBJS_MSER	= $(OBJ)/mser_main.o


mser: $(FINAL_MSER)

$(FINAL_MSER): $(OBJS_MSER)
	$(L_EXE) -o $(FINAL_MSER) $(OBJS_MSER) -L$(EOS_LIB) -leos


$(OBJ)/mser_main.o: $(DIRS) $(SRC)/mser/main.h $(SRC)/mser/main.cpp
	$(C) -o $(OBJ)/mser_main.o $(SRC)/mser/main.cpp



##########
# orient #
##########

FINAL_ORIENT	= $(OUT)/orient$(PEXT)
OBJS_ORIENT	= $(OBJ)/orient_main.o


orient: $(FINAL_ORIENT)

$(FINAL_ORIENT): $(OBJS_ORIENT)
	$(L_EXE) -o $(FINAL_ORIENT) $(OBJS_ORIENT) -L$(EOS_LIB) -leos


$(OBJ)/orient_main.o: $(DIRS) $(SRC)/orient/main.h $(SRC)/orient/main.cpp
	$(C) -o $(OBJ)/orient_main.o $(SRC)/orient/main.cpp



##############
# sur_stereo #
##############

FINAL_SUR_STEREO	= $(OUT)/sur_stereo$(PEXT)
OBJS_SUR_STEREO		= $(OBJ)/sur_stereo_main.o


sur_stereo: $(FINAL_SUR_STEREO)

$(FINAL_SUR_STEREO): $(OBJS_SUR_STEREO)
	$(L_EXE) -o $(FINAL_SUR_STEREO) $(OBJS_SUR_STEREO) -L$(EOS_LIB) -leos


$(OBJ)/sur_stereo_main.o: $(DIRS) $(SRC)/sur_stereo/main.h $(SRC)/sur_stereo/main.cpp
	$(C) -o $(OBJ)/sur_stereo_main.o $(SRC)/sur_stereo/main.cpp



############
# sur_test #
############

FINAL_SUR_TEST	= $(OUT)/sur_test$(PEXT)
OBJS_SUR_TEST	= $(OBJ)/sur_test_main.o


sur_test: $(FINAL_SUR_TEST)

$(FINAL_SUR_TEST): $(OBJS_SUR_TEST)
	$(L_EXE) -o $(FINAL_SUR_TEST) $(OBJS_SUR_TEST) -L$(EOS_LIB) -leos


$(OBJ)/sur_test_main.o: $(DIRS) $(SRC)/sur_test/main.h $(SRC)/sur_test/main.cpp
	$(C) -o $(OBJ)/sur_test_main.o $(SRC)/sur_test/main.cpp



############
# spec_rem #
############

FINAL_SPEC_REM	= $(OUT)/spec_rem$(PEXT)
OBJS_SPEC_REM	= $(OBJ)/spec_rem_main.o


spec_rem: $(FINAL_SPEC_REM)

$(FINAL_SPEC_REM): $(OBJS_SPEC_REM)
	$(L_EXE) -o $(FINAL_SPEC_REM) $(OBJS_SPEC_REM) -L$(EOS_LIB) -leos


$(OBJ)/spec_rem_main.o: $(DIRS) $(SRC)/spec_rem/main.h $(SRC)/spec_rem/main.cpp
	$(C) -o $(OBJ)/spec_rem_main.o $(SRC)/spec_rem/main.cpp



###########
# ds_test #
###########

FINAL_DS_TEST	= $(OUT)/ds_test$(PEXT)
OBJS_DS_TEST	= $(OBJ)/ds_test_main.o


ds_test: $(FINAL_DS_TEST)

$(FINAL_DS_TEST): $(OBJS_DS_TEST)
	$(L_EXE) -o $(FINAL_DS_TEST) $(OBJS_DS_TEST) -L$(EOS_LIB) -leos


$(OBJ)/ds_test_main.o: $(DIRS) $(SRC)/ds_test/main.h $(SRC)/ds_test/main.cpp
	$(C) -o $(OBJ)/ds_test_main.o $(SRC)/ds_test/main.cpp



#################
# sur_bp_stereo #
#################

FINAL_SUR_BP_STEREO	= $(OUT)/sur_bp_stereo$(PEXT)
OBJS_SUR_BP_STEREO	= $(OBJ)/sur_bp_stereo_main.o


sur_bp_stereo: $(FINAL_SUR_BP_STEREO)

$(FINAL_SUR_BP_STEREO): $(OBJS_SUR_BP_STEREO)
	$(L_EXE) -o $(FINAL_SUR_BP_STEREO) $(OBJS_SUR_BP_STEREO) -L$(EOS_LIB) -leos


$(OBJ)/sur_bp_stereo_main.o: $(DIRS) $(SRC)/sur_bp_stereo/main.h $(SRC)/sur_bp_stereo/main.cpp
	$(C) -o $(OBJ)/sur_bp_stereo_main.o $(SRC)/sur_bp_stereo/main.cpp



########
# smes #
########

FINAL_SMES	= $(OUT)/smes$(PEXT)
OBJS_SMES	= $(OBJ)/smes_main.o


smes: $(FINAL_SMES)

$(FINAL_SMES): $(OBJS_SMES)
	$(L_EXE) -o $(FINAL_SMES) $(OBJS_SMES) -L$(EOS_LIB) -leos


$(OBJ)/smes_main.o: $(DIRS) $(SRC)/smes/main.h $(SRC)/smes/main.cpp
	$(C) -o $(OBJ)/smes_main.o $(SRC)/smes/main.cpp



###############
# text_to_svt #
###############

FINAL_TEXT_TO_SVT	= $(OUT)/text_to_svt$(PEXT)
OBJS_TEXT_TO_SVT	= $(OBJ)/text_to_svt_main.o


text_to_svt: $(FINAL_TEXT_TO_SVT)

$(FINAL_TEXT_TO_SVT): $(OBJS_TEXT_TO_SVT)
	$(L_EXE) -o $(FINAL_TEXT_TO_SVT) $(OBJS_TEXT_TO_SVT) -L$(EOS_LIB) -leos


$(OBJ)/text_to_svt_main.o: $(DIRS) $(SRC)/text_to_svt/main.h $(SRC)/text_to_svt/main.cpp
	$(C) -o $(OBJ)/text_to_svt_main.o $(SRC)/text_to_svt/main.cpp



##########
# bessel #
##########

FINAL_BESSEL	= $(OUT)/bessel$(PEXT)
OBJS_BESSEL	= $(OBJ)/bessel_main.o


bessel: $(FINAL_BESSEL)

$(FINAL_BESSEL): $(OBJS_BESSEL)
	$(L_EXE) -o $(FINAL_BESSEL) $(OBJS_BESSEL) -L$(EOS_LIB) -leos


$(OBJ)/bessel_main.o: $(DIRS) $(SRC)/bessel/main.h $(SRC)/bessel/main.cpp
	$(C) -o $(OBJ)/bessel_main.o $(SRC)/bessel/main.cpp



############
# fishbing #
############

FINAL_FISHBING	= $(OUT)/fishbing$(PEXT)
OBJS_FISHBING	= $(OBJ)/fishbing_main.o


fishbing: $(FINAL_FISHBING)

$(FINAL_FISHBING): $(OBJS_FISHBING)
	$(L_EXE) -o $(FINAL_FISHBING) $(OBJS_FISHBING) -L$(EOS_LIB) -leos


$(OBJ)/fishbing_main.o: $(DIRS) $(SRC)/fishbing/main.h $(SRC)/fishbing/main.cpp
	$(C) -o $(OBJ)/fishbing_main.o $(SRC)/fishbing/main.cpp



##########
# sfs_bp #
##########

FINAL_SFS_BP	= $(OUT)/sfs_bp$(PEXT)
OBJS_SFS_BP	= $(OBJ)/sfs_bp_main.o


sfs_bp: $(FINAL_SFS_BP)

$(FINAL_SFS_BP): $(OBJS_SFS_BP)
	$(L_EXE) -o $(FINAL_SFS_BP) $(OBJS_SFS_BP) -L$(EOS_LIB) -leos


$(OBJ)/sfs_bp_main.o: $(DIRS) $(SRC)/sfs_bp/main.h $(SRC)/sfs_bp/main.cpp
	$(C) -o $(OBJ)/sfs_bp_main.o $(SRC)/sfs_bp/main.cpp



###########
# mg_test #
###########

FINAL_MG_TEST	= $(OUT)/mg_test$(PEXT)
OBJS_MG_TEST	= $(OBJ)/mg_test_main.o


mg_test: $(FINAL_MG_TEST)

$(FINAL_MG_TEST): $(OBJS_MG_TEST)
	$(L_EXE) -o $(FINAL_MG_TEST) $(OBJS_MG_TEST) -L$(EOS_LIB) -leos


$(OBJ)/mg_test_main.o: $(DIRS) $(SRC)/mg_test/main.h $(SRC)/mg_test/main.cpp
	$(C) -o $(OBJ)/mg_test_main.o $(SRC)/mg_test/main.cpp



###########
# bp_test #
###########

FINAL_BP_TEST	= $(OUT)/bp_test$(PEXT)
OBJS_BP_TEST	= $(OBJ)/bp_test_main.o


bp_test: $(FINAL_BP_TEST)

$(FINAL_BP_TEST): $(OBJS_BP_TEST)
	$(L_EXE) -o $(FINAL_BP_TEST) $(OBJS_BP_TEST) -L$(EOS_LIB) -leos


$(OBJ)/bp_test_main.o: $(DIRS) $(SRC)/bp_test/main.h $(SRC)/bp_test/main.cpp
	$(C) -o $(OBJ)/bp_test_main.o $(SRC)/bp_test/main.cpp



########
# mscr #
########

FINAL_MSCR	= $(OUT)/mscr$(PEXT)
OBJS_MSCR	= $(OBJ)/mscr_main.o


mscr: $(FINAL_MSCR)

$(FINAL_MSCR): $(OBJS_MSCR)
	$(L_EXE) -o $(FINAL_MSCR) $(OBJS_MSCR) -L$(EOS_LIB) -leos


$(OBJ)/mscr_main.o: $(DIRS) $(SRC)/mscr/main.h $(SRC)/mscr/main.cpp
	$(C) -o $(OBJ)/mscr_main.o $(SRC)/mscr/main.cpp



############
# geo_test #
############

FINAL_GEO_TEST	= $(OUT)/geo_test$(PEXT)
OBJS_GEO_TEST	= $(OBJ)/geo_test_main.o


geo_test: $(FINAL_GEO_TEST)

$(FINAL_GEO_TEST): $(OBJS_GEO_TEST)
	$(L_EXE) -o $(FINAL_GEO_TEST) $(OBJS_GEO_TEST) -L$(EOS_LIB) -leos


$(OBJ)/geo_test_main.o: $(DIRS) $(SRC)/geo_test/main.h $(SRC)/geo_test/main.cpp
	$(C) -o $(OBJ)/geo_test_main.o $(SRC)/geo_test/main.cpp



###############
# Auxilary... #
###############

$(DIRS):
	-$(MKDIR) $(OBJ_BASE)
	-$(MKDIR) $(OBJ)
	-$(MKDIR) $(OUT_BASE)
	-$(MKDIR) $(OUT)
	touch $(DIRS)


prep_final: ${DIRS}
	-cp -d $(EOS_LIB)/*.* $(OUT)
	-cp -R $(DEP)/* $(OUT)
	#-cp /usr/local/lib/libIL.so $(OUT) # For my x64 setup.
	@echo Extra files copied in

line_count: $(EXES) prep_final
	@echo Line Count:
	@find src -exec cat {} \; -name "*.h" -or -name "*.cpp" 2> /dev/null | grep -c ""



###################
# End of makefile #
###################
