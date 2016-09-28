#ifndef THREAD_CLASSIFY_H_
#define THREAD_CLASSIFY_H_

#include <QThread>
#include <QAtomicInt>

#include "FileHDF5.h"
#include <memory>


class ThreadClassify : public QThread
{
    Q_OBJECT
public:
    explicit ThreadClassify(QObject *parent = 0);
    ~ThreadClassify();

    void set_refresh_frequency(int);

    void go(std::shared_ptr<NMX::FileHDF5> r, NMX::Settings params);
    void terminate();

signals:
    void run_complete();
    void data_ready(double percent);


protected:
    void run();

private:
    std::shared_ptr<NMX::FileHDF5> reader_;

    NMX::Settings parameters_;

    QAtomicInt terminating_;
    QAtomicInt refresh_frequency_;

    double percent {0};
};

#endif