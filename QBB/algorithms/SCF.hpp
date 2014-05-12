#ifndef QBB_SCF_HPP
#define QBB_SCF_HPP

#include <QBB/support/sparse_tensor.hpp>
#include <QBB/core/hartree_fock_state.hpp>
#include <QBB/support/integers.hpp>

#include <complex>
#include <functional>

namespace qbb
{

Eigen::MatrixXcd calculate_ideal_states(const sparse_tensor<std::complex<double>, 2>& h);
    
hartree_fock_state
calculate_SCF_solution(const sparse_tensor<std::complex<double>, 2>& h,
                       const sparse_tensor<std::complex<double>, 4>& w,
                       Eigen::MatrixXcd initial_basis, index_t N_alpha, index_t N_beta,
                       std::function<bool(const Eigen::MatrixXcd&, double)> break_condition);

hartree_fock_state calculate_SCF_solution(const sparse_tensor<std::complex<double>, 2>& h,
                                          const sparse_tensor<std::complex<double>, 4>& w,
                                          index_t N_alpha, index_t N_beta, double epsilon);
}

#endif