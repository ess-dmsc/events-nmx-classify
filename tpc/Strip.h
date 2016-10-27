#ifndef NMX_STRIP_H
#define NMX_STRIP_H

#include <vector>
#include <map>
#include <list>
#include <string>

namespace NMX
{

class Strip
{
public:
  Strip() {}
  Strip(const std::vector<int16_t> &data);
  Strip(const std::map<size_t, int16_t> &data);
  Strip suppress_negatives() const;

  int16_t value(int16_t idx) const;
  std::vector<int16_t>  as_vector() const;

  std::string debug() const;

  //return analytical values
  bool nonzero() const {return nonzero_;}
  int16_t num_valid() const {return num_valid_;}
  int16_t start() const {return start_;}
  int16_t end() const {return end_;}
  int64_t integral() const {return integral_;}


  void analyze(int16_t adc_threshold, int16_t over_threshold);
  std::list<size_t> maxima() const {return maxima_;}
  std::list<size_t> global_maxima() const {return global_maxima_;}
  std::list<size_t> VMM_maxima() const {return VMM_maxima_;}

private:
//  std::vector<int16_t> data_;
  std::map<size_t, int16_t> data_;

  bool    nonzero_   {false};
  int16_t num_valid_ {0};
  int16_t start_     {-1};
  int16_t end_       {-1};
  int64_t integral_  {0};

  std::list<size_t> maxima_;
  std::list<size_t> global_maxima_;
  std::list<size_t> VMM_maxima_;

  void find_maxima(int16_t adc_threshold);
  void find_global_maxima();
  void find_VMM_maxima(int16_t adc_threshold, int16_t over_threshold);
};


}

#endif
