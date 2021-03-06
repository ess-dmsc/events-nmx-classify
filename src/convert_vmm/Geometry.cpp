/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Geometry.h>

void Geometry::define_plane(
    uint16_t planeID,
    std::initializer_list<std::pair<uint16_t, uint16_t>> chips) {
  int offset = 0;
  for (auto c : chips) {
    set_mapping(c.first, c.second, planeID, offset);
    offset += EVENTLET_TOTAL_CHANNELS;
  }
}

void Geometry::set_mapping(uint16_t fecID, uint16_t vmmID, uint16_t planeID,
                           uint16_t strip_offset) {
  if (vmmID > 15)
    return;

  if (offsets_.size() <= fecID) {
    for (int i = offsets_.size(); i <= fecID; ++i) {
      offsets_.resize(i + 1);
      offsets_[i] = std::vector<uint16_t>(EVENTLET_MAX_CHIPS, EVENTLET_INVALID_ID);
      planes_.resize(i + 1);
      planes_[i] = std::vector<uint16_t>(EVENTLET_MAX_CHIPS, EVENTLET_INVALID_ID);
    }
  }
  offsets_[fecID][vmmID] = strip_offset;
  planes_[fecID][vmmID] = planeID;
}

uint16_t Geometry::get_strip_ID(uint16_t fecID, uint16_t vmmID,
                                uint32_t channelID) const {
  if ((fecID < offsets_.size()) && (vmmID < offsets_.at(fecID).size()) &&
      (offsets_.at(fecID).at(vmmID) != EVENTLET_INVALID_ID))
    return offsets_.at(fecID).at(vmmID) + channelID;
  else
    return EVENTLET_INVALID_ID;
}

uint16_t Geometry::get_plane_ID(uint16_t fecID, uint16_t vmmID) const {
  if ((fecID < planes_.size()) && (vmmID < planes_.at(fecID).size()) &&
      (planes_.at(fecID).at(vmmID) != EVENTLET_INVALID_ID))
    return planes_.at(fecID).at(vmmID);
  else
    return EVENTLET_INVALID_ID;
}
