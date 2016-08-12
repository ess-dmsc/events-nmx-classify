#ifndef MimicVMMx_HH
#define MimicVMMx_HH

#include <list>

#include "tpcRecord.h"

namespace TPC {

/** \struct VMMxDataPoint
 *  \brief Struct to hold VMMx values
 *
 *  This struct hold the information from one maxima found in onse strip.
 *  It contains the timebin where the maxium starts and the length
 *  (in timebins) of the maxima. The bin of maximum ADC alongth with the
 *  ADC value is contained.
 */

struct VMMxDataPoint
{
    /**> strip Stripnumber */
    int strip;
    /**> locMaxStart First time-bin over ADC threshold */
    int locMaxStart;
    /**> locMaxLength Number of time-bins over threshold */
    int locMaxLength;
    /**> maxBin Time-bin with maximum ADC-value */
    int maxBin;
    /**> maxADC Maximum ADC-value */
    int maxADC;
};

/**
 * \class MimicVMMx
 * \brief Mimic VMM2 og VMM3 data reduction
 *
 * This class mimics the expected data reduction of the
 * VMM2 and VMM3 chips used to read and process the data
 * from the NMX GEM modules.
 *
 * Input: Two arrays for X and Y respectively.
 *        Dimension[NSTRIPS, NTIMEBINS]
 *
 * Output: A list containing VMMxDataPoints
 *
 * Author: Carsten Søgaard, eScience, NBI
 *         soegaard@nbi.dk
 *
 */

class MimicVMMx
{
public:

  /** Main analysis function.
     *  When called, the contents of the arrays is processed */
  std::list<VMMxDataPoint> processEvent(const Record &event);

  /** Set the ADC threshold */
  void setADCThreshold(int16_t ADCthreshold);
  /** Set the minimum number of timebins over the ADC threshold required
     *  for a local max */
  void setNTimebinsOverThreshold(int NtbOverThreshold);

private:
  /** Threshold for an ADC-value to be considered beloning to a maxima */
  int16_t adcthreshold {0};
  /** Minimum length of the maxima to be stored */
  int   tboverthreshold {0};

  std::list<VMMxDataPoint> examine_strip(const std::vector<int16_t>&, int stripID);
};

}

#endif
