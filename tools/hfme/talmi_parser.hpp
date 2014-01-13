#ifndef QBB_TALMI_PARSER_HPP
#define QBB_TALMI_PARSER_HPP

#include <QBB/support/sparse_tensor.hpp>
#include <QBB/support/tensor.hpp>

#include <complex>
#include <string>
#include <vector>

namespace qbb
{
    
class talmi_matrix_elements_loader
{
public:
    explicit talmi_matrix_elements_loader(const std::string& path, double lambda);
    
    const sparse_tensor<std::complex<double>,2>& get_one_particle_hamiltonian() const;
    const sparse_tensor<std::complex<double>,4>& get_two_particle_interaction_operator() const;
private:
    sparse_tensor<std::complex<double>,2> one_particle_hamiltonian_;
    sparse_tensor<std::complex<double>,4> two_particle_interaction_operator_;
};

class talmi_spatial_representation_loader
{
public:
    explicit talmi_spatial_representation_loader(const std::string& path);
    
    const std::vector<double>& get_spatial_coordinates() const;
    const tensor<std::complex<double>,2>& get_transformation_matrix() const;
private:
    std::vector<double> spatial_coordinates_;
    tensor<std::complex<double>,2> transformation_matrix_;
};
    
}

#endif