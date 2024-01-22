#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orccpu.h>

int _orc_data_cache_size_level1;
int _orc_data_cache_size_level2;
int _orc_data_cache_size_level3;
int _orc_cpu_family;
int _orc_cpu_model;
int _orc_cpu_stepping;
const char *_orc_cpu_name = "unknown";

void
orc_get_data_cache_sizes (int *level1, int *level2, int *level3)
{
  if (level1) {
    *level1 = _orc_data_cache_size_level1;
  }
  if (level2) {
    *level2 = _orc_data_cache_size_level2;
  }
  if (level3) {
    *level3 = _orc_data_cache_size_level3;
  }

}

void
orc_get_cpu_family_model_stepping (int *family, int *model, int *stepping)
{
  if (family) {
    *family = _orc_cpu_family;
  }
  if (model) {
    *model = _orc_cpu_model;
  }
  if (stepping) {
    *stepping = _orc_cpu_stepping;
  }
}

const char *
orc_get_cpu_name (void)
{
  return _orc_cpu_name;
}


