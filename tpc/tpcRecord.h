#ifndef NMX_RECORD_H
#define NMX_RECORD_H

#include <vector>
#include <list>
#include <map>
#include <string>

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
  Strip(const std::vector<int16_t> &d);
  std::vector<int16_t>  data() const;

  std::string debug() const;

  std::vector<int16_t> suppress_negatives();

  bool nonzero() const {return nonzero_;}
  int16_t hitbins() const {return hitbins_;}
  int16_t binstart() const {return bin_start_;}
  int16_t binstop() const {return bin_stop_;}
  uint64_t integral() const {return integral_;}
  std::list<size_t> maxima() const {return maxima_;}
  std::list<size_t> global_maxima() const {return global_maxima_;}

private:
  std::vector<int16_t> data_;

  bool              nonzero_ {false};
  int16_t             hitbins_ {0};
  int16_t             bin_start_{-1};
  int16_t             bin_stop_{-1};
  uint64_t          integral_ {0};
  std::list<size_t> maxima_;
  std::list<size_t> global_maxima_;

  void find_maxima();
  void find_global_maxima();
};

class Record
{
public:
  void add_strip(int istrip, const std::vector<int16_t> &raw);

  bool empty() const;
  std::string debug() const;

  size_t strip_start() const;
  size_t strip_stop() const;
  std::list<size_t> valid_strips() const;

  size_t max_time_bins() const {return max_time_bins_;}

  int16_t get(size_t strip, size_t timebin) const;
  Strip get_strip(size_t strip) const;


  // save/load with zero suppression
  std::list<int16_t> save() const;
  void load(std::list<int16_t>);

  void suppress_negatives();

  void analyze();
  double analytic(std::string) const;  //default 0
//  std::list<std::string> catagories() const;

private:
  std::map<size_t, Strip> strips_;

  int start_ {-1};
  int stop_ {-1};
  size_t max_time_bins_ {0};


  std::map<std::string, double> analytics_;
};

struct Event
{
  Record x, y;
  std::map<std::string, double> analytics_;

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

  void analyze();
  double analytic(std::string id) const  //default 0
  {
    if (analytics_.count(id))
      return analytics_.at(id);
    else
      return 0;
  }
};

}

#endif
