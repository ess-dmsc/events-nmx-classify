#include "custom_logger.h"

#include "tpcMimicVMMx.h"

namespace TPC {

std::list<VMMxDataPoint> MimicVMMx::processEvent(const Record &record)
{
  std::list<VMMxDataPoint> ret;

  /** Finds the local maximas in event */

  for (auto i : record.valid_strips())
    ret.splice(ret.end(), examine_strip(record.get_strip(i).data(), i));

  return ret;
}

std::list<VMMxDataPoint> MimicVMMx::examine_strip(const std::vector<int16_t>& strip,
                                                     const int &stripID)
{
  std::list<VMMxDataPoint> ret;

  bool overthreshold {false};
  int tb0 {0};
  int tb1 {0};
  int maxBin {0};
  int16_t maxADC {0};

  for (size_t timebin=0; timebin < strip.size(); ++timebin)
  {
    const auto &adc = strip[timebin];
    if (!overthreshold && (adc >= adcthreshold))
    {
      overthreshold = true;
      tb0 = timebin;
    }

    if (overthreshold && (adc > maxADC))
    {
      maxADC = adc;
      maxBin = timebin;
    }

    if (overthreshold && (adc < adcthreshold))
    {
      overthreshold = false;
      tb1 = timebin;
      if (tb1 - tb0 > tboverthreshold -1)
      {
        VMMxDataPoint dp;
        dp.strip = stripID;
        dp.locMaxStart = tb0;
        dp.locMaxLength = tb1 - tb0 + 1;
        dp.maxBin = maxBin;
        dp.maxADC = maxADC;
        ret.push_back(dp);
      }
    }
  }

  return ret;
}

void MimicVMMx::setADCThreshold(const int16_t &ADCthreshold)
{
  adcthreshold = ADCthreshold;
//  DBG << "<MimicVMMx> ADC peak threshold set to: " << adcthreshold;
}

void MimicVMMx::setNTimebinsOverThreshold(const int &NtbOverThreshold)
{
  tboverthreshold = NtbOverThreshold;
//  DBG << "<MimicVMMx> Required number of timebins over threshold: " << tboverthreshold;
} 

}
