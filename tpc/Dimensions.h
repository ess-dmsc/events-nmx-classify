#ifndef NMX_DIMENSIONS_H
#define NMX_DIMENSIONS_H

#include <vector>

namespace NMX
{

struct Dimensions
{
  int16_t strips { 250 };
  double min {-50};
  double max { 50};

  double transform(double strip)
    { return (strip + 0.5) * (max - min) / double(strips) + min; }
};

}

#endif
