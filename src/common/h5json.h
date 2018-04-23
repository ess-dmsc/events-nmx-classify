#pragma once

#include <nlohmann/json.hpp>
#include <h5cpp/hdf5.hpp>

using json = nlohmann::json;

namespace hdf5
{

void to_json(json& j, const node::Group& g);
void from_json(const json& j, node::Group& g);

//void to_json(json& j, const Enum<int16_t>& e);
//void from_json(const json& j, Enum<int16_t>& e);

void attribute_to_json(json& parent, const attribute::Attribute& g);
void attribute_from_json(const json& j, const std::string& name, const node::Node& g);

void dataset_to_json(json& j, const node::Dataset& d);
void dataset_from_json(const json& j, const std::string& name, const node::Group& g);

}

std::string vector_idx_minlen(size_t idx, size_t max);
