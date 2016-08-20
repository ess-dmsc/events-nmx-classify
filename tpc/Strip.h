#ifndef NMX_STRIP_H
#define NMX_STRIP_H

#include <vector>
#include <list>
#include <string>

namespace NMX
{

class Strip
{
public:
  Strip() {}
  explicit Strip(const std::vector<int16_t> &d);
  Strip suppress_negatives() const;
  int16_t value(int16_t timebin) const;

  std::vector<int16_t>  raw_data() const;

  std::string debug() const;

  //return analytical values
  bool nonzero() const {return nonzero_;}
  int16_t num_valid_bins() const {return num_valid_bins_;}
  int16_t bin_start() const {return start_;}
  int16_t bin_end() const {return end_;}
  uint64_t integral() const {return integral_;}


  void analyze(int adc_threshold, int tb_over_threshold);
  std::list<size_t> maxima() const {return maxima_;}
  std::list<size_t> global_maxima() const {return global_maxima_;}
  std::list<size_t> VMM_maxima() const {return VMM_maxima_;}

private:
  std::vector<int16_t> data_;

  int adc_threshold_ {0};
  int tb_over_threshold_{1};

  bool              nonzero_ {false};
  int16_t           num_valid_bins_  {0};
  int16_t           start_          {-1};
  int16_t           end_            {-1};
  uint64_t          integral_        {0};

  std::list<size_t> maxima_;
  std::list<size_t> global_maxima_;
  std::list<size_t> VMM_maxima_;

  void find_maxima();
  void find_global_maxima();
  void find_VMM_maxima();
};


}

#endif
