#ifndef QBB_TALMI_PARSER_HPP
#define QBB_TALMI_PARSER_HPP

#include <QBB/support/sparse_tensor.hpp>

#include <complex>
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <cassert>
#include <stdexcept>    

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
    
}

#endif