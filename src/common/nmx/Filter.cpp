#include "Filter.h"
#include "doFit.h"

bool MetricTest::operator == (const MetricTest& other) const
{
  return (enabled == other.enabled) &&
      (round_before_compare  == other.round_before_compare) &&
      (metric == other.metric) &&
      (min == other.min) &&
      (max == other.max);
}

MetricTest::MetricTest(std::string name, const NMX::Metric& m)
{
  min = m.min();
  max = m.max();
  metric = name;
  enabled = true;
}

bool MetricTest::validate(const NMX::Metric& m, size_t index) const
{
  if (index >= m.const_data().size())
    return false;
  double val = m.const_data().at(index);
  if (round_before_compare)
    val = std::round((val - m.min()) / m.normalizer());
  return (min <= val) && (val <= max);
}

void to_json(json& j, const MetricTest &s)
{
  j["metric"] = s.metric;
  j["enabled"] = s.enabled;
  j["round_before_compare"] = s.round_before_compare;
  j["min"] = s.min;
  j["max"] = s.max;
}

void from_json(const json& j, MetricTest &s)
{
  if (j.count("metric") && j["metric"].is_string())
    s.metric = j["metric"];
  if (j.count("enabled") && j["enabled"].is_boolean())
    s.enabled = j["enabled"];
  if (j.count("round_before_compare")
      && j["round_before_compare"].is_boolean())
    s.round_before_compare = j["round_before_compare"];
  if (j.count("min") && j["min"].is_number_float())
    s.min = j["min"];
  if (j.count("max") && j["max"].is_number_float())
    s.max = j["max"];
}

HistMap1D Histogram1D::map() const
{
  return map_;
}

uint64_t Histogram1D::count() const
{
  return count_;
}

HistList1D Histogram1D::list() const
{
  return to_list(map_);
}

void Histogram1D::add_one(double bin)
{
  map_[bin]++;
  count_++;
}

void Histogram1D::clear()
{
  map_.clear();
  count_ = 0;
}

std::list<std::string> Histogram1D::values()
{
  return {"mean", "median", "mode", "midrange", "RMS", "harmonic mean"};
}

double Histogram1D::get_value(std::string t) const
{
  if (t == "mean")
    return mean();
  else if (t == "median")
    return median();
  else if (t == "mode")
    return mode();
  else if (t == "midrange")
    return midrange();
  else if (t == "RMS")
    return RMS();
  else if (t == "harmonic mean")
    return harmonic_mean();
  return std::numeric_limits<double>::quiet_NaN();
}

double Histogram1D::mean() const
{
  double sum {0};
  for (const auto& m : map_)
    sum += m.first * m.second;
  return sum / static_cast<double>(count_);
}

double Histogram1D::median() const
{
  double med_count = static_cast<double>(count_) / 2.0;
  double mcount {0};
  double prev = std::numeric_limits<double>::quiet_NaN();
  for (const auto& m : map_)
  {
    mcount += m.second;
    if (mcount > med_count)
      break;
    prev = m.first;
  }
  return prev;
}

double Histogram1D::mode() const
{
  double maxcount = std::numeric_limits<double>::min();
  double maxbin = std::numeric_limits<double>::quiet_NaN();
  for (const auto& m : map_)
    if (m.second > maxcount)
    {
      maxcount = m.second;
      maxbin = m.first;
    }
  return maxbin;
}

double Histogram1D::harmonic_mean() const
{
  double sum {0};
  for (const auto& m : map_)
    sum += m.second / m.first;
  return static_cast<double>(count_) / sum;
}

double Histogram1D::RMS() const
{
  double sum {0};
  for (const auto& m : map_)
    sum += m.first * m.first * m.second;
  return sqrt(sum / static_cast<double>(count_));
}

double Histogram1D::midrange() const
{
  if (map_.empty())
    return std::numeric_limits<double>::quiet_NaN();
  return (map_.begin()->first + map_.rbegin()->first) / 2.0;
}


MetricFilter MetricFilter::cull_disabled() const
{
  MetricFilter ret;
  for (auto t : tests_)
    if (t.enabled)
      ret.tests_.push_back(t);
  return ret;
}

bool MetricFilter::operator == (const MetricFilter& other) const
{
  return (tests_ == other.tests_);
}

bool MetricFilter::validate(
    const std::map<std::string, NMX::Metric>& metrics,
    size_t index) const
{
  for (auto f : tests_)
  {
    if (!f.enabled)
      continue;
    if (!metrics.count(f.metric))
      return false;
    const auto& metric = metrics.at(f.metric);
    if (!f.validate(metric, index))
      return false;
  }
  return true;
}

void MetricFilter::clear()
{
  tests_.clear();
}

size_t MetricFilter::size() const
{
  return tests_.size();
}

MetricTest MetricFilter::test(size_t i) const
{
  if (i >= tests_.size())
    return MetricTest();
  return tests_.at(i);
}

void MetricFilter::set_test(size_t i, MetricTest t)
{
  if (tests_.size() <= i)
    tests_.resize(i+1);
  tests_[i] = t;
}

void MetricFilter::remove(size_t i)
{
  if (i < tests_.size())
    tests_.erase(tests_.begin() + i);
}

void MetricFilter::up(size_t i)
{
  if ((i < tests_.size()) && (i > 0))
    std::swap(tests_[i-1], tests_[i]);
}

void MetricFilter::down(size_t i)
{
  if ((i +1 ) < tests_.size())
    std::swap(tests_[i+1], tests_[i]);
}

std::vector<MetricTest> MetricFilter::tests() const
{
  return tests_;
}

void MetricFilter::add(MetricTest t)
{
  tests_.push_back(t);
}

std::list<std::string> MetricFilter::required_metrics() const
{
  std::list<std::string> ret;
  for (auto t : tests_)
    if (t.enabled)
      ret.push_back(t.metric);
  return ret;
}

std::set<size_t> MetricFilter::get_indices(const NMX::File &file) const
{
  std::set<size_t> ret;
  if (!file.event_count())
    return ret;

  std::map<std::string, NMX::Metric> metrics;
  for (auto m : required_metrics())
    metrics[m] = file.get_metric(m);

  for (size_t i = 0; i < file.num_analyzed(); ++i)
    if (validate(metrics, i))
      ret.insert(i);
  return ret;
}

Histogram1D MetricFilter::get_projection(const NMX::File& file,
                                         std::string proj_metric) const
{
  Histogram1D ret;

  std::map<std::string, NMX::Metric> metrics;
  for (auto m : required_metrics())
    metrics[m] = file.get_metric(m);
  auto projection = file.get_metric(proj_metric);
  auto norm = projection.normalizer();

  for (size_t i = 0; i < file.num_analyzed(); ++i)
    if (validate(metrics, i))
      ret.add_one(int( projection.data().at(i) / norm) * norm);

  return ret;
}

void to_json(json& j, const MetricFilter &s)
{
  j["tests"] = s.tests_;
}

void from_json(const json& j, MetricFilter &s)
{
  s.tests_.clear();
  if (!j.count("tests"))
    return;
  for (auto k : j["tests"])
    s.tests_.push_back(k.get<MetricTest>());
}

IndepVariable::IndepVariable(MetricTest m)
{
  metric = m.metric;
  start = m.min;
  end = m.max;
}

bool IndepVariable::operator == (const IndepVariable& other) const
{
  return (metric == other.metric) &&
      (start == other.start) &&
      (end == other.end) &&
      (step == other.step) &&
      (width == other.width) &&
      (vary_min == other.vary_min) &&
      (vary_max == other.vary_max);
}


void to_json(json& j, const IndepVariable &s)
{
  j["metric"] = s.metric;
  j["vary_min"] = s.vary_min;
  j["vary_max"] = s.vary_max;
  j["start"] = s.start;
  j["end"] = s.end;
  j["step"] = s.step;
  j["width"] = s.width;
}

void from_json(const json& j, IndepVariable &s)
{
  if (j.count("metric") && j["metric"].is_string())
    s.metric = j["metric"];
  if (j.count("start") && j["start"].is_number_float())
    s.start = j["start"];
  if (j.count("end") && j["end"].is_number_float())
    s.end = j["end"];
  if (j.count("step") && j["step"].is_number_float())
    s.step = j["step"];
  if (j.count("width") && j["width"].is_number_float())
    s.width = j["width"];
  if (j.count("vary_min") && j["vary_min"].is_boolean())
    s.vary_min = j["vary_min"];
  if (j.count("vary_max") && j["vary_max"].is_boolean())
    s.vary_max = j["vary_max"];
}

void FilterMerits::doit(const NMX::File& f, std::string proj)
{
  total_count = filter.get_projection(f, proj).count();
  if (!total_count)
    return;

  MetricTest iv;
  iv.enabled = true;
  iv.metric = indvar.metric;
  iv.min = iv.max = 0; // not quite general enough...
  auto endp = indvar.end;
  if (indvar.vary_min && indvar.vary_max)
    endp -= indvar.width;
  for (double i=indvar.start; i <= endp; i+=indvar.step)
  {
    if (indvar.vary_min && indvar.vary_max)
    {
      iv.min = i;
      iv.max = i + indvar.width - 1;
    }
    else if (indvar.vary_min)
      iv.min = i;
    else if (indvar.vary_max)
      iv.max = i;

    MetricFilter filter2 = filter.cull_disabled();
    filter2.add(iv);

    auto hist = filter2.get_projection(f, proj);
    EdgeFitter fitter(hist.map());
    fitter.analyze(fit_type);

    if (!fitter.reasonable())
      continue;

    val_min.push_back(iv.min);
    val_max.push_back(iv.max);
    count.push_back(hist.count());
    efficiency.push_back(double(hist.count()) / double(total_count) * 100.0);
    res.push_back(fitter.resolution(units));
    reserr.push_back(fitter.resolution_error(units));
    pos.push_back(fitter.position(units));
    poserr.push_back(fitter.position_error(units));
    signal.push_back(fitter.signal());
    signalerr.push_back(fitter.signal_error());
    back.push_back(fitter.background());
    backerr.push_back(fitter.background_error());
    snr.push_back(fitter.snr());
    snrerr.push_back(fitter.snr_error());
  }
}

void FilterMerits::save(H5CC::Group& group) const
{
  group.clear();

  auto bgroup = group.require_group("baseline_filters");
  H5CC::from_json(filter.cull_disabled(), bgroup);
  auto igroup = group.require_group("independent_variable");
  H5CC::from_json(indvar, igroup);

  H5CC::Dataset dset
      = group.require_dataset<double>("results", {count.size(),14});
  dset.write(val_min, {count.size(), 1}, {0,0});
  dset.write(val_max, {count.size(), 1}, {0,1});
  dset.write(count, {count.size(), 1}, {0,2});
  dset.write(efficiency, {count.size(), 1}, {0,3});
  dset.write(res, {count.size(), 1}, {0,4});
  dset.write(reserr, {count.size(), 1}, {0,5});
  dset.write(pos, {count.size(), 1}, {0,6});
  dset.write(poserr, {count.size(), 1}, {0,7});
  dset.write(signal, {count.size(), 1}, {0,8});
  dset.write(signalerr, {count.size(), 1}, {0,9});
  dset.write(back, {count.size(), 1}, {0,10});
  dset.write(backerr, {count.size(), 1}, {0,11});
  dset.write(snr, {count.size(), 1}, {0,12});
  dset.write(snrerr, {count.size(), 1}, {0,13});

  dset.write_attribute("baseline_total_count", uint32_t(total_count));
  dset.write_attribute("resolution_pitch", units);
  dset.write_attribute("fit_type", fit_type);
  dset.write_attribute("columns", std::string(
                         "val_min, val_max, count, %count, resolution, "
                         "resolution_uncert, position, position_uncert, "
                         "signal, signal_uncert, background, background_uncert, "
                         "SnR, SnR_uncert"));
}

void FilterMerits::load(const H5CC::Group& group)
{
  json jf = group.open_group("baseline_filters");
  filter = jf;
  json ji = group.open_group("independent_variable");
  indvar = ji;

  auto dset = group.open_dataset("results");
  units = dset.read_attribute<uint32_t>("resolution_pitch");
  fit_type = dset.read_attribute<std::string>("fit_type");
}

bool FilterMerits::operator == (const FilterMerits& other) const
{
  return (filter == other.filter) &&
      (indvar == other.indvar) &&
      (fit_type == other.fit_type) &&
      (units == other.units);
}



