
#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>

static char * read_file (const char *filename);
void output_code (OrcProgram *p, FILE *output);
void output_code_header (OrcProgram *p, FILE *output);
void output_code_test (OrcProgram *p, FILE *output);

int error = FALSE;

double weights_ginger[];
double weights_feathers[];
//double weights_preston[];
double weights_n900[];

int
main (int argc, char *argv[])
{
  char *code;
  int n;
  int i;
  OrcProgram **programs;
  const char *filename = NULL;
  double sum;

  orc_init ();
  orc_test_init ();

  filename = "bench10.orc";
  code = read_file (filename);
  if (!code) {
    printf("benchmorc needs bench10.orc file in current directory\n");
    exit(1);
  }

  n = orc_parse (code, &programs);

#if 0
  sum = 0;
  for(i=0;i<n;i++){
    double perf;
    double weight;

    perf = orc_test_performance_full (programs[i], 0, NULL);

    if (perf == 0) {
      weight = 0;
    } else {
      weight = 1.0/perf/281.0;
      sum++;
    }
    printf(" %g, /* %s */\n", weight, programs[i]->name);
  }
  printf("sum = %g\n", sum);
#else
  sum = 0;
  for(i=0;i<n;i++){
    double perf;
    double weight;

    perf = orc_test_performance_full (programs[i], 0, NULL);
    //weight = weights_ginger[i];
    weight = weights_feathers[i];
    //weight = weights_n900[i];

    sum += weight * perf;
  }
  printf("score %g\n", 100.0/sum);
#endif

  if (error) return 1;
  return 0;
}


static char *
read_file (const char *filename)
{
  FILE *file = NULL;
  char *contents = NULL;
  long size;
  int ret;

  file = fopen (filename, "r");
  if (file == NULL) return NULL;

  ret = fseek (file, 0, SEEK_END);
  if (ret < 0) goto bail;

  size = ftell (file);
  if (size < 0) goto bail;

  ret = fseek (file, 0, SEEK_SET);
  if (ret < 0) goto bail;

  contents = malloc (size + 1);
  if (contents == NULL) goto bail;

  ret = fread (contents, size, 1, file);
  if (ret < 0) goto bail;

  contents[size] = 0;

  return contents;
bail:
  /* something failed */
  if (file) fclose (file);
  if (contents) free (contents);

  return NULL;
}



/* tables */

/* ginger Intel(R) Core(TM)2 CPU         T7600  @ 2.33GHz */

double weights_ginger[] = {
 0.00539898, /* orc_scalarmultiply_f32_ns */
 0.00173034, /* orc_process_int16 */
 0.00229296, /* orc_process_int16_clamp */
 0.00238334, /* orc_process_int8 */
 0.00286, /* orc_process_int8_clamp */
 0.00224671, /* orc_audio_convert_unpack_u8 */
 0.00223485, /* orc_audio_convert_unpack_s8 */
 0.00261931, /* orc_audio_convert_unpack_u16 */
 0.0026756, /* orc_audio_convert_unpack_s16 */
 0.00187791, /* orc_audio_convert_unpack_u16_swap */
 0.00188964, /* orc_audio_convert_unpack_s16_swap */
 0.0018846, /* orc_audio_convert_unpack_u32 */
 0.00208672, /* orc_audio_convert_unpack_s32 */
 0.00158413, /* orc_audio_convert_unpack_u32_swap */
 0.0016592, /* orc_audio_convert_unpack_s32_swap */
 0.00113724, /* orc_audio_convert_unpack_float_s32 */
 0.000966394, /* orc_audio_convert_unpack_float_s32_swap */
 0.00163051, /* orc_audio_convert_unpack_float_double */
 0.00129049, /* orc_audio_convert_unpack_float_double_swap */
 0.00328124, /* orc_audio_convert_unpack_double_double */
 0.0019506, /* orc_audio_convert_unpack_double_double_swap */
 0.000854422, /* orc_audio_convert_unpack_u8_double */
 0.000841177, /* orc_audio_convert_unpack_s8_double */
 0.0013211, /* orc_audio_convert_unpack_u16_double */
 0.0012878, /* orc_audio_convert_unpack_s16_double */
 0.000888871, /* orc_audio_convert_unpack_u16_double_swap */
 0.00113332, /* orc_audio_convert_unpack_s16_double_swap */
 0.00125976, /* orc_audio_convert_unpack_u32_double */
 0.00204625, /* orc_audio_convert_unpack_s32_double */
 0.0010244, /* orc_audio_convert_unpack_u32_double_swap */
 0.00148207, /* orc_audio_convert_unpack_s32_double_swap */
 0.00135233, /* orc_audio_convert_pack_u8 */
 0.0013869, /* orc_audio_convert_pack_s8 */
 0.0021164, /* orc_audio_convert_pack_u16 */
 0.00211852, /* orc_audio_convert_pack_s16 */
 0.00183715, /* orc_audio_convert_pack_u16_swap */
 0.00200417, /* orc_audio_convert_pack_s16_swap */
 0.00193889, /* orc_audio_convert_pack_u32 */
 0.00208307, /* orc_audio_convert_pack_s32 */
 0.00159261, /* orc_audio_convert_pack_u32_swap */
 0.00167437, /* orc_audio_convert_pack_s32_swap */
 0.00143194, /* orc_audio_convert_pack_s32_float */
 0.00118178, /* orc_audio_convert_pack_s32_float_swap */
 0.00268428, /* orc_audio_convert_pack_double_float */
 0.0014616, /* orc_audio_convert_pack_double_float_swap */
 0.000483737, /* orc_audio_convert_pack_double_s8 */
 0.000686549, /* orc_audio_convert_pack_double_s16 */
 0.000577306, /* orc_audio_convert_pack_double_s16_swap */
 0.00100781, /* orc_audio_convert_pack_double_s32 */
 0.000784434, /* orc_audio_convert_pack_double_s32_swap */
 0.0172065, /* gst_orc_splat_u8 */
 0.0121632, /* gst_orc_splat_s16 */
 0.01221, /* gst_orc_splat_u16 */
 0.00740001, /* gst_orc_splat_u32 */
 0.00205476, /* orc_merge_linear_u8 */
 0.000841177, /* orc_merge_linear_u16 */
 0.0120937, /* orc_splat_u16 */
 0.00740001, /* orc_splat_u32 */
 0.00367431, /* orc_downsample_u8 */
 0.00148717, /* orc_downsample_u16 */
 0.00182448, /* gst_videoscale_orc_downsample_u32 */
 0.000360484, /* gst_videoscale_orc_downsample_yuyv */
 0, /* gst_videoscale_orc_resample_nearest_u8 */
 0, /* gst_videoscale_orc_resample_bilinear_u8 */
 0.0011403, /* gst_videoscale_orc_resample_nearest_u32 */
 0.000404898, /* gst_videoscale_orc_resample_bilinear_u32 */
 0.00027372, /* gst_videoscale_orc_resample_merge_bilinear_u32 */
 0.000888871, /* gst_videoscale_orc_merge_bicubic_u8 */
 0.00122619, /* add_int32 */
 0.0044839, /* add_int16 */
 0.00820311, /* add_int8 */
 0.00138106, /* add_uint32 */
 0.0044839, /* add_uint16 */
 0.00807787, /* add_uint8 */
 0.00177998, /* add_float32 */
 0.00737422, /* orc_splat_u32 */
 0.00221845, /* orc_memcpy_u32 */
 0.00294308, /* orc_blend_u8 */
 0.000216663, /* orc_blend_argb */
 0.000213342, /* orc_blend_bgra */
 0.00737422, /* orc_splat_u32 */
 0.000987128, /* deinterlace_line_vfir */
 0.00386205, /* deinterlace_line_linear */
 0.00178901, /* deinterlace_line_linear_blend */
 0, /* deinterlace_line_greedy */
 0.00824266, /* cogorc_memcpy_2d */
 0.00483197, /* cogorc_downsample_horiz_cosite_1tap */
 0.00139512, /* cogorc_downsample_horiz_cosite_3tap */
 0.00157236, /* cogorc_downsample_420_jpeg */
 0.00387619, /* cogorc_downsample_vert_halfsite_2tap */
 0.00175199, /* cogorc_downsample_vert_cosite_3tap */
 0.00131699, /* cogorc_downsample_vert_halfsite_4tap */
 0.00594495, /* cogorc_upsample_horiz_cosite_1tap */
 0.003575, /* cogorc_upsample_horiz_cosite */
 0.00371299, /* cogorc_upsample_vert_avgub */
 0.00495645, /* orc_unpack_yuyv_y */
 0.00172345, /* orc_unpack_yuyv_u */
 0.0019524, /* orc_unpack_yuyv_v */
 0.000834873, /* orc_pack_yuyv */
 0.0059283, /* orc_unpack_uyvy_y */
 0.0015303, /* orc_unpack_uyvy_u */
 0.00172345, /* orc_unpack_uyvy_v */
 0.000837184, /* orc_pack_uyvy */
 0.00245522, /* orc_matrix2_u8 */
 0, /* orc_matrix2_11_u8 */
 0, /* orc_matrix2_12_u8 */
 0.00145158, /* orc_matrix3_u8 */
 0, /* orc_matrix3_100_u8 */
 0.00138146, /* orc_matrix3_100_offset_u8 */
 0.00134289, /* orc_matrix3_000_u8 */
 0.00164957, /* orc_pack_123x */
 0.00160943, /* orc_pack_x123 */
 0.00254681, /* cogorc_combine2_u8 */
 0.000633112, /* cogorc_convert_I420_UYVY */
 0.000635284, /* cogorc_convert_I420_YUY2 */
 0.000541003, /* cogorc_convert_I420_AYUV */
 0.00055886, /* cogorc_convert_YUY2_I420 */
 0.00187425, /* cogorc_convert_UYVY_YUY2 */
 0.00277488, /* cogorc_planar_chroma_420_422 */
 0.00147587, /* cogorc_planar_chroma_420_444 */
 0.00624044, /* cogorc_planar_chroma_422_444 */
 0.0038428, /* cogorc_planar_chroma_444_422 */
 0.00203705, /* cogorc_planar_chroma_444_420 */
 0.00434352, /* cogorc_planar_chroma_422_420 */
 0.0005254, /* cogorc_convert_YUY2_AYUV */
 0.000538103, /* cogorc_convert_UYVY_AYUV */
 0.00100143, /* cogorc_convert_YUY2_Y42B */
 0.00103223, /* cogorc_convert_UYVY_Y42B */
 0.000770722, /* cogorc_convert_YUY2_Y444 */
 0.000778117, /* cogorc_convert_UYVY_Y444 */
 0.00055886, /* cogorc_convert_UYVY_I420 */
 0, /* cogorc_convert_AYUV_I420 */
 0.000341915, /* cogorc_convert_AYUV_YUY2 */
 0.000348717, /* cogorc_convert_AYUV_UYVY */
 0.000312522, /* cogorc_convert_AYUV_Y42B */
 0.000873979, /* cogorc_convert_AYUV_Y444 */
 0.00161992, /* cogorc_convert_Y42B_YUY2 */
 0.00165063, /* cogorc_convert_Y42B_UYVY */
 0.000588503, /* cogorc_convert_Y42B_AYUV */
 0.000805998, /* cogorc_convert_Y444_YUY2 */
 0.000815565, /* cogorc_convert_Y444_UYVY */
 0.0014937, /* cogorc_convert_Y444_AYUV */
 0, /* cogorc_convert_AYUV_ARGB */
 0, /* cogorc_convert_AYUV_BGRA */
 0, /* cogorc_convert_AYUV_ABGR */
 0, /* cogorc_convert_AYUV_RGBA */
 0, /* cogorc_convert_I420_BGRA */
 0, /* cogorc_convert_I420_BGRA_avg */
 0.000819403, /* cogorc_getline_I420 */
 0.00057497, /* cogorc_getline_YUY2 */
 0.000551146, /* cogorc_getline_UYVY */
 0.000562873, /* cogorc_getline_YVYU */
 0.000552297, /* cogorc_getline_Y42B */
 0.00156539, /* cogorc_getline_Y444 */
 0.00260962, /* cogorc_getline_Y800 */
 0.00185722, /* cogorc_getline_BGRA */
 0.000454554, /* cogorc_getline_ABGR */
 0.000452223, /* cogorc_getline_RGBA */
 0.000903587, /* cogorc_getline_NV12 */
 0.000800455, /* cogorc_getline_NV21 */
 0.000286517, /* cogorc_putline_I420 */
 0.000349472, /* cogorc_putline_YUY2 */
 0.000391564, /* cogorc_putline_YVYU */
 0.00035675, /* cogorc_putline_UYVY */
 0.000320473, /* cogorc_putline_Y42B */
 0.000845209, /* cogorc_putline_Y444 */
 0.00172426, /* cogorc_putline_Y800 */
 0.00185867, /* cogorc_putline_BGRA */
 0.000454814, /* cogorc_putline_ABGR */
 0.000451794, /* cogorc_putline_RGBA */
 0.000369853, /* cogorc_putline_NV12 */
 0.000414899, /* cogorc_putline_NV21 */
 0.00161681, /* orc_add2_rshift_add_s16_22_op */
 0.00209961, /* orc_add2_rshift_add_s16_22 */
 0.00161557, /* orc_add2_rshift_sub_s16_22_op */
 0.00209961, /* orc_add2_rshift_sub_s16_22 */
 0.00156654, /* orc_add2_rshift_add_s16_11_op */
 0.00201776, /* orc_add2_rshift_add_s16_11 */
 0.00156654, /* orc_add2_rshift_sub_s16_11_op */
 0.00205078, /* orc_add2_rshift_sub_s16_11 */
 0.00428421, /* orc_add_const_rshift_s16_11 */
 0.00863838, /* orc_add_const_rshift_s16 */
 0.00281436, /* orc_add_s16 */
 0.00372344, /* orc_add_s16_2d */
 0.0022491, /* orc_addc_rshift_s16 */
 0.00466168, /* orc_lshift1_s16 */
 0.00469616, /* orc_lshift2_s16 */
 0.010175, /* orc_lshift_s16_ip */
 0.000855112, /* orc_mas2_add_s16_op */
 0.000850644, /* orc_mas2_add_s16_ip */
 0.000853044, /* orc_mas2_sub_s16_op */
 0.000851328, /* orc_mas2_sub_s16_ip */
 0.000639672, /* orc_mas4_across_add_s16_1991_op */
 0.000673799, /* orc_mas4_across_add_s16_1991_ip */
 0.000640558, /* orc_mas4_across_sub_s16_1991_op */
 0.000673584, /* orc_mas4_across_sub_s16_1991_ip */
 0.00275573, /* orc_subtract_s16 */
 0.00267222, /* orc_add_s16_u8 */
 0.00544385, /* orc_add_s16_u8_2d */
 0.00653211, /* orc_convert_s16_u8 */
 0.00678334, /* orc_convert_u8_s16 */
 0.00689382, /* orc_offsetconvert_u8_s16 */
 0.00584641, /* orc_offsetconvert_s16_u8 */
 0.00273437, /* orc_subtract_s16_u8 */
 0.00238334, /* orc_multiply_and_add_s16_u8 */
 0.012025, /* orc_splat_s16_ns */
 0.000952381, /* orc_splat_s16_2d_4xn */
 0.00172953, /* orc_splat_s16_2d_8xn */
 0.0121284, /* orc_splat_s16_2d */
 0.0176367, /* orc_splat_u8_ns */
 0.0286862, /* orc_splat_u8_2d */
 0.00372606, /* orc_average_u8 */
 0.00242965, /* orc_rrshift6_add_s16_2d */
 0.00107453, /* orc_rrshift6_sub_s16_2d */
 0.0139063, /* orc_rrshift6_s16_ip_2d */
 0.011565, /* orc_rrshift6_s16_ip */
 0.00493334, /* orc_unpack_yuyv_y */
 0.00171647, /* orc_unpack_yuyv_u */
 0.00194363, /* orc_unpack_yuyv_v */
 0.000834543, /* orc_packyuyv */
 0.00591174, /* orc_unpack_uyvy_y */
 0.00153214, /* orc_unpack_uyvy_u */
 0.00172627, /* orc_unpack_uyvy_v */
 0.0029935, /* orc_interleave2_s16 */
 0.00201094, /* orc_interleave2_rrshift1_s16 */
 0.00140531, /* orc_deinterleave2_s16 */
 0.0011584, /* orc_deinterleave2_lshift1_s16 */
 0.00131151, /* orc_haar_deint_lshift1_split_s16 */
 0.00136132, /* orc_haar_deint_split_s16 */
 0.00202333, /* orc_haar_split_s16_lo */
 0.00278474, /* orc_haar_split_s16_hi */
 0.00127417, /* orc_haar_split_s16_op */
 0.00123128, /* orc_haar_split_s16 */
 0.0020668, /* orc_haar_synth_s16_lo */
 0.00175199, /* orc_haar_synth_s16_hi */
 0.00124788, /* orc_haar_synth_s16_op */
 0.0010582, /* orc_haar_synth_s16 */
 0.00130965, /* orc_haar_synth_rrshift1_int_s16 */
 0.00173192, /* orc_haar_synth_int_s16 */
 0.00444622, /* orc_haar_sub_s16 */
 0.00300625, /* orc_haar_add_half_s16 */
 0.00446498, /* orc_haar_add_s16 */
 0.00297619, /* orc_haar_sub_half_s16 */
 0.00296415, /* orc_sum_u8 */
 0.00353913, /* orc_sum_s16 */
 0.00195601, /* orc_sum_square_diff_u8 */
 0.000907029, /* orc_dequantise_s16_2d_4xn */
 0.0010582, /* orc_dequantise_s16_2d_8xn */
 0.0010582, /* orc_dequantise_s16_ip_2d_8xn */
 0.00607507, /* orc_dequantise_s16_ip_2d */
 0.00544062, /* orc_dequantise_s16_ip */
 0.002788, /* orc_dequantise_s16 */
 0.00177551, /* orc_dequantise_var_s16_ip */
 0.00250462, /* orc_quantise1_s16 */
 0.00286, /* orc_quantise2_s16 */
 0.000653211, /* orc_quantdequant1_s16 */
 0.00033615, /* orc_quantdequant3_s16 */
 0.000700332, /* orc_quantdequant2_s16 */
 0.00117709, /* orc_downsample_vert_u8 */
 0.000427844, /* orc_downsample_horiz_u8 */
 0.00341907, /* orc_stats_moment_s16 */
 0.00300625, /* orc_stats_above_s16 */
 0.012025, /* orc_accw */
 0.000979968, /* orc_avg2_8xn_u8 */
 0.000970018, /* orc_avg2_12xn_u8 */
 0.0012075, /* orc_avg2_16xn_u8 */
 0.00257509, /* orc_avg2_32xn_u8 */
 0.0042909, /* orc_avg2_nxm_u8 */
 0.00106312, /* orc_combine4_8xn_u8 */
 0.00116315, /* orc_combine4_12xn_u8 */
 0.00147629, /* orc_combine4_16xn_u8 */
 0.0013289, /* orc_combine4_24xn_u8 */
 0.00158025, /* orc_combine4_32xn_u8 */
 0.00119665, /* orc_combine4_nxm_u8 */
 0.00118816, /* orc_combine2_8xn_u8 */
 0.00167189, /* orc_combine2_12xn_u8 */
 0.00178851, /* orc_combine2_16xn_u8 */
 0.00229296, /* orc_combine2_nxm_u8 */
 0.00304518, /* orc_sad_nxm_u8 */
 0.0010836, /* orc_sad_8x8_u8 */
 0.00171215, /* orc_sad_12x12_u8 */
 0.00228437, /* orc_sad_16xn_u8 */
 0.00310406, /* orc_sad_32xn_u8 */
};

double weights_feathers[] = {
 0.00176674, /* orc_scalarmultiply_f32_ns */
 0.000896403, /* orc_process_int16 */
 0.00102409, /* orc_process_int16_clamp */
 0.00175739, /* orc_process_int8 */
 0.00199928, /* orc_process_int8_clamp */
 0.00128416, /* orc_audio_convert_unpack_u8 */
 0.00124322, /* orc_audio_convert_unpack_s8 */
 0.00124922, /* orc_audio_convert_unpack_u16 */
 0.000936916, /* orc_audio_convert_unpack_s16 */
 0.000697959, /* orc_audio_convert_unpack_u16_swap */
 0.000734324, /* orc_audio_convert_unpack_s16_swap */
 0.000839816, /* orc_audio_convert_unpack_u32 */
 0.000816487, /* orc_audio_convert_unpack_s32 */
 0.000634514, /* orc_audio_convert_unpack_u32_swap */
 0.000595246, /* orc_audio_convert_unpack_s32_swap */
 0.000487162, /* orc_audio_convert_unpack_float_s32 */
 0.000428761, /* orc_audio_convert_unpack_float_s32_swap */
 0.000618504, /* orc_audio_convert_unpack_float_double */
 0.000436194, /* orc_audio_convert_unpack_float_double_swap */
 0.000732247, /* orc_audio_convert_unpack_double_double */
 0.000467868, /* orc_audio_convert_unpack_double_double_swap */
 0.000278073, /* orc_audio_convert_unpack_u8_double */
 0.000401095, /* orc_audio_convert_unpack_s8_double */
 0.00034272, /* orc_audio_convert_unpack_u16_double */
 0.000389783, /* orc_audio_convert_unpack_s16_double */
 0.000251722, /* orc_audio_convert_unpack_u16_double_swap */
 0.000287167, /* orc_audio_convert_unpack_s16_double_swap */
 0.000300853, /* orc_audio_convert_unpack_u32_double */
 0.000451471, /* orc_audio_convert_unpack_s32_double */
 0.000260903, /* orc_audio_convert_unpack_u32_double_swap */
 0.000299304, /* orc_audio_convert_unpack_s32_double_swap */
 0.00102965, /* orc_audio_convert_pack_u8 */
 0.00077765, /* orc_audio_convert_pack_s8 */
 0.00122714, /* orc_audio_convert_pack_u16 */
 0.00130956, /* orc_audio_convert_pack_s16 */
 0.000746063, /* orc_audio_convert_pack_u16_swap */
 0.000710323, /* orc_audio_convert_pack_s16_swap */
 0.000892469, /* orc_audio_convert_pack_u32 */
 0.000816487, /* orc_audio_convert_pack_s32 */
 0.000664095, /* orc_audio_convert_pack_u32_swap */
 0.000595103, /* orc_audio_convert_pack_s32_swap */
 0.000182346, /* orc_audio_convert_pack_s32_float */
 0.000173448, /* orc_audio_convert_pack_s32_float_swap */
 0.000426486, /* orc_audio_convert_pack_double_float */
 0.000385769, /* orc_audio_convert_pack_double_float_swap */
 0.000191034, /* orc_audio_convert_pack_double_s8 */
 0.000282214, /* orc_audio_convert_pack_double_s16 */
 0.000176126, /* orc_audio_convert_pack_double_s16_swap */
 0.000307117, /* orc_audio_convert_pack_double_s32 */
 0.00025851, /* orc_audio_convert_pack_double_s32_swap */
 0.0111646, /* gst_orc_splat_u8 */
 0.00699502, /* gst_orc_splat_s16 */
 0.00691013, /* gst_orc_splat_u16 */
 0.00413805, /* gst_orc_splat_u32 */
 0.001056, /* orc_merge_linear_u8 */
 0.000460378, /* orc_merge_linear_u16 */
 0.00480908, /* orc_splat_u16 */
 0.00413805, /* orc_splat_u32 */
 0.00247563, /* orc_downsample_u8 */
 0.0012043, /* orc_downsample_u16 */
 0.000941771, /* gst_videoscale_orc_downsample_u32 */
 0.000243822, /* gst_videoscale_orc_downsample_yuyv */
 0, /* gst_videoscale_orc_resample_nearest_u8 */
 0, /* gst_videoscale_orc_resample_bilinear_u8 */
 0.000419784, /* gst_videoscale_orc_resample_nearest_u32 */
 0.000205113, /* gst_videoscale_orc_resample_bilinear_u32 */
 9.11178e-05, /* gst_videoscale_orc_resample_merge_bilinear_u32 */
 0.000559767, /* gst_videoscale_orc_merge_bicubic_u8 */
 0.000731494, /* add_int32 */
 0.00227032, /* add_int16 */
 0.00425557, /* add_int8 */
 0.000843298, /* add_uint32 */
 0.00228123, /* add_uint16 */
 0.00429408, /* add_uint8 */
 0.000779444, /* add_float32 */
 0.00287573, /* orc_splat_u32 */
 0.00143497, /* orc_memcpy_u32 */
 0.00156706, /* orc_blend_u8 */
 0.000170433, /* orc_blend_argb */
 0.000155691, /* orc_blend_bgra */
 0.00416224, /* orc_splat_u32 */
 0.000828572, /* deinterlace_line_vfir */
 0.00311485, /* deinterlace_line_linear */
 0.00141079, /* deinterlace_line_linear_blend */
 0.00111036, /* deinterlace_line_greedy */
 0.0047578, /* cogorc_memcpy_2d */
 0.00336522, /* cogorc_downsample_horiz_cosite_1tap */
 0.000725899, /* cogorc_downsample_horiz_cosite_3tap */
 0.00103451, /* cogorc_downsample_420_jpeg */
 0.00312168, /* cogorc_downsample_vert_halfsite_2tap */
 0.00117741, /* cogorc_downsample_vert_cosite_3tap */
 0.000738761, /* cogorc_downsample_vert_halfsite_4tap */
 0.00351851, /* cogorc_upsample_horiz_cosite_1tap */
 0.00271658, /* cogorc_upsample_horiz_cosite */
 0.00310466, /* cogorc_upsample_vert_avgub */
 0.00338926, /* orc_unpack_yuyv_y */
 0.00159584, /* orc_unpack_yuyv_u */
 0.00177936, /* orc_unpack_yuyv_v */
 0.000771956, /* orc_pack_yuyv */
 0.00385247, /* orc_unpack_uyvy_y */
 0.00143714, /* orc_unpack_uyvy_u */
 0.00127476, /* orc_unpack_uyvy_v */
 0.000771956, /* orc_pack_uyvy */
 0.00136874, /* orc_matrix2_u8 */
 0.001006, /* orc_matrix2_11_u8 */
 0.000972328, /* orc_matrix2_12_u8 */
 0.000732435, /* orc_matrix3_u8 */
 0.000748613, /* orc_matrix3_100_u8 */
 0.000790826, /* orc_matrix3_100_offset_u8 */
 0.000787763, /* orc_matrix3_000_u8 */
 0.00123352, /* orc_pack_123x */
 0.00117449, /* orc_pack_x123 */
 0.00143859, /* cogorc_combine2_u8 */
 0.000452955, /* cogorc_convert_I420_UYVY */
 0.000638478, /* cogorc_convert_I420_YUY2 */
 0.000578653, /* cogorc_convert_I420_AYUV */
 0.000415819, /* cogorc_convert_YUY2_I420 */
 0.000881131, /* cogorc_convert_UYVY_YUY2 */
 0.00143278, /* cogorc_planar_chroma_420_422 */
 0.00123156, /* cogorc_planar_chroma_420_444 */
 0.00356485, /* cogorc_planar_chroma_422_444 */
 0.0021052, /* cogorc_planar_chroma_444_422 */
 0.000850432, /* cogorc_planar_chroma_444_420 */
 0.00254714, /* cogorc_planar_chroma_422_420 */
 0.000500952, /* cogorc_convert_YUY2_AYUV */
 0.000435128, /* cogorc_convert_UYVY_AYUV */
 0.000608873, /* cogorc_convert_YUY2_Y42B */
 0.000608782, /* cogorc_convert_UYVY_Y42B */
 0.000502715, /* cogorc_convert_YUY2_Y444 */
 0.000504197, /* cogorc_convert_UYVY_Y444 */
 0.000418321, /* cogorc_convert_UYVY_I420 */
 0.000121437, /* cogorc_convert_AYUV_I420 */
 0.000337226, /* cogorc_convert_AYUV_YUY2 */
 0.000339164, /* cogorc_convert_AYUV_UYVY */
 0.000191338, /* cogorc_convert_AYUV_Y42B */
 0.00054473, /* cogorc_convert_AYUV_Y444 */
 0.000889045, /* cogorc_convert_Y42B_YUY2 */
 0.000758637, /* cogorc_convert_Y42B_UYVY */
 0.000500151, /* cogorc_convert_Y42B_AYUV */
 0.000582774, /* cogorc_convert_Y444_YUY2 */
 0.000573941, /* cogorc_convert_Y444_UYVY */
 0.000992513, /* cogorc_convert_Y444_AYUV */
 0.000179624, /* cogorc_convert_AYUV_ARGB */
 0.000179536, /* cogorc_convert_AYUV_BGRA */
 0.000162291, /* cogorc_convert_AYUV_ABGR */
 0.000179611, /* cogorc_convert_AYUV_RGBA */
 0.000202502, /* cogorc_convert_I420_BGRA */
 0.000147072, /* cogorc_convert_I420_BGRA_avg */
 0.00068815, /* cogorc_getline_I420 */
 0.000571616, /* cogorc_getline_YUY2 */
 0.000571911, /* cogorc_getline_UYVY */
 0.000572026, /* cogorc_getline_YVYU */
 0.000602279, /* cogorc_getline_Y42B */
 0.00121666, /* cogorc_getline_Y444 */
 0.00170376, /* cogorc_getline_Y800 */
 0.000914838, /* cogorc_getline_BGRA */
 0.000548762, /* cogorc_getline_ABGR */
 0.000391498, /* cogorc_getline_RGBA */
 0.000538565, /* cogorc_getline_NV12 */
 0.0003665, /* cogorc_getline_NV21 */
 0.000185278, /* cogorc_putline_I420 */
 0.00038307, /* cogorc_putline_YUY2 */
 0.000296066, /* cogorc_putline_YVYU */
 0.000286583, /* cogorc_putline_UYVY */
 0.000181013, /* cogorc_putline_Y42B */
 0.000619042, /* cogorc_putline_Y444 */
 0.00159584, /* cogorc_putline_Y800 */
 0.000914838, /* cogorc_putline_BGRA */
 0.000393664, /* cogorc_putline_ABGR */
 0.000548656, /* cogorc_putline_RGBA */
 0.000371959, /* cogorc_putline_NV12 */
 0.000260259, /* cogorc_putline_NV21 */
 0.00103791, /* orc_add2_rshift_add_s16_22_op */
 0.000941737, /* orc_add2_rshift_add_s16_22 */
 0.0010417, /* orc_add2_rshift_sub_s16_22_op */
 0.00132172, /* orc_add2_rshift_sub_s16_22 */
 0.00103151, /* orc_add2_rshift_add_s16_11_op */
 0.00134482, /* orc_add2_rshift_add_s16_11 */
 0.000684662, /* orc_add2_rshift_sub_s16_11_op */
 0.00134291, /* orc_add2_rshift_sub_s16_11 */
 0.00249079, /* orc_add_const_rshift_s16_11 */
 0.00321329, /* orc_add_const_rshift_s16 */
 0.00163619, /* orc_add_s16 */
 0.00183164, /* orc_add_s16_2d */
 0.00124922, /* orc_addc_rshift_s16 */
 0.00269892, /* orc_lshift1_s16 */
 0.00271658, /* orc_lshift2_s16 */
 0.00247133, /* orc_lshift_s16_ip */
 0.000504783, /* orc_mas2_add_s16_op */
 0.000547495, /* orc_mas2_add_s16_ip */
 0.000506219, /* orc_mas2_sub_s16_op */
 0.000487229, /* orc_mas2_sub_s16_ip */
 0.000380612, /* orc_mas4_across_add_s16_1991_op */
 0.000422149, /* orc_mas4_across_add_s16_1991_ip */
 0.000378889, /* orc_mas4_across_sub_s16_1991_op */
 0.000422149, /* orc_mas4_across_sub_s16_1991_ip */
 0.0016287, /* orc_subtract_s16 */
 0.00143819, /* orc_add_s16_u8 */
 0.00187371, /* orc_add_s16_u8_2d */
 0.00423657, /* orc_convert_s16_u8 */
 0.00432671, /* orc_convert_u8_s16 */
 0.00381121, /* orc_offsetconvert_u8_s16 */
 0.00376584, /* orc_offsetconvert_s16_u8 */
 0.00194732, /* orc_subtract_s16_u8 */
 0.00145254, /* orc_multiply_and_add_s16_u8 */
 0.00692695, /* orc_splat_s16_ns */
 0.00120295, /* orc_splat_s16_2d_4xn */
 0.00166652, /* orc_splat_s16_2d_8xn */
 0.00691493, /* orc_splat_s16_2d */
 0.0109922, /* orc_splat_u8_ns */
 0.0134864, /* orc_splat_u8_2d */
 0.00233906, /* orc_average_u8 */
 0.00133175, /* orc_rrshift6_add_s16_2d */
 0.0010021, /* orc_rrshift6_sub_s16_2d */
 0.00353507, /* orc_rrshift6_s16_ip_2d */
 0.00519521, /* orc_rrshift6_s16_ip */
 0.00338926, /* orc_unpack_yuyv_y */
 0.00159049, /* orc_unpack_yuyv_u */
 0.0017883, /* orc_unpack_yuyv_v */
 0.000770842, /* orc_packyuyv */
 0.00385247, /* orc_unpack_uyvy_y */
 0.00113925, /* orc_unpack_uyvy_u */
 0.0015896, /* orc_unpack_uyvy_v */
 0.00156858, /* orc_interleave2_s16 */
 0.00108746, /* orc_interleave2_rrshift1_s16 */
 0.00100422, /* orc_deinterleave2_s16 */
 0.000880871, /* orc_deinterleave2_lshift1_s16 */
 0.00041741, /* orc_haar_deint_lshift1_split_s16 */
 0.000608953, /* orc_haar_deint_split_s16 */
 0.00132912, /* orc_haar_split_s16_lo */
 0.00163244, /* orc_haar_split_s16_hi */
 0.000910158, /* orc_haar_split_s16_op */
 0.00113425, /* orc_haar_split_s16 */
 0.00135829, /* orc_haar_synth_s16_lo */
 0.00091821, /* orc_haar_synth_s16_hi */
 0.00090192, /* orc_haar_synth_s16_op */
 0.00102011, /* orc_haar_synth_s16 */
 0.0007121, /* orc_haar_synth_rrshift1_int_s16 */
 0.00102262, /* orc_haar_synth_int_s16 */
 0.0022667, /* orc_haar_sub_s16 */
 0.00210575, /* orc_haar_add_half_s16 */
 0.0022631, /* orc_haar_add_s16 */
 0.00140176, /* orc_haar_sub_half_s16 */
 0.0018942, /* orc_sum_u8 */
 0.0031881, /* orc_sum_s16 */
 0.000997539, /* orc_sum_square_diff_u8 */
 0.000465676, /* orc_dequantise_s16_2d_4xn */
 0.00084084, /* orc_dequantise_s16_2d_8xn */
 0.000933434, /* orc_dequantise_s16_ip_2d_8xn */
 0.00136717, /* orc_dequantise_s16_ip_2d */
 0.00146449, /* orc_dequantise_s16_ip */
 0.00112042, /* orc_dequantise_s16 */
 0.000680877, /* orc_dequantise_var_s16_ip */
 0.00107718, /* orc_quantise1_s16 */
 0.00126364, /* orc_quantise2_s16 */
 0.000513894, /* orc_quantdequant1_s16 */
 0.00025293, /* orc_quantdequant3_s16 */
 0.000552596, /* orc_quantdequant2_s16 */
 0.000792588, /* orc_downsample_vert_u8 */
 0.000323814, /* orc_downsample_horiz_u8 */
 0.00221382, /* orc_stats_moment_s16 */
 0.00191974, /* orc_stats_above_s16 */
 0.00711744, /* orc_accw */
 0.00151278, /* orc_avg2_8xn_u8 */
 0.00141727, /* orc_avg2_12xn_u8 */
 0.000921564, /* orc_avg2_16xn_u8 */
 0.00156177, /* orc_avg2_32xn_u8 */
 0.00249193, /* orc_avg2_nxm_u8 */
 0.000512749, /* orc_combine4_8xn_u8 */
 0.000500305, /* orc_combine4_12xn_u8 */
 0.00049487, /* orc_combine4_16xn_u8 */
 0.000754166, /* orc_combine4_24xn_u8 */
 0.00065116, /* orc_combine4_32xn_u8 */
 0.000597022, /* orc_combine4_nxm_u8 */
 0.00086426, /* orc_combine2_8xn_u8 */
 0.000841659, /* orc_combine2_12xn_u8 */
 0.000783526, /* orc_combine2_16xn_u8 */
 0.00115409, /* orc_combine2_nxm_u8 */
 0.00116772, /* orc_sad_nxm_u8 */
 0.000875992, /* orc_sad_8x8_u8 */
 0.00107602, /* orc_sad_12x12_u8 */
 0.000814229, /* orc_sad_16xn_u8 */
 0.00116016, /* orc_sad_32xn_u8 */
};

double weights_n900[] = {
 0.00189692, /* orc_scalarmultiply_f32_ns */
 0.000655569, /* orc_process_int16 */
 0.000607111, /* orc_process_int16_clamp */
 0.00130039, /* orc_process_int8 */
 0.00121357, /* orc_process_int8_clamp */
 0, /* orc_audio_convert_unpack_u8 */
 0.000886761, /* orc_audio_convert_unpack_s8 */
 0, /* orc_audio_convert_unpack_u16 */
 0.000885403, /* orc_audio_convert_unpack_s16 */
 0, /* orc_audio_convert_unpack_u16_swap */
 0.000884163, /* orc_audio_convert_unpack_s16_swap */
 0, /* orc_audio_convert_unpack_u32 */
 0.000886388, /* orc_audio_convert_unpack_s32 */
 0, /* orc_audio_convert_unpack_u32_swap */
 0.000885463, /* orc_audio_convert_unpack_s32_swap */
 0.000887546, /* orc_audio_convert_unpack_float_s32 */
 0.00088498, /* orc_audio_convert_unpack_float_s32_swap */
 0, /* orc_audio_convert_unpack_float_double */
 0, /* orc_audio_convert_unpack_float_double_swap */
 0.000453781, /* orc_audio_convert_unpack_double_double */
 0.000455403, /* orc_audio_convert_unpack_double_double_swap */
 0, /* orc_audio_convert_unpack_u8_double */
 0, /* orc_audio_convert_unpack_s8_double */
 0, /* orc_audio_convert_unpack_u16_double */
 0, /* orc_audio_convert_unpack_s16_double */
 0, /* orc_audio_convert_unpack_u16_double_swap */
 0, /* orc_audio_convert_unpack_s16_double_swap */
 0, /* orc_audio_convert_unpack_u32_double */
 0, /* orc_audio_convert_unpack_s32_double */
 0, /* orc_audio_convert_unpack_u32_double_swap */
 0, /* orc_audio_convert_unpack_s32_double_swap */
 0, /* orc_audio_convert_pack_u8 */
 0.000923392, /* orc_audio_convert_pack_s8 */
 0, /* orc_audio_convert_pack_u16 */
 0.0010451, /* orc_audio_convert_pack_s16 */
 0, /* orc_audio_convert_pack_u16_swap */
 0.000928539, /* orc_audio_convert_pack_s16_swap */
 0, /* orc_audio_convert_pack_u32 */
 0.000889152, /* orc_audio_convert_pack_s32 */
 0, /* orc_audio_convert_pack_u32_swap */
 0.000878315, /* orc_audio_convert_pack_s32_swap */
 0, /* orc_audio_convert_pack_s32_float */
 0, /* orc_audio_convert_pack_s32_float_swap */
 0, /* orc_audio_convert_pack_double_float */
 0, /* orc_audio_convert_pack_double_float_swap */
 0, /* orc_audio_convert_pack_double_s8 */
 0, /* orc_audio_convert_pack_double_s16 */
 0, /* orc_audio_convert_pack_double_s16_swap */
 0, /* orc_audio_convert_pack_double_s32 */
 0, /* orc_audio_convert_pack_double_s32_swap */
 0.00364483, /* gst_orc_splat_u8 */
 0.00176666, /* gst_orc_splat_s16 */
 0.00176698, /* gst_orc_splat_u16 */
 0.000887025, /* gst_orc_splat_u32 */
 0.00120903, /* orc_merge_linear_u8 */
 0.000840928, /* orc_merge_linear_u16 */
 0.00176687, /* orc_splat_u16 */
 0.000886406, /* orc_splat_u32 */
 0.00206071, /* orc_downsample_u8 */
 0.00111783, /* orc_downsample_u16 */
 0.00057776, /* gst_videoscale_orc_downsample_u32 */
 0.000282225, /* gst_videoscale_orc_downsample_yuyv */
 0, /* gst_videoscale_orc_resample_nearest_u8 */
 0, /* gst_videoscale_orc_resample_bilinear_u8 */
 0, /* gst_videoscale_orc_resample_nearest_u32 */
 0, /* gst_videoscale_orc_resample_bilinear_u32 */
 0, /* gst_videoscale_orc_resample_merge_bilinear_u32 */
 0.000810787, /* gst_videoscale_orc_merge_bicubic_u8 */
 0.00190264, /* add_int32 */
 0.00361624, /* add_int16 */
 0.00507259, /* add_int8 */
 0.00191598, /* add_uint32 */
 0.00361182, /* add_uint16 */
 0.00507259, /* add_uint8 */
 0.00153987, /* add_float32 */
 0.00124016, /* orc_splat_u32 */
 0.00113814, /* orc_memcpy_u32 */
 0.000908623, /* orc_blend_u8 */
 0.000133396, /* orc_blend_argb */
 0, /* orc_blend_bgra */
 0.000901581, /* orc_splat_u32 */
 0.000844679, /* deinterlace_line_vfir */
 0.00413285, /* deinterlace_line_linear */
 0.00123704, /* deinterlace_line_linear_blend */
 0.00109371, /* deinterlace_line_greedy */
 0.00381793, /* cogorc_memcpy_2d */
 0.00377314, /* cogorc_downsample_horiz_cosite_1tap */
 0.0010716, /* cogorc_downsample_horiz_cosite_3tap */
 0.00183775, /* cogorc_downsample_420_jpeg */
 0.00354723, /* cogorc_downsample_vert_halfsite_2tap */
 0.00110979, /* cogorc_downsample_vert_cosite_3tap */
 0.000901003, /* cogorc_downsample_vert_halfsite_4tap */
 0.00170136, /* cogorc_upsample_horiz_cosite_1tap */
 0.00165625, /* cogorc_upsample_horiz_cosite */
 0.00447476, /* cogorc_upsample_vert_avgub */
 0.00463618, /* orc_unpack_yuyv_y */
 0.00171047, /* orc_unpack_yuyv_u */
 0.00154902, /* orc_unpack_yuyv_v */
 0.000832261, /* orc_pack_yuyv */
 0.00380119, /* orc_unpack_uyvy_y */
 0.00190989, /* orc_unpack_uyvy_u */
 0.00170967, /* orc_unpack_uyvy_v */
 0.000832185, /* orc_pack_uyvy */
 0.00104047, /* orc_matrix2_u8 */
 0.000862238, /* orc_matrix2_11_u8 */
 0.00082022, /* orc_matrix2_12_u8 */
 0.00078049, /* orc_matrix3_u8 */
 0.000649602, /* orc_matrix3_100_u8 */
 0.000727013, /* orc_matrix3_100_offset_u8 */
 0.000760795, /* orc_matrix3_000_u8 */
 0.000790078, /* orc_pack_123x */
 0.000792433, /* orc_pack_x123 */
 0.00117391, /* cogorc_combine2_u8 */
 0.000196525, /* cogorc_convert_I420_UYVY */
 0.00019252, /* cogorc_convert_I420_YUY2 */
 0, /* cogorc_convert_I420_AYUV */
 0.000114783, /* cogorc_convert_YUY2_I420 */
 0.000977855, /* cogorc_convert_UYVY_YUY2 */
 0.000740782, /* cogorc_planar_chroma_420_422 */
 0.000600582, /* cogorc_planar_chroma_420_444 */
 0.0018566, /* cogorc_planar_chroma_422_444 */
 0.00219326, /* cogorc_planar_chroma_444_422 */
 0.00130591, /* cogorc_planar_chroma_444_420 */
 0.00211275, /* cogorc_planar_chroma_422_420 */
 0.000442738, /* cogorc_convert_YUY2_AYUV */
 0.000448218, /* cogorc_convert_UYVY_AYUV */
 0.000126284, /* cogorc_convert_YUY2_Y42B */
 0.00013561, /* cogorc_convert_UYVY_Y42B */
 0.000141577, /* cogorc_convert_YUY2_Y444 */
 0.000186874, /* cogorc_convert_UYVY_Y444 */
 9.1806e-05, /* cogorc_convert_UYVY_I420 */
 4.92179e-05, /* cogorc_convert_AYUV_I420 */
 0.000387252, /* cogorc_convert_AYUV_YUY2 */
 0.000386643, /* cogorc_convert_AYUV_UYVY */
 7.10203e-05, /* cogorc_convert_AYUV_Y42B */
 0.000121778, /* cogorc_convert_AYUV_Y444 */
 0.000693003, /* cogorc_convert_Y42B_YUY2 */
 0.00068451, /* cogorc_convert_Y42B_UYVY */
 0.000366249, /* cogorc_convert_Y42B_AYUV */
 0.000602631, /* cogorc_convert_Y444_YUY2 */
 0.00059332, /* cogorc_convert_Y444_UYVY */
 0.000742439, /* cogorc_convert_Y444_AYUV */
 0.00016071, /* cogorc_convert_AYUV_ARGB */
 0.00015913, /* cogorc_convert_AYUV_BGRA */
 0.000159045, /* cogorc_convert_AYUV_ABGR */
 0.000159024, /* cogorc_convert_AYUV_RGBA */
 0, /* cogorc_convert_I420_BGRA */
 0, /* cogorc_convert_I420_BGRA_avg */
 0, /* cogorc_getline_I420 */
 0.000447772, /* cogorc_getline_YUY2 */
 0.000449395, /* cogorc_getline_UYVY */
 0.000448533, /* cogorc_getline_YVYU */
 0.000385634, /* cogorc_getline_Y42B */
 0.000794574, /* cogorc_getline_Y444 */
 0, /* cogorc_getline_Y800 */
 0.000886484, /* cogorc_getline_BGRA */
 0.000554285, /* cogorc_getline_ABGR */
 0.000558078, /* cogorc_getline_RGBA */
 0.000405223, /* cogorc_getline_NV12 */
 0.000448563, /* cogorc_getline_NV21 */
 8.13877e-05, /* cogorc_putline_I420 */
 0.000387218, /* cogorc_putline_YUY2 */
 0.00035472, /* cogorc_putline_YVYU */
 0.000388082, /* cogorc_putline_UYVY */
 7.53593e-05, /* cogorc_putline_Y42B */
 0.000140356, /* cogorc_putline_Y444 */
 0.0017132, /* cogorc_putline_Y800 */
 0.000887432, /* cogorc_putline_BGRA */
 0.000554304, /* cogorc_putline_ABGR */
 0.000558475, /* cogorc_putline_RGBA */
 0.000325961, /* cogorc_putline_NV12 */
 0.000302852, /* cogorc_putline_NV21 */
 0.00165479, /* orc_add2_rshift_add_s16_22_op */
 0.00203387, /* orc_add2_rshift_add_s16_22 */
 0.00178764, /* orc_add2_rshift_sub_s16_22_op */
 0.00191923, /* orc_add2_rshift_sub_s16_22 */
 0.00240863, /* orc_add2_rshift_add_s16_11_op */
 0.00236533, /* orc_add2_rshift_add_s16_11 */
 0.00238607, /* orc_add2_rshift_sub_s16_11_op */
 0.00215832, /* orc_add2_rshift_sub_s16_11 */
 0.00181178, /* orc_add_const_rshift_s16_11 */
 0.00276915, /* orc_add_const_rshift_s16 */
 0.00281042, /* orc_add_s16 */
 0.00166994, /* orc_add_s16_2d */
 0.00213569, /* orc_addc_rshift_s16 */
 0.00285013, /* orc_lshift1_s16 */
 0.00245598, /* orc_lshift2_s16 */
 0.00370756, /* orc_lshift_s16_ip */
 0.000576921, /* orc_mas2_add_s16_op */
 0.000574115, /* orc_mas2_add_s16_ip */
 0.000577379, /* orc_mas2_sub_s16_op */
 0.000572396, /* orc_mas2_sub_s16_ip */
 0.000463221, /* orc_mas4_across_add_s16_1991_op */
 0.000454726, /* orc_mas4_across_add_s16_1991_ip */
 0.000459198, /* orc_mas4_across_sub_s16_1991_op */
 0.000454726, /* orc_mas4_across_sub_s16_1991_ip */
 0.00228059, /* orc_subtract_s16 */
 0.00221536, /* orc_add_s16_u8 */
 0.00207322, /* orc_add_s16_u8_2d */
 0.00292092, /* orc_convert_s16_u8 */
 0.0036697, /* orc_convert_u8_s16 */
 0.00262931, /* orc_offsetconvert_u8_s16 */
 0.00169662, /* orc_offsetconvert_s16_u8 */
 0.00238882, /* orc_subtract_s16_u8 */
 0.00214581, /* orc_multiply_and_add_s16_u8 */
 0.00171415, /* orc_splat_s16_ns */
 0.000393839, /* orc_splat_s16_2d_4xn */
 0.00108717, /* orc_splat_s16_2d_8xn */
 0.00238507, /* orc_splat_s16_2d */
 0.00710336, /* orc_splat_u8_ns */
 0.00403526, /* orc_splat_u8_2d */
 0.0050664, /* orc_average_u8 */
 0.00134097, /* orc_rrshift6_add_s16_2d */
 0.000807761, /* orc_rrshift6_sub_s16_2d */
 0.00307394, /* orc_rrshift6_s16_ip_2d */
 0.00316505, /* orc_rrshift6_s16_ip */
 0.00414213, /* orc_unpack_yuyv_y */
 0.00171048, /* orc_unpack_yuyv_u */
 0.00152237, /* orc_unpack_yuyv_v */
 0.000832094, /* orc_packyuyv */
 0.00399968, /* orc_unpack_uyvy_y */
 0.00191216, /* orc_unpack_uyvy_u */
 0.00171008, /* orc_unpack_uyvy_v */
 0.000897314, /* orc_interleave2_s16 */
 0.000901892, /* orc_interleave2_rrshift1_s16 */
 0.00076676, /* orc_deinterleave2_s16 */
 0.000698548, /* orc_deinterleave2_lshift1_s16 */
 0.00052955, /* orc_haar_deint_lshift1_split_s16 */
 0.000541159, /* orc_haar_deint_split_s16 */
 0.00193144, /* orc_haar_split_s16_lo */
 0.00199633, /* orc_haar_split_s16_hi */
 0.00100157, /* orc_haar_split_s16_op */
 0.00110899, /* orc_haar_split_s16 */
 0.00250888, /* orc_haar_synth_s16_lo */
 0.00237495, /* orc_haar_synth_s16_hi */
 0.00102733, /* orc_haar_synth_s16_op */
 0.000903217, /* orc_haar_synth_s16 */
 0.000876559, /* orc_haar_synth_rrshift1_int_s16 */
 0.000887662, /* orc_haar_synth_int_s16 */
 0.00410694, /* orc_haar_sub_s16 */
 0.0032058, /* orc_haar_add_half_s16 */
 0.00388831, /* orc_haar_add_s16 */
 0.00291108, /* orc_haar_sub_half_s16 */
 0.000801367, /* orc_sum_u8 */
 0.00114909, /* orc_sum_s16 */
 0.000370646, /* orc_sum_square_diff_u8 */
 0.00013075, /* orc_dequantise_s16_2d_4xn */
 0.000133912, /* orc_dequantise_s16_2d_8xn */
 9.03212e-05, /* orc_dequantise_s16_ip_2d_8xn */
 0.00113719, /* orc_dequantise_s16_ip_2d */
 0.00110444, /* orc_dequantise_s16_ip */
 0.0010195, /* orc_dequantise_s16 */
 0.00102223, /* orc_dequantise_var_s16_ip */
 0.000730524, /* orc_quantise1_s16 */
 0.00110444, /* orc_quantise2_s16 */
 0.000335265, /* orc_quantdequant1_s16 */
 0.000192182, /* orc_quantdequant3_s16 */
 0.000417592, /* orc_quantdequant2_s16 */
 0.000898304, /* orc_downsample_vert_u8 */
 0.000362124, /* orc_downsample_horiz_u8 */
 0.000544868, /* orc_stats_moment_s16 */
 0.000453732, /* orc_stats_above_s16 */
 0.00292441, /* orc_accw */
 0.000359527, /* orc_avg2_8xn_u8 */
 0.000354736, /* orc_avg2_12xn_u8 */
 0.00149989, /* orc_avg2_16xn_u8 */
 0.00233937, /* orc_avg2_32xn_u8 */
 0.00314972, /* orc_avg2_nxm_u8 */
 0.00035942, /* orc_combine4_8xn_u8 */
 0.000194902, /* orc_combine4_12xn_u8 */
 0.000479254, /* orc_combine4_16xn_u8 */
 0.000570636, /* orc_combine4_24xn_u8 */
 0.00024299, /* orc_combine4_32xn_u8 */
 0.00065699, /* orc_combine4_nxm_u8 */
 0.000471688, /* orc_combine2_8xn_u8 */
 0.000324071, /* orc_combine2_12xn_u8 */
 0.000253177, /* orc_combine2_16xn_u8 */
 0.000954874, /* orc_combine2_nxm_u8 */
 0.000585976, /* orc_sad_nxm_u8 */
 0.000462648, /* orc_sad_8x8_u8 */
 0.000516877, /* orc_sad_12x12_u8 */
 0.000539588, /* orc_sad_16xn_u8 */
 0.000534326, /* orc_sad_32xn_u8 */
};

