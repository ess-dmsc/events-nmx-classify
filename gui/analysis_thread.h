#ifndef ANALYSIS_THREAD_H_
#define ANALYSIS_THREAD_H_

#include <QThread>
#include <cstdint>
#include <boost/atomic.hpp>

#include "tpcReader.h"
#include <memory>

#include "entry2d.h"


class AnalysisThread : public QThread
{
    Q_OBJECT
public:
    explicit AnalysisThread(QObject *parent = 0);
    ~AnalysisThread();

    void set_refresh_frequency(int);
    void set_bounds(int min, int max);
    void set_box_bounds(int x1, int x2, int y1, int y2);
    void go(std::shared_ptr<TPC::Reader> r, QString weight_type, double normalize_by);
    void terminate();

signals:
    void run_complete();
    void data_ready(std::shared_ptr<EntryList> data, double percent);
    void histogram_ready(std::shared_ptr<EntryList> histo, QString codomain);
    void subhist_ready(std::shared_ptr<EntryList> histo, QString codomain);

protected:
    void run();

private:
    std::shared_ptr<TPC::Reader> reader_;

    boost::atomic<bool> terminating_;
    boost::atomic<int>  refresh_frequency_;

    QString weight_type_;
    double normalize_by_ {1};
    double percent {0};

    boost::atomic<int> min_ {std::numeric_limits<int>::min()};
    boost::atomic<int> max_ {std::numeric_limits<int>::max()};

    boost::atomic<int> x1_ {std::numeric_limits<int>::min()};
    boost::atomic<int> x2_ {std::numeric_limits<int>::max()};
    boost::atomic<int> y1_ {std::numeric_limits<int>::min()};
    boost::atomic<int> y2_ {std::numeric_limits<int>::max()};


    std::map<int,std::map<std::pair<int,int>, double>> data_;

    void make_projection();
};

#endif
