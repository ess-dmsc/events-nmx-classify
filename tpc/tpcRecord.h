#ifndef NMX_RECORD_H
#define NMX_RECORD_H

#include <vector>
#include <list>
#include <map>

namespace TPC
{

struct Dimensions
{
  int strips { 250 };
  double min {-50};
  double max { 50};

  double transfrom(double strip)
    { return (strip + 0.5) * (max - min) / double(strips) + min; }
};

class Strip
{
public:
  Strip() {}
  Strip(const std::vector<short> &d);
  std::vector<short>  data() const;

  std::string debug() const;

  std::vector<short> suppress_negatives();

  bool nonzero() const {return nonzero_;}
  short hitbins() const {return hitbins_;}
  uint64_t integral() const {return integral_;}
  double integral_normalized() const {return integral_normalized_;}
  std::list<size_t> maxima() const {return maxima_;}
  std::list<size_t> global_maxima() const {return global_maxima_;}

private:
  std::vector<short> data_;

  bool              nonzero_ {false};
  short             hitbins_ {0};
  uint64_t          integral_ {0};
  double            integral_normalized_ {0.0};
  std::list<size_t> maxima_;
  std::list<size_t> global_maxima_;

  void find_maxima();
  void find_global_maxima();
};

class Record
{
public:
  void add_strip(int istrip, const std::vector<short> &raw);

  bool empty() const;
  std::string debug() const;

  size_t strip_start() const;
  size_t strip_stop() const;
  std::list<size_t> valid_strips() const;

  size_t num_time_bins() const;

  uint16_t hitbins() const {return hitbins_;}
  uint64_t integral() const {return integral_;}
  double integral_normalized() const {return integral_normalized_;}

  short get(size_t strip, size_t timebin) const;
  Strip get_strip(size_t strip) const;


  // save/load with zero suppression
  std::list<short> save() const;
  void load(std::list<short>);

  void suppress_negatives();

private:
  std::map<size_t, Strip> strips_;

  int start_ {-1};
  int stop_ {-1};
  size_t num_time_bins_ {0};

  uint16_t          hitbins_ {0};
  uint64_t          integral_ {0};
  double            integral_normalized_ {0.0};

};

struct Event
{
  Record x, y;

  bool empty() const
  {
    return (x.empty() && y.empty());
  }

  void suppress_negatives()
  {
    x.suppress_negatives();
    y.suppress_negatives();
  }

  std::string debug() const
  {
    return "X: " + x.debug() + "\n"
           + "Y: " + y.debug();
  }

  short hitbins() const { return x.hitbins() + y.hitbins(); }
  uint64_t integral() const {return x.integral() + y.integral(); }
  double integral_normalized() const
  {
    double ret = 0;
    if (hitbins())
      ret = integral() / double(hitbins());
    return ret;
  }
};

}

#endif
