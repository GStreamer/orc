.init bench10_init

#.init gst_volume_orc_init

.function orc_scalarmultiply_f32_ns
.dest 4 d1 float
.floatparam 4 p1

mulf d1, d1, p1


.function orc_process_int16
.dest 2 d1 gint16
.param 2 p1
.temp 4 t1

mulswl t1, d1, p1
shrsl t1, t1, 13
convlw d1, t1


.function orc_process_int16_clamp
.dest 2 d1 gint16
.param 2 p1
.temp 4 t1

mulswl t1, d1, p1
shrsl t1, t1, 13
convssslw d1, t1


.function orc_process_int8
.dest 1 d1 gint8
.param 1 p1
.temp 2 t1

mulsbw t1, d1, p1
shrsw t1, t1, 5
convwb d1, t1


.function orc_process_int8_clamp
.dest 1 d1 gint8
.param 1 p1
.temp 2 t1

mulsbw t1, d1, p1
shrsw t1, t1, 5
convssswb d1, t1



#.init gst_audio_convert_orc_init

.function orc_audio_convert_unpack_u8
.dest 4 d1 gint32
.source 1 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 2 t2
.temp 4 t3

convubw t2, s1
convuwl t3, t2
shll t3, t3, p1
xorl d1, t3, c1


.function orc_audio_convert_unpack_s8
.dest 4 d1 gint32
.source 1 s1 guint8
.param 4 p1
.temp 2 t2
.temp 4 t3

convubw t2, s1
convuwl t3, t2
shll d1, t3, p1


.function orc_audio_convert_unpack_u16
.dest 4 d1 gint32
.source 2 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t2

convuwl t2, s1
shll t2, t2, p1
xorl d1, t2, c1


.function orc_audio_convert_unpack_s16
.dest 4 d1 gint32
.source 2 s1 guint8
.param 4 p1
.temp 4 t2

convuwl t2, s1
shll d1, t2, p1


.function orc_audio_convert_unpack_u16_swap
.dest 4 d1 gint32
.source 2 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 2 t1
.temp 4 t2

swapw t1, s1
convuwl t2, t1
shll t2, t2, p1
xorl d1, t2, c1


.function orc_audio_convert_unpack_s16_swap
.dest 4 d1 gint32
.source 2 s1 guint8
.param 4 p1
.temp 2 t1
.temp 4 t2

swapw t1, s1
convuwl t2, t1
shll d1, t2, p1


.function orc_audio_convert_unpack_u32
.dest 4 d1 gint32
.source 4 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

shll t1, s1, p1
xorl d1, t1, c1


.function orc_audio_convert_unpack_s32
.dest 4 d1 gint32
.source 4 s1 guint8
.param 4 p1

shll d1, s1, p1


.function orc_audio_convert_unpack_u32_swap
.dest 4 d1 gint32
.source 4 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

swapl t1, s1
shll t1, t1, p1
xorl d1, t1, c1


.function orc_audio_convert_unpack_s32_swap
.dest 4 d1 gint32
.source 4 s1 guint8
.param 4 p1
.temp 4 t1

swapl t1, s1
shll d1, t1, p1

.function orc_audio_convert_unpack_float_s32
.source 4 s1 gfloat
.dest 4 d1 guint32
.temp 4 t1

loadl t1, s1
# multiply with 2147483647.0
mulf t1, t1, 0x4F000000
# add 0.5 for rounding
addf t1, t1, 0x3F000000
convfl d1, t1

.function orc_audio_convert_unpack_float_s32_swap
.source 4 s1 gfloat
.dest 4 d1 guint32
.temp 4 t1

swapl t1, s1
# multiply with 2147483647.0
mulf t1, t1, 0x4F000000
# add 0.5 for rounding
addf t1, t1, 0x3F000000
convfl d1, t1

.function orc_audio_convert_unpack_float_double
.dest 8 d1 gdouble
.source 4 s1 gfloat

convfd d1, s1

.function orc_audio_convert_unpack_float_double_swap
.dest 8 d1 gdouble
.source 4 s1 gfloat
.temp 4 t1

swapl t1, s1
convfd d1, t1

.function orc_audio_convert_unpack_double_double
.dest 8 d1 gdouble
.source 8 s1 gdouble

copyq d1, s1

.function orc_audio_convert_unpack_double_double_swap
.dest 8 d1 gdouble
.source 8 s1 gdouble

swapq d1, s1

.function orc_audio_convert_unpack_u8_double
.dest 8 d1 gdouble
.source 1 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 2 t2
.temp 4 t3

convubw t2, s1
convuwl t3, t2
shll t3, t3, p1
xorl t3, t3, c1
convld d1, t3

.function orc_audio_convert_unpack_s8_double
.dest 8 d1 gdouble
.source 1 s1 guint8
.param 4 p1
.temp 2 t2
.temp 4 t3

convubw t2, s1
convuwl t3, t2
shll t3, t3, p1
convld d1, t3

.function orc_audio_convert_unpack_u16_double
.dest 8 d1 gdouble
.source 2 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t2

convuwl t2, s1
shll t2, t2, p1
xorl t2, t2, c1
convld d1, t2

.function orc_audio_convert_unpack_s16_double
.dest 8 d1 gdouble
.source 2 s1 guint8
.param 4 p1
.temp 4 t2

convuwl t2, s1
shll t2, t2, p1
convld d1, t2

.function orc_audio_convert_unpack_u16_double_swap
.dest 8 d1 gdouble
.source 2 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 2 t1
.temp 4 t2

swapw t1, s1
convuwl t2, t1
shll t2, t2, p1
xorl t2, t2, c1
convld d1, t2

.function orc_audio_convert_unpack_s16_double_swap
.dest 8 d1 gdouble
.source 2 s1 guint8
.param 4 p1
.temp 2 t1
.temp 4 t2

swapw t1, s1
convuwl t2, t1
shll t2, t2, p1
convld d1, t2

.function orc_audio_convert_unpack_u32_double
.dest 8 d1 gdouble
.source 4 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

shll t1, s1, p1
xorl t1, t1, c1
convld d1, t1

.function orc_audio_convert_unpack_s32_double
.dest 8 d1 gdouble
.source 4 s1 guint8
.param 4 p1
.temp 4 t1

shll t1, s1, p1
convld d1, t1

.function orc_audio_convert_unpack_u32_double_swap
.dest 8 d1 gdouble
.source 4 s1 guint8
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

swapl t1, s1
shll t1, t1, p1
xorl t1, t1, c1
convld d1, t1

.function orc_audio_convert_unpack_s32_double_swap
.dest 8 d1 gdouble
.source 4 s1 guint8
.param 4 p1
.temp 4 t1

swapl t1, s1
shll t1, t1, p1
convld d1, t1

.function orc_audio_convert_pack_u8
.dest 1 d1 guint8
.source 4 s1 gint32
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1
.temp 2 t2

xorl t1, s1, c1
shrul t1, t1, p1
convlw t2, t1
convwb d1, t2


.function orc_audio_convert_pack_s8
.dest 1 d1 guint8
.source 4 s1 gint32
.param 4 p1
.temp 4 t1
.temp 2 t2

shrsl t1, s1, p1
convlw t2, t1
convwb d1, t2



.function orc_audio_convert_pack_u16
.dest 2 d1 guint8
.source 4 s1 gint32
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

xorl t1, s1, c1
shrul t1, t1, p1
convlw d1, t1


.function orc_audio_convert_pack_s16
.dest 2 d1 guint8
.source 4 s1 gint32
.param 4 p1
.temp 4 t1

shrsl t1, s1, p1
convlw d1, t1


.function orc_audio_convert_pack_u16_swap
.dest 2 d1 guint8
.source 4 s1 gint32
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1
.temp 2 t2

xorl t1, s1, c1
shrul t1, t1, p1
convlw t2, t1
swapw d1, t2


.function orc_audio_convert_pack_s16_swap
.dest 2 d1 guint8
.source 4 s1 gint32
.param 4 p1
.temp 4 t1
.temp 2 t2

shrsl t1, s1, p1
convlw t2, t1
swapw d1, t2



.function orc_audio_convert_pack_u32
.dest 4 d1 guint8
.source 4 s1 gint32
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

xorl t1, s1, c1
shrul d1, t1, p1


.function orc_audio_convert_pack_s32
.dest 4 d1 guint8
.source 4 s1 gint32
.param 4 p1

shrsl d1, s1, p1


.function orc_audio_convert_pack_u32_swap
.dest 4 d1 guint8
.source 4 s1 gint32
.param 4 p1
.const 4 c1 0x80000000
.temp 4 t1

xorl t1, s1, c1
shrul t1, t1, p1
swapl d1, t1


.function orc_audio_convert_pack_s32_swap
.dest 4 d1 guint8
.source 4 s1 gint32
.param 4 p1
.temp 4 t1

shrsl t1, s1, p1
swapl d1, t1

.function orc_audio_convert_pack_s32_float
.dest 4 d1 gfloat
.source 4 s1 gint32
.temp 4 t1

convlf t1, s1
# divide by 2147483647.0
divf t1, t1, 0x4F000000
storel d1, t1

.function orc_audio_convert_pack_s32_float_swap
.dest 4 d1 gfloat
.source 4 s1 gint32
.temp 4 t1

convlf t1, s1
# divide by 2147483647.0
divf t1, t1, 0x4F000000
swapl d1, t1

.function orc_audio_convert_pack_double_float
.dest 4 d1 gfloat
.source 8 s1 gdouble

convdf d1, s1

.function orc_audio_convert_pack_double_float_swap
.dest 4 d1 gfloat
.source 8 s1 gdouble
.temp 4 t1

convdf t1, s1
swapl d1, t1

.function orc_audio_convert_pack_double_s8
.dest 1 d1 guint8
.source 8 s1 gdouble
.param 4 p1
.temp 4 t1
.temp 2 t2

convdl t1, s1
shrsl t1, t1, p1
convlw t2, t1
convwb d1, t2

.function orc_audio_convert_pack_double_s16
.dest 2 d1 guint8
.source 8 s1 gdouble
.param 4 p1
.temp 4 t1

convdl t1, s1
shrsl t1, t1, p1
convlw d1, t1

.function orc_audio_convert_pack_double_s16_swap
.dest 2 d1 guint8
.source 8 s1 gdouble
.param 4 p1
.temp 4 t1
.temp 2 t2

convdl t1, s1
shrsl t1, t1, p1
convlw t2, t1
swapw d1, t2

.function orc_audio_convert_pack_double_s32
.dest 4 d1 guint8
.source 8 s1 gdouble
.param 4 p1
.temp 4 t1

convdl t1, s1
shrsl d1, t1, p1

.function orc_audio_convert_pack_double_s32_swap
.dest 4 d1 guint8
.source 8 s1 gdouble
.param 4 p1
.temp 4 t1

convdl t1, s1
shrsl t1, t1, p1
swapl d1, t1


#.init gst_videotestsrc_orc_init

.function gst_orc_splat_u8
.dest 1 d1 guint8
.param 1 p1

copyb d1, p1


.function gst_orc_splat_s16
.dest 2 d1 gint8
.param 2 p1

copyw d1, p1


.function gst_orc_splat_u16
.dest 2 d1 guint8
.param 2 p1

copyw d1, p1


.function gst_orc_splat_u32
.dest 4 d1 guint8
.param 4 p1

copyl d1, p1



#.init gst_videoscale_orc_init

.function orc_merge_linear_u8
.dest 1 d1
.source 1 s1
.source 1 s2
.param 1 p1
.temp 2 t1
.temp 2 t2
.temp 1 a
.temp 1 t

loadb a, s1
convubw t1, s1
convubw t2, s2
subw t2, t2, t1
mullw t2, t2, p1
addw t2, t2, 128
convhwb t, t2
addb d1, t, a



.function orc_merge_linear_u16
.dest 2 d1
.source 2 s1
.source 2 s2
.param 2 p1
.param 2 p2
.temp 4 t1
.temp 4 t2

# This is slightly different thatn the u8 case, since muluwl
# tends to be much faster than mulll
muluwl t1, s1, p1
muluwl t2, s2, p2
addl t1, t1, t2
shrul t1, t1, 16
convlw d1, t1


.function orc_splat_u16
.dest 2 d1
.param 2 p1

copyw d1, p1


.function orc_splat_u32
.dest 4 d1
.param 4 p1

copyl d1, p1


.function orc_downsample_u8
.dest 1 d1 guint8
.source 2 s1 guint8
.temp 1 t1
.temp 1 t2

splitwb t1, t2, s1
avgub d1, t1, t2


.function orc_downsample_u16
.dest 2 d1 guint16
.source 4 s1 guint16
.temp 2 t1
.temp 2 t2

splitlw t1, t2, s1
avguw d1, t1, t2


.function gst_videoscale_orc_downsample_u32
.dest 4 d1 guint8
.source 8 s1 guint8
.temp 4 t1
.temp 4 t2

splitql t1, t2, s1
x4 avgub d1, t1, t2


.function gst_videoscale_orc_downsample_yuyv
.dest 4 d1 guint8
.source 8 s1 guint8
.temp 4 yyyy
.temp 4 uvuv
.temp 2 t1
.temp 2 t2
.temp 2 yy
.temp 2 uv

x4 splitwb yyyy, uvuv, s1
x2 splitwb t1, t2, yyyy
x2 avgub yy, t1, t2
splitlw t1, t2, uvuv
x2 avgub uv, t1, t2
x2 mergebw d1, yy, uv



.function gst_videoscale_orc_resample_nearest_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.param 4 p1
.param 4 p2

ldresnearb d1, s1, p1, p2


.function gst_videoscale_orc_resample_bilinear_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.param 4 p1
.param 4 p2

ldreslinb d1, s1, p1, p2


.function gst_videoscale_orc_resample_nearest_u32
.dest 4 d1 guint8
.source 4 s1 guint8
.param 4 p1
.param 4 p2

ldresnearl d1, s1, p1, p2


.function gst_videoscale_orc_resample_bilinear_u32
.dest 4 d1 guint8
.source 4 s1 guint8
.param 4 p1
.param 4 p2

ldreslinl d1, s1, p1, p2


.function gst_videoscale_orc_resample_merge_bilinear_u32
.dest 4 d1 guint8
.dest 4 d2 guint8
.source 4 s1 guint8
.source 4 s2 guint8
.temp 4 a
.temp 4 b
.temp 4 t
.temp 8 t1
.temp 8 t2
.param 4 p1
.param 4 p2
.param 4 p3

ldreslinl b, s2, p2, p3
storel d2, b
loadl a, s1
x4 convubw t1, a
x4 convubw t2, b
x4 subw t2, t2, t1
x4 mullw t2, t2, p1
x4 convhwb t, t2
x4 addb d1, t, a



.function gst_videoscale_orc_merge_bicubic_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.source 1 s4 guint8
.param 4 p1
.param 4 p2
.param 4 p3
.param 4 p4
.temp 2 t1
.temp 2 t2

mulubw t1, s2, p2
mulubw t2, s3, p3
addw t1, t1, t2
mulubw t2, s1, p1
subw t1, t1, t2
mulubw t2, s4, p4
subw t1, t1, t2
addw t1, t1, 32
shrsw t1, t1, 6
convsuswb d1, t1



#.init gst_adder_orc_init

.function add_int32
.dest 4 d1 gint32
.source 4 s1 gint32

addssl d1, d1, s1


.function add_int16
.dest 2 d1 gint16
.source 2 s1 gint16

addssw d1, d1, s1


.function add_int8
.dest 1 d1 gint8
.source 1 s1 gint8

addssb d1, d1, s1


.function add_uint32
.dest 4 d1 guint32
.source 4 s1 guint32

addusl d1, d1, s1


.function add_uint16
.dest 2 d1 guint16
.source 2 s1 guint16

addusw d1, d1, s1


.function add_uint8
.dest 1 d1 guint8
.source 1 s1 guint8

addusb d1, d1, s1


.function add_float32
.dest 4 d1 float
.source 4 s1 float

addf d1, d1, s1


.function orc_splat_u32
.dest 4 d1 guint32
.param 4 p1 guint32

copyl d1, p1

.function orc_memcpy_u32
.dest 4 d1 guint32
.source 4 s1 guint32

copyl d1, s1

.function orc_blend_u8
.flags 2d
.dest 1 d1 guint8
.source 1 s1 guint8
.param 2 p1
.temp 2 t1
.temp 2 t2
.const 1 c1 8 

convubw t1, d1
convubw t2, s1
subw t2, t2, t1
mullw t2, t2, p1
shlw t1, t1, c1
addw t2, t1, t2
shruw t2, t2, c1
convsuswb d1, t2


.function orc_blend_argb
.flags 2d
.dest 4 d guint8
.source 4 s guint8
.param 2 alpha
.temp 4 t
.temp 2 tw
.temp 1 tb
.temp 4 a
.temp 8 d_wide
.temp 8 s_wide
.temp 8 a_wide
.const 4 a_alpha 0x000000ff

loadl t, s
convlw tw, t
convwb tb, tw
splatbl a, tb
x4 convubw a_wide, a
x4 mullw a_wide, a_wide, alpha
x4 shruw a_wide, a_wide, 8
x4 convubw s_wide, t
loadl t, d
x4 convubw d_wide, t
x4 subw s_wide, s_wide, d_wide
x4 mullw s_wide, s_wide, a_wide
x4 div255w s_wide, s_wide
x4 addw d_wide, d_wide, s_wide
x4 convwb t, d_wide
orl t, t, a_alpha
storel d, t

.function orc_blend_bgra
.flags 2d
.dest 4 d guint8
.source 4 s guint8
.param 2 alpha
.temp 4 t
.temp 4 t2
.temp 2 tw
.temp 1 tb
.temp 4 a
.temp 8 d_wide
.temp 8 s_wide
.temp 8 a_wide
.const 4 a_alpha 0xff000000

loadl t, s
shrul t2, t, 24
convlw tw, t2
convwb tb, tw
splatbl a, tb
x4 convubw a_wide, a
x4 mullw a_wide, a_wide, alpha
x4 shruw a_wide, a_wide, 8
x4 convubw s_wide, t
loadl t, d
x4 convubw d_wide, t
x4 subw s_wide, s_wide, d_wide
x4 mullw s_wide, s_wide, a_wide
x4 div255w s_wide, s_wide
x4 addw d_wide, d_wide, s_wide
x4 convwb t, d_wide
orl t, t, a_alpha
storel d, t


.function orc_splat_u32
.dest 4 d1 guint32
.param 4 p1

copyl d1, p1


.function deinterlace_line_vfir
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.source 1 s4 guint8
.source 1 s5 guint8
.temp 2 t1
.temp 2 t2
.temp 2 t3

convubw t1, s1
convubw t2, s5
addw t1, t1, t2
convubw t2, s2
convubw t3, s4
addw t2, t2, t3
shlw t2, t2, 2
convubw t3, s3
shlw t3, t3, 1
subw t2, t2, t1
addw t2, t2, t3
addw t2, t2, 4
shrsw t2, t2, 3
convsuswb d1, t2


.function deinterlace_line_linear
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8

avgub d1, s1, s2


.function deinterlace_line_linear_blend
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.temp 2 t1
.temp 2 t2
.temp 2 t3

convubw t1, s1
convubw t2, s2
convubw t3, s3
addw t1, t1, t2
addw t3, t3, t3
addw t1, t1, t3
addw t1, t1, 2
shrsw t1, t1, 2
convsuswb d1, t1


.function deinterlace_line_greedy
.dest 1 d1
.source 1 m0
.source 1 t1
.source 1 b1
.source 1 m2
.param 1 max_comb
.temp 1 tm0
.temp 1 tm2
.temp 1 tb1
.temp 1 tt1
.temp 1 avg
.temp 1 l2_diff
.temp 1 lp2_diff
.temp 1 t2
.temp 1 t3
.temp 1 best
.temp 1 min
.temp 1 max


loadb tm0, m0
loadb tm2, m2

loadb tb1, b1
loadb tt1, t1
avgub avg, tt1, tb1

maxub t2, tm0, avg
minub t3, tm0, avg
subb l2_diff, t2, t3

maxub t2, tm2, avg
minub t3, tm2, avg
subb lp2_diff, t2, t3

xorb l2_diff, l2_diff, 0x80
xorb lp2_diff, lp2_diff, 0x80
cmpgtsb t3, l2_diff, lp2_diff

andb t2, tm2, t3
andnb t3, t3, tm0
orb best, t2, t3

maxub max, tt1, tb1
minub min, tt1, tb1
addusb max, max, max_comb
subusb min, min, max_comb
minub best, best, max
maxub d1, best, min




.function cogorc_memcpy_2d
.flags 2d
.dest 1 d1 guint8
.source 1 s1 guint8

copyb d1, s1


.function cogorc_downsample_horiz_cosite_1tap
.dest 1 d1 guint8
.source 2 s1 guint8

select0wb d1, s1


.function cogorc_downsample_horiz_cosite_3tap
.dest 1 d1 guint8
.source 2 s1 guint8
.source 2 s2 guint8
.temp 1 t1
.temp 1 t2
.temp 1 t3
.temp 2 t4
.temp 2 t5
.temp 2 t6

copyw t4, s1
select0wb t1, t4
select1wb t2, t4
select0wb t3, s2
convubw t4, t1
convubw t5, t2
convubw t6, t3
mullw t5, t5, 2
addw t4, t4, t6
addw t4, t4, t5
addw t4, t4, 2
shrsw t4, t4, 2
convsuswb d1, t4


.function cogorc_downsample_420_jpeg
.dest 1 d1 guint8
.source 2 s1 guint8
.source 2 s2 guint8
.temp 2 t1
.temp 1 t2
.temp 1 t3
.temp 1 t4
.temp 1 t5

copyw t1, s1
select0wb t2, t1
select1wb t3, t1
avgub t2, t2, t3
copyw t1, s2
select0wb t4, t1
select1wb t5, t1
avgub t4, t4, t5
avgub d1, t2, t4


.function cogorc_downsample_vert_halfsite_2tap
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8

avgub d1, s1, s2


.function cogorc_downsample_vert_cosite_3tap
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.temp 2 t1
.temp 2 t2
.temp 2 t3

convubw t1, s1
convubw t2, s2
convubw t3, s3
mullw t2, t2, 2
addw t1, t1, t3
addw t1, t1, t2
addw t1, t1, 2
shrsw t1, t1, 2
convsuswb d1, t1



.function cogorc_downsample_vert_halfsite_4tap
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.source 1 s4 guint8
.temp 2 t1
.temp 2 t2
.temp 2 t3
.temp 2 t4

convubw t1, s1
convubw t2, s2
convubw t3, s3
convubw t4, s4
addw t2, t2, t3
mullw t2, t2, 26
addw t1, t1, t4
mullw t1, t1, 6
addw t2, t2, t1
addw t2, t2, 32
shrsw t2, t2, 6
convsuswb d1, t2


.function cogorc_upsample_horiz_cosite_1tap
.dest 2 d1 guint8
.source 1 s1 guint8
.temp 1 t1

copyb t1, s1
mergebw d1, t1, t1


.function cogorc_upsample_horiz_cosite
.dest 2 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.temp 1 t1
.temp 1 t2

copyb t1, s1
avgub t2, t1, s2
mergebw d1, t1, t2


.function cogorc_upsample_vert_avgub
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8

avgub d1, s1, s2




.function orc_unpack_yuyv_y
.dest 1 d1 guint8
.source 2 s1 guint8

select0wb d1, s1


.function orc_unpack_yuyv_u
.dest 1 d1 guint8
.source 4 s1 guint8
.temp 2 t1

select0lw t1, s1
select1wb d1, t1


.function orc_unpack_yuyv_v
.dest 1 d1 guint8
.source 4 s1 guint8
.temp 2 t1

select1lw t1, s1
select1wb d1, t1


.function orc_pack_yuyv
.dest 4 d1 guint8
.source 2 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.temp 1 t1
.temp 1 t2
.temp 2 t3
.temp 2 t4
.temp 2 t5

copyw t5, s1
select0wb t1, t5
select1wb t2, t5
mergebw t3, t1, s2
mergebw t4, t2, s3
mergewl d1, t3, t4


.function orc_unpack_uyvy_y
.dest 1 d1 guint8
.source 2 s1 guint8

select1wb d1, s1


.function orc_unpack_uyvy_u
.dest 1 d1 guint8
.source 4 s1 guint8
.temp 2 t1

select0lw t1, s1
select0wb d1, t1


.function orc_unpack_uyvy_v
.dest 1 d1 guint8
.source 4 s1 guint8
.temp 2 t1

select1lw t1, s1
select0wb d1, t1


.function orc_pack_uyvy
.dest 4 d1 guint8
.source 2 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.temp 1 t1
.temp 1 t2
.temp 2 t3
.temp 2 t4
.temp 2 t5

copyw t5, s1
select0wb t1, t5
select1wb t2, t5
mergebw t3, s2, t1
mergebw t4, s3, t2
mergewl d1, t3, t4


.function orc_matrix2_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.param 2 p1
.param 2 p2
.param 2 p3
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t1, t1, p1
convubw t2, s2
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, 6
convsuswb d1, t1


.function orc_matrix2_11_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2
.temp 2 t3
.temp 2 t4

convubw t1, s1
subw t1, t1, 16
mullw t3, t1, p1
convubw t2, s2
subw t2, t2, 128
mullw t4, t2, p2
addw t3, t3, t4
addw t3, t3, 128
shrsw t3, t3, 8
addw t3, t3, t1
addw t3, t3, t2
convsuswb d1, t3


.function orc_matrix2_12_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2
.temp 2 t3
.temp 2 t4

convubw t1, s1
subw t1, t1, 16
mullw t3, t1, p1
convubw t2, s2
subw t2, t2, 128
mullw t4, t2, p2
addw t3, t3, t4
addw t3, t3, 128
shrsw t3, t3, 8
addw t3, t3, t1
addw t3, t3, t2
addw t3, t3, t2
convsuswb d1, t3


.function orc_matrix3_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t1, t1, p1
convubw t2, s2
mullw t2, t2, p2
addw t1, t1, t2
convubw t2, s3
mullw t2, t2, p3
addw t1, t1, t2
addw t1, t1, p4
shrsw t1, t1, 6
convsuswb d1, t1


.function orc_matrix3_100_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.param 2 p1
.param 2 p2
.param 2 p3
.temp 2 t1
.temp 2 t2
.temp 2 t3
#.temp 2 t4

convubw t1, s1
subw t1, t1, 16
mullw t3, t1, p1
convubw t2, s2
subw t2, t2, 128
mullw t2, t2, p2
addw t3, t3, t2
convubw t2, s3
subw t2, t2, 128
mullw t2, t2, p3
addw t3, t3, t2
addw t3, t3, 128
shrsw t3, t3, 8
addw t3, t3, t1
convsuswb d1, t3


.function orc_matrix3_100_offset_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.param 2 p5
#.param 2 p6
.temp 2 t1
.temp 2 t2
.temp 2 t3
#.temp 2 t3
#.temp 2 t4

convubw t3, s1
mullw t1, t3, p1
convubw t2, s2
mullw t2, t2, p2
addw t1, t1, t2
convubw t2, s3
mullw t2, t2, p3
addw t1, t1, t2
addw t1, t1, p4
shrsw t1, t1, p5
#addw t1, t1, p6
addw t1, t1, t3
convsuswb d1, t1



.function orc_matrix3_000_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.param 2 p5
#.param 2 p6
.temp 2 t1
.temp 2 t2
#.temp 2 t3
#.temp 2 t4

convubw t1, s1
mullw t1, t1, p1
convubw t2, s2
mullw t2, t2, p2
addw t1, t1, t2
convubw t2, s3
mullw t2, t2, p3
addw t1, t1, t2
addw t1, t1, p4
shrsw t1, t1, p5
#addw t1, t1, p6
convwb d1, t1



.function orc_pack_123x
.dest 4 d1 guint32
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.param 1 p1
.temp 2 t1
.temp 2 t2

mergebw t1, s1, s2
mergebw t2, s3, p1
mergewl d1, t1, t2


.function orc_pack_x123
.dest 4 d1 guint32
.source 1 s1 guint8
.source 1 s2 guint8
.source 1 s3 guint8
.param 1 p1
.temp 2 t1
.temp 2 t2

mergebw t1, p1, s1
mergebw t2, s2, s3
mergewl d1, t1, t2


.function cogorc_combine2_u8
.dest 1 d1 guint8
.source 1 s1 guint8
.source 1 s2 guint8
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t1, t1, p1
convubw t2, s2
mullw t2, t2, p2
addw t1, t1, t2
shruw t1, t1, 8
convsuswb d1, t1


.function cogorc_convert_I420_UYVY
.dest 4 d1 guint8
.dest 4 d2 guint8
.source 2 y1 guint8
.source 2 y2 guint8
.source 1 u guint8
.source 1 v guint8
.temp 2 uv

mergebw uv, u, v
x2 mergebw d1, uv, y1
x2 mergebw d2, uv, y2


.function cogorc_convert_I420_YUY2
.dest 4 d1 guint8
.dest 4 d2 guint8
.source 2 y1 guint8
.source 2 y2 guint8
.source 1 u guint8
.source 1 v guint8
.temp 2 uv

mergebw uv, u, v
x2 mergebw d1, y1, uv
x2 mergebw d2, y2, uv



.function cogorc_convert_I420_AYUV
.dest 4 d1 guint8
.dest 4 d2 guint8
.source 1 y1 guint8
.source 1 y2 guint8
.source 1 u guint8
.source 1 v guint8
.const 1 c255 255
.temp 2 uv
.temp 2 ay
.temp 1 tu
.temp 1 tv

loadupdb tu, u
loadupdb tv, v
mergebw uv, tu, tv
mergebw ay, c255, y1
mergewl d1, ay, uv
mergebw ay, c255, y2
mergewl d2, ay, uv


.function cogorc_convert_YUY2_I420
.dest 2 y1 guint8
.dest 2 y2 guint8
.dest 1 u guint8
.dest 1 v guint8
.source 4 yuv1 guint8
.source 4 yuv2 guint8
.temp 2 t1
.temp 2 t2
.temp 2 ty

x2 splitwb t1, ty, yuv1
storew y1, ty
x2 splitwb t2, ty, yuv2
storew y2, ty
x2 avgub t1, t1, t2
splitwb v, u, t1


.function cogorc_convert_UYVY_YUY2
.flags 2d
.dest 4 yuy2 guint8
.source 4 uyvy guint8

x2 swapw yuy2, uyvy


.function cogorc_planar_chroma_420_422
.flags 2d
.dest 1 d1 guint8
.dest 1 d2 guint8
.source 1 s guint8

copyb d1, s
copyb d2, s


.function cogorc_planar_chroma_420_444
.flags 2d
.dest 2 d1 guint8
.dest 2 d2 guint8
.source 1 s guint8
.temp 2 t

splatbw t, s
storew d1, t
storew d2, t


.function cogorc_planar_chroma_422_444
.flags 2d
.dest 2 d1 guint8
.source 1 s guint8
.temp 2 t

splatbw t, s
storew d1, t


.function cogorc_planar_chroma_444_422
.flags 2d
.dest 1 d guint8
.source 2 s guint8
.temp 1 t1
.temp 1 t2

splitwb t1, t2, s
avgub d, t1, t2


.function cogorc_planar_chroma_444_420
.flags 2d
.dest 1 d guint8
.source 2 s1 guint8
.source 2 s2 guint8
.temp 2 t
.temp 1 t1
.temp 1 t2

x2 avgub t, s1, s2
splitwb t1, t2, t
avgub d, t1, t2


.function cogorc_planar_chroma_422_420
.flags 2d
.dest 1 d guint8
.source 1 s1 guint8
.source 1 s2 guint8

avgub d, s1, s2


.function cogorc_convert_YUY2_AYUV
.flags 2d
.dest 8 ayuv guint8
.source 4 yuy2 guint8
.const 2 c255 0xff
.temp 2 yy
.temp 2 uv
.temp 4 ayay
.temp 4 uvuv

x2 splitwb uv, yy, yuy2
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_convert_UYVY_AYUV
.flags 2d
.dest 8 ayuv guint8
.source 4 uyvy guint8
.const 2 c255 0xff
.temp 2 yy
.temp 2 uv
.temp 4 ayay
.temp 4 uvuv

x2 splitwb yy, uv, uyvy
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_convert_YUY2_Y42B
.flags 2d
.dest 2 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 4 yuy2 guint8
.temp 2 uv

x2 splitwb uv, y, yuy2
splitwb v, u, uv


.function cogorc_convert_UYVY_Y42B
.flags 2d
.dest 2 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 4 uyvy guint8
.temp 2 uv

x2 splitwb y, uv, uyvy
splitwb v, u, uv


.function cogorc_convert_YUY2_Y444
.flags 2d
.dest 2 y guint8
.dest 2 uu guint8
.dest 2 vv guint8
.source 4 yuy2 guint8
.temp 2 uv
.temp 1 u
.temp 1 v

x2 splitwb uv, y, yuy2
splitwb v, u, uv
splatbw uu, u
splatbw vv, v


.function cogorc_convert_UYVY_Y444
.flags 2d
.dest 2 y guint8
.dest 2 uu guint8
.dest 2 vv guint8
.source 4 uyvy guint8
.temp 2 uv
.temp 1 u
.temp 1 v

x2 splitwb y, uv, uyvy
splitwb v, u, uv
splatbw uu, u
splatbw vv, v


.function cogorc_convert_UYVY_I420
.dest 2 y1 guint8
.dest 2 y2 guint8
.dest 1 u guint8
.dest 1 v guint8
.source 4 yuv1 guint8
.source 4 yuv2 guint8
.temp 2 t1
.temp 2 t2
.temp 2 ty

x2 splitwb ty, t1, yuv1
storew y1, ty
x2 splitwb ty, t2, yuv2
storew y2, ty
x2 avgub t1, t1, t2
splitwb v, u, t1



.function cogorc_convert_AYUV_I420
.flags 2d
.dest 2 y1 guint8
.dest 2 y2 guint8
.dest 1 u guint8
.dest 1 v guint8
.source 8 ayuv1 guint8
.source 8 ayuv2 guint8
.temp 4 ay
.temp 4 uv1
.temp 4 uv2
.temp 4 uv
.temp 2 uu
.temp 2 vv
.temp 1 t1
.temp 1 t2

x2 splitlw uv1, ay, ayuv1
x2 select1wb y1, ay
x2 splitlw uv2, ay, ayuv2
x2 select1wb y2, ay
x4 avgub uv, uv1, uv2
x2 splitwb vv, uu, uv
splitwb t1, t2, uu
avgub u, t1, t2
splitwb t1, t2, vv
avgub v, t1, t2



.function cogorc_convert_AYUV_YUY2
.flags 2d
.dest 4 yuy2 guint8
.source 8 ayuv guint8
.temp 2 yy
.temp 2 uv1
.temp 2 uv2
.temp 4 ayay
.temp 4 uvuv

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
x2 select1wb yy, ayay
x2 mergebw yuy2, yy, uv1


.function cogorc_convert_AYUV_UYVY
.flags 2d
.dest 4 yuy2 guint8
.source 8 ayuv guint8
.temp 2 yy
.temp 2 uv1
.temp 2 uv2
.temp 4 ayay
.temp 4 uvuv

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
x2 select1wb yy, ayay
x2 mergebw yuy2, uv1, yy



.function cogorc_convert_AYUV_Y42B
.flags 2d
.dest 2 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 8 ayuv guint8
.temp 4 ayay
.temp 4 uvuv
.temp 2 uv1
.temp 2 uv2

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
splitwb v, u, uv1
x2 select1wb y, ayay


.function cogorc_convert_AYUV_Y444
.flags 2d
.dest 1 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 4 ayuv guint8
.temp 2 ay
.temp 2 uv

splitlw uv, ay, ayuv
splitwb v, u, uv
select1wb y, ay


.function cogorc_convert_Y42B_YUY2
.flags 2d
.dest 4 yuy2 guint8
.source 2 y guint8
.source 1 u guint8
.source 1 v guint8
.temp 2 uv

mergebw uv, u, v
x2 mergebw yuy2, y, uv


.function cogorc_convert_Y42B_UYVY
.flags 2d
.dest 4 uyvy guint8
.source 2 y guint8
.source 1 u guint8
.source 1 v guint8
.temp 2 uv

mergebw uv, u, v
x2 mergebw uyvy, uv, y


.function cogorc_convert_Y42B_AYUV
.flags 2d
.dest 8 ayuv guint8
.source 2 yy guint8
.source 1 u guint8
.source 1 v guint8
.const 1 c255 255
.temp 2 uv
.temp 2 ay
.temp 4 uvuv
.temp 4 ayay

mergebw uv, u, v
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_convert_Y444_YUY2
.flags 2d
.dest 4 yuy2 guint8
.source 2 y guint8
.source 2 u guint8
.source 2 v guint8
.temp 2 uv
.temp 4 uvuv
.temp 2 uv1
.temp 2 uv2

x2 mergebw uvuv, u, v
splitlw uv1, uv2, uvuv
x2 avgub uv, uv1, uv2
x2 mergebw yuy2, y, uv


.function cogorc_convert_Y444_UYVY
.flags 2d
.dest 4 uyvy guint8
.source 2 y guint8
.source 2 u guint8
.source 2 v guint8
.temp 2 uv
.temp 4 uvuv
.temp 2 uv1
.temp 2 uv2

x2 mergebw uvuv, u, v
splitlw uv1, uv2, uvuv
x2 avgub uv, uv1, uv2
x2 mergebw uyvy, uv, y


.function cogorc_convert_Y444_AYUV
.flags 2d
.dest 4 ayuv guint8
.source 1 yy guint8
.source 1 u guint8
.source 1 v guint8
.const 1 c255 255
.temp 2 uv
.temp 2 ay

mergebw uv, u, v
mergebw ay, c255, yy
mergewl ayuv, ay, uv



.function cogorc_convert_AYUV_ARGB
.flags 2d
.dest 4 argb guint8
.source 4 ayuv guint8
.temp 2 t1
.temp 2 t2
.temp 1 a
.temp 1 y
.temp 1 u
.temp 1 v
.temp 2 wy
.temp 2 wu
.temp 2 wv
.temp 2 wr
.temp 2 wg
.temp 2 wb
.temp 1 r
.temp 1 g
.temp 1 b
.temp 4 x
.const 1 c8 8

x4 subb x, ayuv, 128
splitlw t1, t2, x
splitwb y, a, t2
splitwb v, u, t1
convsbw wy, y
convsbw wu, u
convsbw wv, v

mullw t1, wy, 42
shrsw t1, t1, c8
addssw wy, wy, t1

addssw wr, wy, wv
mullw t1, wv, 103
shrsw t1, t1, c8
subssw wr, wr, t1
addssw wr, wr, wv

addssw wb, wy, wu
addssw wb, wb, wu
mullw t1, wu, 4
shrsw t1, t1, c8
addssw wb, wb, t1

mullw t1, wu, 100
shrsw t1, t1, c8
subssw wg, wy, t1
mullw t1, wv, 104
shrsw t1, t1, c8
subssw wg, wg, t1
subssw wg, wg, t1

convssswb r, wr
convssswb g, wg
convssswb b, wb

mergebw t1, a, r
mergebw t2, g, b
mergewl x, t1, t2
x4 addb argb, x, 128



.function cogorc_convert_AYUV_BGRA
.flags 2d
.dest 4 argb guint8
.source 4 ayuv guint8
.temp 2 t1
.temp 2 t2
.temp 1 a
.temp 1 y
.temp 1 u
.temp 1 v
.temp 2 wy
.temp 2 wu
.temp 2 wv
.temp 2 wr
.temp 2 wg
.temp 2 wb
.temp 1 r
.temp 1 g
.temp 1 b
.temp 4 x
.const 1 c8 8

x4 subb x, ayuv, 128
splitlw t1, t2, x
splitwb y, a, t2
splitwb v, u, t1
convsbw wy, y
convsbw wu, u
convsbw wv, v

mullw t1, wy, 42
shrsw t1, t1, c8
addssw wy, wy, t1

addssw wr, wy, wv
mullw t1, wv, 103
shrsw t1, t1, c8
subssw wr, wr, t1
addssw wr, wr, wv

addssw wb, wy, wu
addssw wb, wb, wu
mullw t1, wu, 4
shrsw t1, t1, c8
addssw wb, wb, t1

mullw t1, wu, 100
shrsw t1, t1, c8
subssw wg, wy, t1
mullw t1, wv, 104
shrsw t1, t1, c8
subssw wg, wg, t1
subssw wg, wg, t1

convssswb r, wr
convssswb g, wg
convssswb b, wb

mergebw t1, b, g
mergebw t2, r, a
mergewl x, t1, t2
x4 addb argb, x, 128




.function cogorc_convert_AYUV_ABGR
.flags 2d
.dest 4 argb guint8
.source 4 ayuv guint8
.temp 2 t1
.temp 2 t2
.temp 1 a
.temp 1 y
.temp 1 u
.temp 1 v
.temp 2 wy
.temp 2 wu
.temp 2 wv
.temp 2 wr
.temp 2 wg
.temp 2 wb
.temp 1 r
.temp 1 g
.temp 1 b
.temp 4 x
.const 1 c8 8

x4 subb x, ayuv, 128
splitlw t1, t2, x
splitwb y, a, t2
splitwb v, u, t1
convsbw wy, y
convsbw wu, u
convsbw wv, v

mullw t1, wy, 42
shrsw t1, t1, c8
addssw wy, wy, t1

addssw wr, wy, wv
mullw t1, wv, 103
shrsw t1, t1, c8
subssw wr, wr, t1
addssw wr, wr, wv

addssw wb, wy, wu
addssw wb, wb, wu
mullw t1, wu, 4
shrsw t1, t1, c8
addssw wb, wb, t1

mullw t1, wu, 100
shrsw t1, t1, c8
subssw wg, wy, t1
mullw t1, wv, 104
shrsw t1, t1, c8
subssw wg, wg, t1
subssw wg, wg, t1

convssswb r, wr
convssswb g, wg
convssswb b, wb

mergebw t1, a, b
mergebw t2, g, r
mergewl x, t1, t2
x4 addb argb, x, 128



.function cogorc_convert_AYUV_RGBA
.flags 2d
.dest 4 argb guint8
.source 4 ayuv guint8
.temp 2 t1
.temp 2 t2
.temp 1 a
.temp 1 y
.temp 1 u
.temp 1 v
.temp 2 wy
.temp 2 wu
.temp 2 wv
.temp 2 wr
.temp 2 wg
.temp 2 wb
.temp 1 r
.temp 1 g
.temp 1 b
.temp 4 x
.const 1 c8 8

x4 subb x, ayuv, 128
splitlw t1, t2, x
splitwb y, a, t2
splitwb v, u, t1
convsbw wy, y
convsbw wu, u
convsbw wv, v

mullw t1, wy, 42
shrsw t1, t1, c8
addssw wy, wy, t1

addssw wr, wy, wv
mullw t1, wv, 103
shrsw t1, t1, c8
subssw wr, wr, t1
addssw wr, wr, wv

addssw wb, wy, wu
addssw wb, wb, wu
mullw t1, wu, 4
shrsw t1, t1, c8
addssw wb, wb, t1

mullw t1, wu, 100
shrsw t1, t1, c8
subssw wg, wy, t1
mullw t1, wv, 104
shrsw t1, t1, c8
subssw wg, wg, t1
subssw wg, wg, t1

convssswb r, wr
convssswb g, wg
convssswb b, wb

mergebw t1, r, g
mergebw t2, b, a
mergewl x, t1, t2
x4 addb argb, x, 128



.function cogorc_convert_I420_BGRA
.dest 4 argb guint8
.source 1 y guint8
.source 1 u guint8
.source 1 v guint8
.temp 2 t1
.temp 2 t2
.temp 1 t3
.temp 2 wy
.temp 2 wu
.temp 2 wv
.temp 2 wr
.temp 2 wg
.temp 2 wb
.temp 1 r
.temp 1 g
.temp 1 b
.temp 4 x
.const 1 c8 8
.const 1 c128 128

subb t3, y, c128
convsbw wy, t3
loadupib t3, u
subb t3, t3, c128
convsbw wu, t3
loadupib t3, v
subb t3, t3, c128
convsbw wv, t3

mullw t1, wy, 42
shrsw t1, t1, c8
addssw wy, wy, t1

addssw wr, wy, wv
mullw t1, wv, 103
shrsw t1, t1, c8
subssw wr, wr, t1
addssw wr, wr, wv

addssw wb, wy, wu
addssw wb, wb, wu
mullw t1, wu, 4
shrsw t1, t1, c8
addssw wb, wb, t1

mullw t1, wu, 100
shrsw t1, t1, c8
subssw wg, wy, t1
mullw t1, wv, 104
shrsw t1, t1, c8
subssw wg, wg, t1
subssw wg, wg, t1

convssswb r, wr
convssswb g, wg
convssswb b, wb

mergebw t1, b, g
mergebw t2, r, 255
mergewl x, t1, t2
x4 addb argb, x, c128



.function cogorc_convert_I420_BGRA_avg
.dest 4 argb guint8
.source 1 y guint8
.source 1 u1 guint8
.source 1 u2 guint8
.source 1 v1 guint8
.source 1 v2 guint8
.temp 2 t1
.temp 2 t2
.temp 1 t3
.temp 1 t4
.temp 2 wy
.temp 2 wu
.temp 2 wv
.temp 2 wr
.temp 2 wg
.temp 2 wb
.temp 1 r
.temp 1 g
.temp 1 b
.temp 4 x
.const 1 c8 8
.const 1 c128 128

subb t3, y, c128
convsbw wy, t3
loadupib t3, u1
loadupib t4, u2
avgub t3, t3, t4
subb t3, t3, c128
convsbw wu, t3
loadupib t3, v1
loadupib t4, v2
avgub t3, t3, t4
subb t3, t3, c128
convsbw wv, t3

mullw t1, wy, 42
shrsw t1, t1, c8
addssw wy, wy, t1

addssw wr, wy, wv
mullw t1, wv, 103
shrsw t1, t1, c8
subssw wr, wr, t1
addssw wr, wr, wv

addssw wb, wy, wu
addssw wb, wb, wu
mullw t1, wu, 4
shrsw t1, t1, c8
addssw wb, wb, t1

mullw t1, wu, 100
shrsw t1, t1, c8
subssw wg, wy, t1
mullw t1, wv, 104
shrsw t1, t1, c8
subssw wg, wg, t1
subssw wg, wg, t1

convssswb r, wr
convssswb g, wg
convssswb b, wb

mergebw t1, b, g
mergebw t2, r, 255
mergewl x, t1, t2
x4 addb argb, x, c128



.function cogorc_getline_I420
.dest 4 d guint8
.source 1 y guint8
.source 1 u guint8
.source 1 v guint8
.const 1 c255 255
.temp 2 uv
.temp 2 ay
.temp 1 tu
.temp 1 tv

loadupdb tu, u
loadupdb tv, v
mergebw uv, tu, tv
mergebw ay, c255, y
mergewl d, ay, uv


.function cogorc_getline_YUY2
.dest 8 ayuv guint8
.source 4 yuy2 guint8
.const 2 c255 0xff
.temp 2 yy
.temp 2 uv
.temp 4 ayay
.temp 4 uvuv

x2 splitwb uv, yy, yuy2
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_getline_UYVY
.dest 8 ayuv guint8
.source 4 uyvy guint8
.const 2 c255 0xff
.temp 2 yy
.temp 2 uv
.temp 4 ayay
.temp 4 uvuv

x2 splitwb yy, uv, uyvy
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_getline_YVYU
.dest 8 ayuv guint8
.source 4 uyvy guint8
.const 2 c255 0xff
.temp 2 yy
.temp 2 uv
.temp 4 ayay
.temp 4 uvuv

x2 splitwb yy, uv, uyvy
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_getline_Y42B
.dest 8 ayuv guint8
.source 2 yy guint8
.source 1 u guint8
.source 1 v guint8
.const 1 c255 255
.temp 2 uv
.temp 2 ay
.temp 4 uvuv
.temp 4 ayay

mergebw uv, u, v
x2 mergebw ayay, c255, yy
mergewl uvuv, uv, uv
x2 mergewl ayuv, ayay, uvuv


.function cogorc_getline_Y444
.dest 4 ayuv guint8
.source 1 y guint8
.source 1 u guint8
.source 1 v guint8
.const 1 c255 255
.temp 2 uv
.temp 2 ay

mergebw uv, u, v
mergebw ay, c255, y
mergewl ayuv, ay, uv


.function cogorc_getline_Y800
.dest 4 ayuv guint8
.source 1 y guint8
.const 1 c255 255
.const 2 c0xffff 0xffff
.temp 2 ay

mergebw ay, c255, y
mergewl ayuv, ay, c0xffff


.function cogorc_getline_BGRA
.dest 4 argb guint8
.source 4 bgra guint8

swapl argb, bgra


.function cogorc_getline_ABGR
.dest 4 argb guint8
.source 4 abgr guint8
.temp 1 a
.temp 1 r
.temp 1 g
.temp 1 b
.temp 2 gr
.temp 2 ab
.temp 2 ar
.temp 2 gb

splitlw gr, ab, abgr
splitwb r, g, gr
splitwb b, a, ab
mergebw ar, a, r
mergebw gb, g, b
mergewl argb, ar, gb


.function cogorc_getline_RGBA
.dest 4 argb guint8
.source 4 rgba guint8
.temp 1 a
.temp 1 r
.temp 1 g
.temp 1 b
.temp 2 rg
.temp 2 ba
.temp 2 ar
.temp 2 gb

splitlw ba, rg, rgba
splitwb g, r, rg
splitwb a, b, ba
mergebw ar, a, r
mergebw gb, g, b
mergewl argb, ar, gb


.function cogorc_getline_NV12
.dest 8 d guint8
.source 2 y guint8
.source 2 uv guint8
.const 1 c255 255
.temp 4 ay
.temp 4 uvuv

mergewl uvuv, uv, uv
x2 mergebw ay, c255, y
x2 mergewl d, ay, uvuv


.function cogorc_getline_NV21
.dest 8 d guint8
.source 2 y guint8
.source 2 vu guint8
.const 1 c255 255
.temp 2 uv
.temp 4 ay
.temp 4 uvuv

swapw uv, vu
mergewl uvuv, uv, uv
x2 mergebw ay, c255, y
x2 mergewl d, ay, uvuv


.function cogorc_putline_I420
.dest 2 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 8 ayuv guint8
.temp 4 ay
.temp 4 uv
.temp 2 uu
.temp 2 vv
.temp 1 t1
.temp 1 t2

x2 splitlw uv, ay, ayuv
x2 select1wb y, ay
x2 splitwb vv, uu, uv
splitwb t1, t2, uu
avgub u, t1, t2
splitwb t1, t2, vv
avgub v, t1, t2



.function cogorc_putline_YUY2
.dest 4 yuy2 guint8
.source 8 ayuv guint8
.temp 2 yy
.temp 2 uv1
.temp 2 uv2
.temp 4 ayay
.temp 4 uvuv

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
x2 select1wb yy, ayay
x2 mergebw yuy2, yy, uv1


.function cogorc_putline_YVYU
.dest 4 yuy2 guint8
.source 8 ayuv guint8
.temp 2 yy
.temp 2 uv1
.temp 2 uv2
.temp 4 ayay
.temp 4 uvuv

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
x2 select1wb yy, ayay
swapw uv1, uv1
x2 mergebw yuy2, yy, uv1


.function cogorc_putline_UYVY
.dest 4 yuy2 guint8
.source 8 ayuv guint8
.temp 2 yy
.temp 2 uv1
.temp 2 uv2
.temp 4 ayay
.temp 4 uvuv

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
x2 select1wb yy, ayay
x2 mergebw yuy2, uv1, yy



.function cogorc_putline_Y42B
.dest 2 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 8 ayuv guint8
.temp 4 ayay
.temp 4 uvuv
.temp 2 uv1
.temp 2 uv2

x2 splitlw uvuv, ayay, ayuv
splitlw uv1, uv2, uvuv
x2 avgub uv1, uv1, uv2
splitwb v, u, uv1
x2 select1wb y, ayay


.function cogorc_putline_Y444
.dest 1 y guint8
.dest 1 u guint8
.dest 1 v guint8
.source 4 ayuv guint8
.temp 2 ay
.temp 2 uv

splitlw uv, ay, ayuv
splitwb v, u, uv
select1wb y, ay


.function cogorc_putline_Y800
.dest 1 y guint8
.source 4 ayuv guint8
.temp 2 ay

select0lw ay, ayuv
select1wb y, ay


.function cogorc_putline_BGRA
.dest 4 bgra guint8
.source 4 argb guint8

swapl bgra, argb


.function cogorc_putline_ABGR
.dest 4 abgr guint8
.source 4 argb guint8
.temp 1 a
.temp 1 r
.temp 1 g
.temp 1 b
.temp 2 gr
.temp 2 ab
.temp 2 ar
.temp 2 gb

splitlw gb, ar, argb
splitwb b, g, gb
splitwb r, a, ar
mergebw ab, a, b
mergebw gr, g, r
mergewl abgr, ab, gr


.function cogorc_putline_RGBA
.dest 4 rgba guint8
.source 4 argb guint8
.temp 1 a
.temp 1 r
.temp 1 g
.temp 1 b
.temp 2 rg
.temp 2 ba
.temp 2 ar
.temp 2 gb

splitlw gb, ar, argb
splitwb b, g, gb
splitwb r, a, ar
mergebw ba, b, a
mergebw rg, r, g
mergewl rgba, rg, ba


.function cogorc_putline_NV12
.dest 2 y guint8
.dest 2 uv guint8
.source 8 ayuv guint8
.temp 4 ay
.temp 4 uvuv
.temp 2 uv1
.temp 2 uv2

x2 splitlw uvuv, ay, ayuv
x2 select1wb y, ay
splitlw uv1, uv2, uvuv
x2 avgub uv, uv1, uv2


.function cogorc_putline_NV21
.dest 2 y guint8
.dest 2 vu guint8
.source 8 ayuv guint8
.temp 4 ay
.temp 4 uvuv
.temp 2 uv1
.temp 2 uv2
.temp 2 uv

x2 splitlw uvuv, ay, ayuv
x2 select1wb y, ay
splitlw uv1, uv2, uvuv
x2 avgub uv, uv1, uv2
swapw vu, uv



#.init schro_orc_init

.function orc_add2_rshift_add_s16_22_op
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.temp 2 t1

addw t1, s2, s3
addw t1, t1, 2
shrsw t1, t1, 2
addw d1, s1, t1


.function orc_add2_rshift_add_s16_22
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
addw d1, d1, t1


.function orc_add2_rshift_sub_s16_22_op
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.temp 2 t1

addw t1, s2, s3
addw t1, t1, 2
shrsw t1, t1, 2
subw d1, s1, t1


.function orc_add2_rshift_sub_s16_22
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

addw t1, s1, s2
addw t1, t1, 2
shrsw t1, t1, 2
subw d1, d1, t1


.function orc_add2_rshift_add_s16_11_op
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.temp 2 t1

avgsw t1, s2, s3
addw d1, s1, t1


.function orc_add2_rshift_add_s16_11
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

avgsw t1, s1, s2
addw d1, d1, t1


.function orc_add2_rshift_sub_s16_11_op
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.temp 2 t1

avgsw t1, s2, s3
subw d1, s1, t1


.function orc_add2_rshift_sub_s16_11
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

avgsw t1, s1, s2
subw d1, d1, t1


.function orc_add_const_rshift_s16_11
.dest 2 d1 int16_t
.source 2 s1 int16_t
.temp 2 t1

addw t1, s1, 1
shrsw d1, t1, 1


.function orc_add_const_rshift_s16
.dest 2 d1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1

addw t1, d1, p1
shrsw d1, t1, p2


.function orc_add_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

addw d1, s1, s2


.function orc_add_s16_2d
.flags 2d
.dest 2 d1 int16_t
.source 2 s1 int16_t

addw d1, d1, s1


.function orc_addc_rshift_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.param 2 p1

addw t1, s1, s2
shrsw d1, t1, p1


.function orc_lshift1_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

shlw d1, s1, 1


.function orc_lshift2_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

shlw d1, s1, 2


.function orc_lshift_s16_ip
.dest 2 d1 int16_t
.param 2 p1

shlw d1, d1, p1


.function orc_mas2_add_s16_op
.dest 2 d1 int16_t
.source 2 s0 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 4 p2
.param 4 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shrsl t2, t2, p3
convlw t1, t2
addw d1, s0, t1


.function orc_mas2_add_s16_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 4 p2
.param 4 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shrsl t2, t2, p3
convlw t1, t2
addw d1, d1, t1


.function orc_mas2_sub_s16_op
.dest 2 d1 int16_t
.source 2 s0 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 4 p2
.param 4 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shrsl t2, t2, p3
convlw t1, t2
subw d1, s0, t1


.function orc_mas2_sub_s16_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 4 t2
.param 2 p1
.param 4 p2
.param 4 p3

addw t1, s1, s2
mulswl t2, t1, p1
addl t2, t2, p2
shrsl t2, t2, p3
convlw t1, t2
subw d1, d1, t1


.function orc_mas4_across_add_s16_1991_op
.dest 2 d1 int16_t
.source 2 s0 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.source 2 s4 int16_t
.param 4 p1
.param 4 p2
.temp 2 t1
.temp 2 t2
.temp 4 t3
.temp 4 t4

addw t1, s2, s3
mulswl t3, t1, 9
addw t2, s1, s4
convswl t4, t2
subl t3, t3, t4
addl t3, t3, p1
shrsl t3, t3, p2
convlw t1, t3
addw d1, s0, t1


.function orc_mas4_across_add_s16_1991_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 4 p1
.param 4 p2
.temp 2 t1
.temp 2 t2
.temp 4 t3
.temp 4 t4

loadoffw t1, s1, 1
loadoffw t2, s1, 2
addw t1, t1, t2
mulswl t3, t1, 9
loadw t1, s1
loadoffw t2, s1, 3
addw t2, t1, t2
convswl t4, t2
subl t3, t3, t4
addl t3, t3, p1
shrsl t3, t3, p2
convlw t1, t3
addw d1, d1, t1


.function orc_mas4_across_sub_s16_1991_op
.dest 2 d1 int16_t
.source 2 s0 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.source 2 s3 int16_t
.source 2 s4 int16_t
.param 4 p1
.param 4 p2
.temp 2 t1
.temp 2 t2
.temp 4 t3
.temp 4 t4

addw t1, s2, s3
mulswl t3, t1, 9
addw t2, s1, s4
convswl t4, t2
subl t3, t3, t4
addl t3, t3, p1
shrsl t3, t3, p2
convlw t1, t3
subw d1, s0, t1


.function orc_mas4_across_sub_s16_1991_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 4 p1
.param 4 p2
.temp 2 t1
.temp 2 t2
.temp 4 t3
.temp 4 t4

loadoffw t1, s1, 1
loadoffw t2, s1, 2
addw t1, t1, t2
mulswl t3, t1, 9
loadw t1, s1
loadoffw t2, s1, 3
addw t2, t1, t2
convswl t4, t2
subl t3, t3, t4
addl t3, t3, p1
shrsl t3, t3, p2
convlw t1, t3
subw d1, d1, t1


.function orc_subtract_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

subw d1, s1, s2


.function orc_add_s16_u8
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 1 s2
.temp 2 t1

convubw t1, s2
addw d1, t1, s1


.function orc_add_s16_u8_2d
.flags 2d
.dest 2 d1 int16_t
.source 1 s1
.temp 2 t1

convubw t1, s1
addw d1, d1, t1


.function orc_convert_s16_u8
.dest 2 d1
.source 1 s1

convubw d1, s1


.function orc_convert_u8_s16
.dest 1 d1
.source 2 s1 int16_t

convsuswb d1, s1


.function orc_offsetconvert_u8_s16
.dest 1 d1
.source 2 s1 int16_t
.temp 2 t1

addw t1, s1, 128
convsuswb d1, t1


.function orc_offsetconvert_s16_u8
.dest 2 d1 int16_t
.source 1 s1
.temp 2 t1

convubw t1, s1
subw d1, t1, 128


.function orc_subtract_s16_u8
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 1 s2
.temp 2 t1

convubw t1, s2
subw d1, s1, t1


.function orc_multiply_and_add_s16_u8
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 1 s2
.temp 2 t1

convubw t1, s2
mullw t1, t1, s1
addw d1, d1, t1


.function orc_splat_s16_ns
.dest 2 d1 int16_t
.param 2 p1

copyw d1, p1


.function orc_splat_s16_2d_4xn
.n 4
.flags 2d
.dest 2 d1 int16_t
.param 2 p1

copyw d1, p1


.function orc_splat_s16_2d_8xn
.n 8
.flags 2d
.dest 2 d1 int16_t
.param 2 p1

copyw d1, p1


.function orc_splat_s16_2d
.flags 2d
.dest 2 d1 int16_t
.param 2 p1

copyw d1, p1


.function orc_splat_u8_ns
.dest 1 d1
.param 1 p1

copyb d1, p1


.function orc_splat_u8_2d
.flags 2d
.dest 1 d1
.param 1 p1

copyb d1, p1


.function orc_average_u8
.dest 1 d1
.source 1 s1
.source 1 s2

avgub d1, s1, s2


.function orc_rrshift6_add_s16_2d
.flags 2d
.dest 1 d1 uint8_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

addw t1, s2, 32
shrsw t1, t1, 6
addw t1, s1, t1
convsuswb d1, t1


.function orc_rrshift6_sub_s16_2d
.flags 2d
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.temp 2 t1

subw t1, d2, 8160
shrsw t1, t1, 6
copyw d2, t1
subw d1, d1, t1


.function orc_rrshift6_s16_ip_2d
.flags 2d
.dest 2 d1 int16_t
.temp 2 t1

subw t1, d1, 8160
shrsw d1, t1, 6


.function orc_rrshift6_s16_ip
.dest 2 d1 int16_t
.temp 2 t1

subw t1, d1, 8160
shrsw d1, t1, 6


.function orc_unpack_yuyv_y
.dest 1 d1
.source 2 s1

select0wb d1, s1


.function orc_unpack_yuyv_u
.dest 1 d1
.source 4 s1
.temp 2 t1

select0lw t1, s1
select1wb d1, t1


.function orc_unpack_yuyv_v
.dest 1 d1
.source 4 s1
.temp 2 t1

select1lw t1, s1
select1wb d1, t1


.function orc_packyuyv
.dest 4 d1
.source 2 s1 uint8_t
.source 1 s2
.source 1 s3
.temp 1 t1
.temp 1 t2
.temp 2 t3
.temp 2 t4
.temp 2 t5

copyw t5, s1
select0wb t1, t5
select1wb t2, t5
mergebw t3, t1, s2
mergebw t4, t2, s3
mergewl d1, t3, t4


.function orc_unpack_uyvy_y
.dest 1 d1
.source 2 s1

select1wb d1, s1


.function orc_unpack_uyvy_u
.dest 1 d1
.source 4 s1
.temp 2 t1

select0lw t1, s1
select0wb d1, t1


.function orc_unpack_uyvy_v
.dest 1 d1
.source 4 s1
.temp 2 t1

select1lw t1, s1
select0wb d1, t1


.function orc_interleave2_s16
.dest 4 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

mergewl d1, s1, s2


.function orc_interleave2_rrshift1_s16
.dest 4 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

addw t1, s1, 1
shrsw t1, t1, 1
addw t2, s2, 1
shrsw t2, t2, 1
mergewl d1, t1, t2


.function orc_deinterleave2_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 4 s1 int16_t
.temp 4 t1

copyl t1, s1
select0lw d1, t1
select1lw d2, t1


.function orc_deinterleave2_lshift1_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 4 s1 int16_t
.temp 4 t1
.temp 2 t2
.temp 2 t3

copyl t1, s1
select0lw t2, t1
shlw d1, t2, 1
select1lw t3, t1
shlw d2, t3, 1


.function orc_haar_deint_lshift1_split_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 4 s1 int16_t
.temp 2 t1
.temp 2 t2
.temp 4 t3

copyl t3, s1
select0lw t1, t3
select1lw t2, t3
shlw t1, t1, 1
shlw t2, t2, 1
subw t2, t2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_deint_split_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 4 s1 int16_t
.temp 2 t1
.temp 2 t2
.temp 4 t3

copyl t3, s1
select0lw t1, t3
select1lw t2, t3
subw t2, t2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_split_s16_lo
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

copyw t1, s1
subw t2, s2, t1
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_split_s16_hi
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t

subw d1, s2, s1


.function orc_haar_split_s16_op
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

copyw t1, s1
subw t2, s2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_split_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.temp 2 t1
.temp 2 t2

copyw t1, d1
copyw t2, d2
subw t2, t2, t1
copyw d2, t2
avgsw t2, t2, 0
addw d1, t1, t2


.function orc_haar_synth_s16_lo
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1

avgsw t1, s2, 0
subw d1, s1, t1


.function orc_haar_synth_s16_hi
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2
.temp 2 t3

copyw t2, s2
avgsw t3, t2, 0
subw t1, s1, t3
addw d1, t2, t1


.function orc_haar_synth_s16_op
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2
.temp 2 t3

copyw t2, s2
avgsw t3, t2, 0
subw t1, s1, t3
copyw d1, t1
addw d2, t2, t1


.function orc_haar_synth_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.temp 2 t1
.temp 2 t2
.temp 2 t3

copyw t1, d1
copyw t2, d2
avgsw t3, t2, 0
subw t1, t1, t3
copyw d1, t1
addw d2, t2, t1


.function orc_haar_synth_rrshift1_int_s16
.dest 4 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

copyw t2, s2
avgsw t1, t2, 0
subw t1, s1, t1
addw t2, t2, t1
avgsw t1, t1, 0
avgsw t2, t2, 0
mergewl d1, t1, t2


.function orc_haar_synth_int_s16
.dest 4 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

copyw t2, s2
avgsw t1, t2, 0
subw t1, s1, t1
addw t2, t2, t1
mergewl d1, t1, t2


.function orc_haar_sub_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

subw d1, d1, s1


.function orc_haar_add_half_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.temp 2 t1

avgsw t1, s1, 0
addw d1, d1, t1


.function orc_haar_add_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t

addw d1, d1, s1


.function orc_haar_sub_half_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.temp 2 t1

avgsw t1, s1, 0
subw d1, d1, t1


.function orc_sum_u8
.accumulator 4 a1 int32_t
.source 1 s1
.temp 2 t1
.temp 4 t2

convubw t1, s1
convuwl t2, t1
accl a1, t2


.function orc_sum_s16
.accumulator 4 a1 int32_t
.source 2 s1 int16_t
.temp 4 t1

convswl t1, s1
accl a1, t1


.function orc_sum_square_diff_u8
.accumulator 4 a1 int32_t
.source 1 s1
.source 1 s2
.temp 2 t1
.temp 2 t2
.temp 4 t3

convubw t1, s1
convubw t2, s2
subw t1, t1, t2
mullw t1, t1, t1
convuwl t3, t1
accl a1, t3


.function orc_dequantise_s16_2d_4xn
.n 4
.flags 2d
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_s16_2d_8xn
.n 8
.flags 2d
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_s16_ip_2d_8xn
.n 8
.flags 2d
.dest 2 d1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, d1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_s16_ip_2d
.flags 2d
.dest 2 d1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, d1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_s16_ip
.dest 2 d1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, d1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
mullw t1, t1, p1
addw t1, t1, p2
shrsw t1, t1, 2
mullw d1, t1, t2


.function orc_dequantise_var_s16_ip
.dest 2 d1 int16_t
.source 2 s1 int16_t
.source 2 s2 int16_t
.temp 2 t1
.temp 2 t2

copyw t1, d1
signw t2, t1
absw t1, t1
mullw t1, t1, s1
addw t1, t1, s2
shrsw t1, t1, 2
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantise1_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.param 2 p3
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
mulhuw t1, t1, p1
shruw t1, t1, p3
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantise2_s16
.dest 2 d1 int16_t
.source 2 s1 int16_t
.param 2 p1
.param 2 p2
.temp 2 t1
.temp 2 t2

copyw t1, s1
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
shruw t1, t1, p1
mullw d1, t1, t2


# only works for values between -16384 and 16384
.function orc_quantdequant1_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.param 2 p5
.temp 2 t1
.temp 2 t2

copyw t1, d2
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
mulhuw t1, t1, p1
shruw t1, t1, p3
mullw t2, t1, t2
copyw d1, t2
signw t2, t2
mullw t1, t1, p4
addw t1, t1, p5
shrsw t1, t1, 2
mullw d2, t1, t2


# only works for values between -16384 and 16384
.function orc_quantdequant3_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.param 2 p5
.param 4 p6
.temp 2 t1
.temp 2 t2
.temp 4 t3

copyw t1, d2
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
muluwl t3, t1, p1
addl t3, t3, p6
shrul t3, t3, p3
convlw t1, t3
mullw t2, t1, t2
copyw d1, t2
signw t2, t2
mullw t1, t1, p4
addw t1, t1, p5
shrsw t1, t1, 2
mullw d2, t1, t2


# only works for values between -16384 and 16384
.function orc_quantdequant2_s16
.dest 2 d1 int16_t
.dest 2 d2 int16_t
.param 2 p1
.param 2 p2
.param 2 p4
.param 2 p5
.temp 2 t1
.temp 2 t2

copyw t1, d2
signw t2, t1
absw t1, t1
shlw t1, t1, 2
subw t1, t1, p2
shruw t1, t1, p1
mullw t2, t1, t2
copyw d1, t2
signw t2, t2
mullw t1, t1, p4
addw t1, t1, p5
shrsw t1, t1, 2
mullw d2, t1, t2



.function orc_downsample_vert_u8
.dest 1 d1
.source 1 s1
.source 1 s2
.source 1 s3
.source 1 s4
.temp 2 t1
.temp 2 t2
.temp 2 t3

convubw t1, s1
convubw t2, s4
addw t1, t1, t2
mullw t1, t1, 6
convubw t2, s2
convubw t3, s3
addw t2, t2, t3
mullw t2, t2, 26
addw t2, t2, t1
addw t2, t2, 32
shruw t2, t2, 6
convwb d1, t2


.function orc_downsample_horiz_u8
.dest 1 d1
.source 2 s1 uint8_t
.temp 2 a
.temp 2 b
.temp 2 t1
.temp 1 t2
.temp 1 t3
.temp 2 c

loadw t1, s1
select1wb t2, t1
convubw a, t2
loadoffw t1, s1, 2
select0wb t2, t1
convubw b, t2
addw c, a, b
mullw c, c, 6

loadoffw t1, s1, 1
splitwb t2, t3, t1
convubw a, t2
convubw b, t3
addw a, a, b
mullw a, a, 26
addw c, c, a
addw c, c, 32
shruw c, c, 6
convwb d1, c


.function orc_stats_moment_s16
.source 2 s1 int16_t
.accumulator 4 a1 int32_t
.temp 2 t1
.temp 4 t2

absw t1, s1
subw t1, t1, 2
maxsw t1, t1, 0
convuwl t2, t1
accl a1, t2


.function orc_stats_above_s16
.source 2 s1 int16_t
.accumulator 4 a1 int32_t
.temp 2 t1
.temp 4 t2

absw t1, s1
subw t1, t1, 1
maxsw t1, t1, 0
minsw t1, t1, 1
convuwl t2, t1
accl a1, t2


.function orc_accw
.accumulator 2 a1 int
.source 2 s1 int16_t
.temp 2 t1

absw t1, s1
accw a1, t1


.function orc_avg2_8xn_u8
.flags 2d
.n 8
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

avgub d1, s1, s2


.function orc_avg2_12xn_u8
.flags 2d
.n 12
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

avgub d1, s1, s2


.function orc_avg2_16xn_u8
.flags 2d
.n 16
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

avgub d1, s1, s2


.function orc_avg2_32xn_u8
.flags 2d
.n 32
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

avgub d1, s1, s2


.function orc_avg2_nxm_u8
.flags 2d
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

avgub d1, s1, s2


.function orc_combine4_8xn_u8
.flags 2d
.n 8
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.source 1 s3 uint8_t
.source 1 s4 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_12xn_u8
.flags 2d
.n 12
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.source 1 s3 uint8_t
.source 1 s4 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_16xn_u8
.flags 2d
.n 16
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.source 1 s3 uint8_t
.source 1 s4 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_24xn_u8
.flags 2d
.n 24
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.source 1 s3 uint8_t
.source 1 s4 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_32xn_u8
.flags 2d
.n 32
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.source 1 s3 uint8_t
.source 1 s4 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
convsuswb d1, t2


.function orc_combine4_nxm_u8
.flags 2d
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.source 1 s3 uint8_t
.source 1 s4 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
mullw t2, t1, p1
convubw t1, s2
mullw t1, t1, p2
addw t2, t2, t1
convubw t1, s3
mullw t1, t1, p3
addw t2, t2, t1
convubw t1, s4
mullw t1, t1, p4
addw t2, t2, t1
addw t2, t2, 8
shrsw t2, t2, 4
convsuswb d1, t2


.function orc_combine2_8xn_u8
.flags 2d
.n 8
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_combine2_12xn_u8
.flags 2d
.n 12
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_combine2_16xn_u8
.flags 2d
.n 16
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_combine2_nxm_u8
.flags 2d
.dest 1 d1 uint8_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t
.param 2 p1
.param 2 p2
.param 2 p3
.param 2 p4
.temp 2 t1
.temp 2 t2

convubw t1, s1
convubw t2, s2
mullw t1, t1, p1
mullw t2, t2, p2
addw t1, t1, t2
addw t1, t1, p3
shrsw t1, t1, p4
convsuswb d1, t1



.function orc_sad_nxm_u8
.flags 2d
.accumulator 4 a1 uint32_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

accsadubl a1, s1, s2


.function orc_sad_8x8_u8
.flags 2d
.n 8
.m 8
.accumulator 4 a1 uint32_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

accsadubl a1, s1, s2



.function orc_sad_12x12_u8
.flags 2d
.n 12
.m 12
.accumulator 4 a1 uint32_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

accsadubl a1, s1, s2



.function orc_sad_16xn_u8
.flags 2d
.n 16
.accumulator 4 a1 uint32_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

accsadubl a1, s1, s2



.function orc_sad_32xn_u8
.flags 2d
.n 32
.accumulator 4 a1 uint32_t
.source 1 s1 uint8_t
.source 1 s2 uint8_t

accsadubl a1, s1, s2



