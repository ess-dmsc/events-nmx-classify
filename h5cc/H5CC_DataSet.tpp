namespace H5CC {

template<typename T>
void DataSet::write(const std::vector<T>& data, H5::PredType h5type)
{
  if (data.size() != space_.data_size())
    return;
  try
  {
    Location<H5::DataSet>::location_.write(data.data(), h5type, space_.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::read(std::vector<T>& data, H5::PredType h5type) const
{
  data.resize(space_.data_size());
  try
  {
    Location<H5::DataSet>::location_.read(data.data(), h5type, space_.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::read(std::vector<T>& data, H5::PredType h5type, Space slab, std::initializer_list<hsize_t> index) const
{
  data.resize(slab.data_size());
  auto space = space_;
  if (!space.select_slab(slab, index))
    return;
  try
  {
    Location<H5::DataSet>::location_.read(data.data(), h5type, slab.space(), space.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::write(const std::vector<T>& data, H5::PredType h5type, Space slab, std::initializer_list<hsize_t> index)
{
  auto space = space_;
  if (!space.select_slab(slab, index))
    return;
  try
  {
    Location<H5::DataSet>::location_.write(data.data(), h5type, space_.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::read(std::vector<T>& data, H5::PredType h5type,
                std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index) const
{
  read(data, h5type, space_.slab_space(slab_size), index);
}

template<typename T>
void DataSet::write(const std::vector<T>& data, H5::PredType h5type,
                    std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index)
{
  write(data, h5type, space_.slab_space(slab_size), index);
}


}
