#ifndef TPC_RECORD_H
#define TPC_RECORD_H

#include "tpcStrip.h"

#include <map>

namespace TPC
{

class Record
{
public:
  void add_strip(int16_t idx, const Strip &raw);

  bool empty() const;
  std::string debug() const;

  int16_t strip_start() const {return strip_start_;}
  int16_t   strip_end() const {return strip_end_;}
  size_t  strip_span() const;

  int16_t time_start() const {return time_start_;}
  int16_t   time_end() const {return time_end_;}
  size_t  time_span() const;

  std::list<int16_t > valid_strips() const;

  int16_t get(int16_t  strip, int16_t  timebin) const;
  Strip get_strip(int16_t  strip) const;


  Record suppress_negatives() const;

  void analyze();
  double analytic(std::string) const;  //default 0
  std::list<std::string> categories() const;

  // save/load with zero suppression
  std::list<int16_t> save() const;

private:
  std::map<int16_t , Strip> strips_;

  int16_t strip_start_ {-1};
  int16_t strip_end_   {-1};
  int16_t time_start_ {-1};
  int16_t time_end_   {-1};

  std::map<std::string, double> analytics_;
};


}

#endif
