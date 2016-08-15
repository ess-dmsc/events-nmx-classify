#ifndef TPC_DIMENSIONS_H
#define TPC_DIMENSIONS_H

#include <vector>

namespace TPC
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
