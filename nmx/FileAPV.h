#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include <memory>
#include <map>
#include "Event.h"

namespace NMX {

struct Metrics
{
  Metrics() {}
  Metrics(size_t size, std::string descr)
    : description(descr)
  {
    data.resize(size, 0);
  }

  void add(size_t idx, double val)
  {
    if (idx >= data.size())
      return;
    data[idx] = val;
    min = std::min(min, val);
    max = std::max(max, val);
    sum += val;
  }

  void write_H5(H5CC::Group group, std::string name) const;
  void read_H5(const H5CC::Group &group, std::string name);

  std::vector<double> data;
  std::string description;
  double min{std::numeric_limits<double>::max()};
  double max{std::numeric_limits<double>::min()};
  double sum{0};
};


class FileAPV
{
public:
  FileAPV(std::string filename);
  ~FileAPV() { save_analysis(); }

  size_t event_count();
  Event get_event(size_t index);
  void analyze_event(size_t index);

  size_t num_analyzed() const;

  void set_parameters(const Settings&);
  Settings get_parameters() const {return analysis_params_;}
  std::list<std::string> metrics() const;
  Metrics get_metric(std::string cat); //const?

  std::list<std::string> analysis_groups() const;
  bool create_analysis(std::string name);
  void delete_analysis(std::string name);
  bool load_analysis(std::string name);
  bool save_analysis();

private:
  H5CC::File file_;
  size_t event_count_ {0};

  H5CC::DataSet  dataset_;

  std::list<std::string> analysis_groups_;

  std::string current_analysis_name_;
  Settings analysis_params_;
  std::map<std::string, Metrics> metrics_;
  size_t num_analyzed_ {0};

  Record read_record(size_t index, size_t plane);
  void read_analysis_groups();

  void push_event_metrics(size_t index, const Event& event);

  void clear_analysis();
};

}

#endif
