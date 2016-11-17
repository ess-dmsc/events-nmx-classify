#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include <memory>
#include <map>
#include "Event.h"

namespace NMX {

struct Metric
{
  Metric() {}
  Metric(size_t size, std::string descr)
    : description(descr)
  {
    data.resize(size, 0);
  }

  void add(size_t idx, double val);
  void write_H5(H5CC::Group group, std::string name) const;
  void read_H5(const H5CC::Group &group, std::string name, bool withdata = true);

  std::vector<double> data;
  std::string description;
  double min{std::numeric_limits<double>::max()};
  double max{std::numeric_limits<double>::min()};
  double sum{0};
};

struct Analysis
{
  Analysis() {}

  void open(H5CC::Group group, std::string name, size_t eventnum);
  void save();
  void clear();

  Metric metric(std::string name) const;
  void set_parameters(const Settings&);

  void analyze_event(size_t index, Event event);
  Event gather_metrics(size_t index, Event event) const;

  std::string name_;
  Settings params_ { Event().parameters() };
  size_t num_analyzed_ {0};
  size_t max_num_ {0};
  std::map<std::string, Metric> metrics_;
  std::map<std::string, H5CC::DataSet> datasets_;
  H5CC::Group group_;
};


class FileAPV
{
public:
  FileAPV(std::string filename);
  ~FileAPV() { analysis_.save(); }

  size_t event_count() const;
  Event get_event(size_t index) const;
  void analyze_event(size_t index);

  size_t num_analyzed() const;

  void set_parameters(const Settings&);
  Settings get_parameters() const {return analysis_.params_;}
  std::list<std::string> metrics() const;
  Metric get_metric(std::string cat) const;

  std::list<std::string> analysis_groups() const;
  void create_analysis(std::string name);
  void delete_analysis(std::string name);
  void load_analysis(std::string name);

private:
  H5CC::File file_;
  H5CC::DataSet  dataset_;
  Analysis analysis_;

  Record read_record(size_t index, size_t plane) const;
};

}

#endif
