#ifndef QP_ENTRY2D_H
#define QP_ENTRY2D_H

#include <list>
#include <vector>
#include <map>

namespace QPlot
{

using Entry = std::pair<std::vector<int>, double>;
using EntryList = std::list<Entry>;
using HistoData = std::map<double,double>;

}

#endif
