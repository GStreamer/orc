
.function audio_add_mono_to_stereo_scaled_s16
.dest 4 d1 short
.source 4 s1 short
.source 2 s2 short
.param 2 volume
.temp 4 s2_scaled
.temp 2 t
.temp 4 s2_stereo

mulswl s2_scaled, s2, volume
shrsl s2_scaled, s2_scaled, 12
convssslw t, s2_scaled
mergewl s2_stereo, t, t
x2 addssw d1, s1, s2_stereo

