#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include <memory>
#include <map>
#include <atomic>
#include "Event.h"

namespace NMX {

class FileAPV
{
public:
  FileAPV(std::string filename);
  ~FileAPV() { save_analysis(); }

  size_t event_count();
  Event get_event(size_t index);
  void analyze_event(size_t index);

  void clear_analysis();
  size_t num_analyzed() const;

  void set_parameters(const Settings&);
  Settings get_parameters() const {return analysis_params_;}
  std::list<std::string> metrics() const;
  std::vector<Variant> get_metric(std::string cat); //const?
  std::string metric_description(std::string cat) const;

  std::list<std::string> analysis_groups() const;
  bool create_analysis(std::string name);
  void delete_analysis(std::string name);
  bool load_analysis(std::string name);
  bool save_analysis();

  std::shared_ptr<std::atomic<double>> progress() { return progress_; }

private:
  H5CC::File file_;
  size_t event_count_ {0};

  H5CC::DataSet  dataset_;

  std::list<std::string> analysis_groups_;
  std::string current_analysis_name_;
  Settings analysis_params_;
  std::map<std::string, std::vector<Variant>> metrics_;
  std::map<std::string, std::string> metrics_descr_;
  size_t num_analyzed_ {0};

  Record read_record(size_t index, size_t plane);
  void read_analysis_groups();

  void push_event_metrics(size_t index, const Event& event);

  std::shared_ptr<std::atomic<double>> progress_;
};

}

#endif
