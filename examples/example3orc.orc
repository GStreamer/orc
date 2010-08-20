
.function convert_I420_AYUV
.flags 2d
.dest 4 d1
.dest 4 d2
.source 1 y1
.source 1 y2
.source 1 u
.source 1 v
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

