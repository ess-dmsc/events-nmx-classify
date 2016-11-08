#ifndef TIMER_H_
#define TIMER_H_

#include <string>
#include <cstdint>

#include <boost/thread.hpp>
#include <boost/timer/timer.hpp>

class Timer: public boost::timer::cpu_timer
{
public:
  Timer(bool start = false) {if (!start) stop(); timeout_limit = 0;}
  Timer(uint64_t timeout, bool start = false) { if (!start) stop(); timeout_limit = timeout;}
  double s () {return elapsed().wall / secs;}
  double ms () {return elapsed().wall / msecs;}
  double us () {return elapsed().wall / usecs;}
  double ns() {return elapsed().wall;}
  bool timeout() {
    return (s() > static_cast<double>(timeout_limit));
  }

  inline static void wait_ms(int millisex)
  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(millisex));
  }

  inline static void wait_us(int microsex)
  {
    boost::this_thread::sleep(boost::posix_time::microseconds(microsex));
  }

  inline std::string done()
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
  
  inline std::string ETA()
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

private:
  static constexpr double secs  = 1000000000;
  static constexpr double msecs = 1000000;
  static constexpr double usecs = 1000;
  uint64_t timeout_limit;


};

#endif