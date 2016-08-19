#ifndef ANALYSIS_THREAD_H_
#define ANALYSIS_THREAD_H_

#include <QThread>
#include <QString>
#include <QVector>
#include <cstdint>
#include <boost/atomic.hpp>

#include "Reader.h"
#include <memory>

#include "entry2d.h"

struct HistParams
{
  int x1 {std::numeric_limits<int>::min()},
      x2{std::numeric_limits<int>::max()};
  int y1 {std::numeric_limits<int>::min()},
      y2{std::numeric_limits<int>::max()};
  double cutoff {0};
};

struct HistSubset
{
  double min{std::numeric_limits<int>::max()};
  double max{std::numeric_limits<int>::min()};
  double avg{0};
  double total_count{0};
  EntryList data;
};

using MultiHists = QVector<HistSubset>;

class AnalysisThread : public QThread
{
    Q_OBJECT
public:
    explicit AnalysisThread(QObject *parent = 0);
    ~AnalysisThread();

    void set_refresh_frequency(int);
    void set_bounds(int min, int max);

    void request_hists(QVector<HistParams>);
    void go(std::shared_ptr<NMX::Reader> r, QString weight_type, double normalize_by);
    void terminate();

signals:
    void run_complete();
    void data_ready(std::shared_ptr<EntryList> data, double percent);

    void hists_ready(std::shared_ptr<MultiHists>);

protected:
    void run();

private:
    std::shared_ptr<NMX::Reader> reader_;

    boost::atomic<bool> terminating_;
    boost::atomic<int>  refresh_frequency_;

    QString weight_type_;
    double normalize_by_ {1};
    double percent {0};

    boost::atomic<int> min_ {std::numeric_limits<int>::min()};
    boost::atomic<int> max_ {std::numeric_limits<int>::max()};

    QVector<HistParams> subset_params_;

    std::map<int,std::map<std::pair<int,int>, double>> data_;

    void make_projections();
};

#endif
