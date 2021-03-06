#pragma once

#include <string>
#include <cstdint>
#include <cmath>

#include <boost/timer/timer.hpp>

class CustomTimer : public boost::timer::cpu_timer {
private:
  const double secs = std::pow(10, 9);
  const double msecs = std::pow(10, 6);
  const double usecs = std::pow(10, 3);
  uint64_t timeout_limit;
public:
  CustomTimer(bool start = false)
  {
    if (!start)
      stop();
    timeout_limit = 0;
  }

  CustomTimer(uint64_t timeout, bool start = false)
  {
    if (!start)
      stop();
    timeout_limit = timeout;
  }

  double s () {return elapsed().wall / secs;}
  double ms () {return elapsed().wall / msecs;}
  double us () {return elapsed().wall / usecs;}
  double ns() {return elapsed().wall;}
  bool timeout()
  {
    return (s() > static_cast<double>(timeout_limit));
  }

  std::string done()
  {
    uint64_t e_tot = static_cast<uint64_t>(s());
    uint64_t e_h = e_tot / 3600;
    uint64_t e_m = (e_tot % 3600) / 60;
    uint64_t e_s = (e_tot % 60);

    std::string answer = std::to_string(e_h) + ":";
    if (e_m < 10) answer += "0";
    answer += std::to_string(e_m) + ":";
    if (e_s < 10) answer += "0";
    answer += std::to_string(e_s);
    return answer;
  }
  
  std::string ETA()
  {
    uint64_t ETA_tot = static_cast<uint64_t>(
        ceil(static_cast<double>(timeout_limit) - s())
                                   );
    uint64_t ETA_h = ETA_tot / 3600;
    uint64_t ETA_m = (ETA_tot % 3600) / 60;
    uint64_t ETA_s = (ETA_tot % 60);

    std::string answer = std::to_string(ETA_h) + ":";
    if (ETA_m < 10) answer += "0";
    answer += std::to_string(ETA_m) + ":";
    if (ETA_s < 10) answer += "0";
    answer += std::to_string(ETA_s);
    return answer;
  }
};

