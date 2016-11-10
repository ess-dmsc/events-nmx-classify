#ifndef THREAD_CLASSIFY_H_
#define THREAD_CLASSIFY_H_

#include <QThread>
#include <QAtomicInt>

#include "FileAPV.h"
#include <memory>


class ThreadClassify : public QThread
{
    Q_OBJECT
public:
    explicit ThreadClassify(QObject *parent = 0);
    ~ThreadClassify();

    void set_refresh_frequency(int);

    void go(std::shared_ptr<NMX::FileAPV> r);
    void load(std::shared_ptr<NMX::FileAPV> r, QString name);

    void terminate();

signals:
    void run_complete();
    void data_ready(double percent);

protected:
    void run();

    void run_analyze();
    void run_load();

private:
    std::shared_ptr<NMX::FileAPV> reader_;
    QString name_;

    bool analyze_;

    QAtomicInt terminating_;
    QAtomicInt refresh_frequency_;

    double percent {0};
};

#endif
