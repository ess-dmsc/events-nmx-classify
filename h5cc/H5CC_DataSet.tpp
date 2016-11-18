namespace H5CC {

template<typename T>
void DataSet::read(std::vector<T>& data, Space slab, std::initializer_list<hsize_t> index, H5::DataType h5type) const
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
void DataSet::write(const std::vector<T>& data, Space slab, std::initializer_list<hsize_t> index, H5::DataType h5type)
{
  auto space = space_;
  if (!space.select_slab(slab, index))
    return;
  try
  {
    Location<H5::DataSet>::location_.write(data.data(), h5type, slab.space(), space.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::write(const T& data,
                    std::initializer_list<hsize_t> index,
                    H5::DataType h5type)
{
  auto space = space_;
  if (!space.select_element(index))
    return;
  try
  {
    Space dspace({1});
    Location<H5::DataSet>::location_.write(&data, h5type, dspace.space(), space.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::write(const std::vector<T>& data, H5::DataType h5type)
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
void DataSet::read(T& data,
                   std::initializer_list<hsize_t> index,
                   H5::DataType h5type) const
{
  auto space = space_;
  if (!space.select_element(index))
    return;
  try
  {
    Space dspace({1});
    Location<H5::DataSet>::location_.read(&data, h5type, dspace.space(), space.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::read(std::vector<T>& data, H5::DataType h5type) const
{
  data.resize(space_.data_size());
  try
  {
    Location<H5::DataSet>::location_.read(data.data(), h5type, space_.space());
  }
  catch (...) {}
}

template<typename T>
void DataSet::read(std::vector<T>& data,
                   std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index,
                   H5::DataType h5type) const
{
  read(data, space_.slab_space(slab_size), index, h5type);
}

template<typename T>
void DataSet::write(const std::vector<T>& data,
                    std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index,
                    H5::DataType h5type)
{
  write(data, space_.slab_space(slab_size), index, h5type);
}

template<typename T>
void DataSet::write(const T& data,
                    std::initializer_list<hsize_t> index)
{
  write(data, index, type_);
}

template<typename T>
void DataSet::write(const std::vector<T>& data)
{
  write(data, type_);
}

template<typename T>
void DataSet::read(T& data,
                   std::initializer_list<hsize_t> index) const
{
  read(data, index, type_);
}

template<typename T>
void DataSet::read(std::vector<T>& data) const
{
  read(data, type_);
}

template<typename T>
void DataSet::read(std::vector<T>& data,
                   std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index) const
{
  read(data, slab_size, index, type_);
}

template<typename T>
void DataSet::write(const std::vector<T>& data,
                    std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index)
{
  write(data, slab_size, index, type_);
}


}
