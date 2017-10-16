#pragma once

#include <Eventlet.h>

namespace NMX
{

class EventletPacket
{
  public:
    EventletPacket() {}
    EventletPacket(size_t size);
    void add(const Eventlet& e);
    void clear_and_keep_capacity();

    std::vector<Eventlet> eventlets;
    uint64_t time_start, time_end;

    std::vector<uint32_t> to_h5() const;
    void to_h5(std::vector<uint32_t>& packet) const;
    void from_h5(const std::vector<uint32_t>& packet);

    struct CompareStart
    {
        bool operator()(const EventletPacket &a, const EventletPacket &b);
    };

    struct CompareEnd
    {
        bool operator()(const EventletPacket &a, const EventletPacket &b);
    };

};

}
