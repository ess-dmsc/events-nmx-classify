#include "Subhist.h"

#include "CustomLogger.h"

void Histogram::add_to_hist(int32_t x, int32_t y, int32_t bin, int64_t increment)
{
  if ((x < x1_) || (x > x2_)
      || (y < y1_) || (y > y2_))
    return;

  weighted_sum_ += bin*increment;
  total_count_ += increment;
}


void Histogram::reset_data()
{
  weighted_sum_ = 0;
  total_count_ = 0;
}

double Histogram::avg() const
{
  if (total_count_ != 0)
    return weighted_sum_ / total_count_;
  else
    return 0;
}

void Histogram::set_x(int32_t new_x1, int32_t new_x2)
{
  x1_ = std::min(new_x1, new_x2);
  x2_ = std::max(new_x1, new_x2);
  reset_data();
}

void Histogram::set_y(int32_t new_y1, int32_t new_y2)
{
  y1_ = std::min(new_y1, new_y2);
  y2_ = std::max(new_y1, new_y2);
  reset_data();
}

void Histogram::set_width(int64_t w)
{
  auto center = center_x();
  x1_ = center - w / 2;
  x2_ = center + w / 2;
  reset_data();
}

void Histogram::set_height(int64_t h)
{
  auto center = center_y();
  y1_ = center - h / 2;
  y2_ = center + h / 2;
  reset_data();
}

void Histogram::set_center_x(int64_t x)
{
  auto w = width();
  x1_ = x - w/2;
  x2_ = x + w/2;
  reset_data();
}

void Histogram::set_center_y(int64_t y)
{
  auto h = height();
  y1_ = y - h/2;
  y2_ = y + h/2;
  reset_data();
}

