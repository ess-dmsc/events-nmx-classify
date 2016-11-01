#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <QColor>
#include "qp_entry2d.h"

class Histogram
{
public:

  void reset_data();
  void close_data();

  void add_to_hist(int32_t x, int32_t y, int32_t bin, int64_t increment);

  int32_t x1() const { return x1_; }
  int32_t x2() const { return x2_; }
  int32_t y1() const { return y1_; }
  int32_t y2() const { return y2_; }
  int32_t bin_min() const { return bin_min_; }
  int32_t bin_max() const { return bin_max_; }

  int64_t width() const { return int64_t(x2_) - int64_t(x1_); }
  int64_t height() const { return int64_t(y2_) - int64_t(y1_); }
  int64_t center_x() const { return ((int64_t(x2_) + int64_t(x1_)) / 2); }
  int64_t center_y() const { return ((int64_t(y2_) + int64_t(y1_)) / 2); }

  void set_x(int32_t new_x1, int32_t new_x2);
  void set_y(int32_t new_y1, int32_t new_y2);
  void set_bin_bounds(int32_t new1, int32_t new2);

  void set_width(int64_t w);
  void set_height(int64_t h);
  void set_center_x(int64_t x);
  void set_center_y(int64_t y);

  double min() const {return min_;}
  double max() const {return max_;}
  double avg() const;
  double total_count() const {return total_count_;}
  const std::map<int32_t, int64_t> & data() const { return data_; }

  bool visible {true};
  QColor color {Qt::black};


private:
  int32_t x1_{std::numeric_limits<int32_t>::min()};
  int32_t x2_{std::numeric_limits<int32_t>::max()};
  int32_t y1_{std::numeric_limits<int32_t>::min()};
  int32_t y2_{std::numeric_limits<int32_t>::max()};

  int32_t bin_min_{std::numeric_limits<int32_t>::min()};
  int32_t bin_max_{std::numeric_limits<int32_t>::max()};

  double min_{std::numeric_limits<double>::max()};
  double max_{std::numeric_limits<double>::min()};
  double weighted_sum_{0};
  double total_count_{0};
  std::map<int32_t, int64_t> data_;

  bool done_ {false};
};

#endif
