#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include <map>
#include "Event.h"

namespace NMX {

class Metric
{
public:
  Metric() {}
  Metric(std::string descr)
    : description_(descr)
  {}

  void add(size_t idx, double val);
  void write_H5(H5CC::DataSet dataset) const;
  void read_H5(const H5CC::DataSet &dataset);
  void read_H5_data(const H5CC::DataSet &dataset);

  std::vector<double>& data() { return data_; }
  std::string description() const { return description_; }
  double min() const { return min_; }
  double max() const { return max_; }
  double sum() const { return sum_; }

  double normalizer() const;

private:
  std::vector<double> data_;
  std::string description_;
  double min_{std::numeric_limits<double>::max()};
  double max_{std::numeric_limits<double>::min()};
  double sum_{0};
};

class Analysis
{
public:
  Analysis() {}
  Analysis(H5CC::Group group, size_t eventnum);
  ~Analysis() { save(); }

  std::list<std::string> metrics() const;
  Metric metric(std::string name) const;
  void set_parameters(const Settings&);

  Settings parameters() const { return params_; }
  size_t num_analyzed() const { return num_analyzed_; }
  std::string name() const { return group_.name(); }

  void analyze_event(size_t index, Event event);
  Event gather_metrics(size_t index, Event event) const;

private:
  Settings params_ { Event().parameters() };
  size_t num_analyzed_ {0};
  size_t max_num_ {0};
  std::map<std::string, Metric> metrics_;
  std::map<std::string, H5CC::DataSet> datasets_;
  H5CC::Group group_;

  void save();
};


class FileAPV
{
public:
  FileAPV(std::string filename);

  size_t event_count() const;
  Event get_event(size_t index) const;
  void analyze_event(size_t index);

  size_t num_analyzed() const;

  void set_parameters(const Settings&);
  Settings parameters() const;
  std::list<std::string> metrics() const;
  Metric get_metric(std::string cat) const;

  std::list<std::string> analyses() const;
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
