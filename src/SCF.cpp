#include <QBB/algorithms/SCF.hpp>
#include <QBB/utility/assert.hpp>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/SparseCore>

#include <cmath>

#include <iostream>

namespace qbb
{

namespace
{

Eigen::MatrixXcd calculate_fock_operator(const sparse_tensor<std::complex<double>, 2>& h,
                                         const sparse_tensor<std::complex<double>, 4>& w,
                                         const Eigen::MatrixXcd& b)
{
    Eigen::MatrixXcd F = Eigen::MatrixXcd::Zero(h.shape()[0], h.shape()[1]);

    for (auto nonzero_value : h.nonzeros())
    {
        const auto& indices = nonzero_value.indices();

        index_t i = indices[0];
        index_t l = indices[1];

        F(i, l) = nonzero_value.value();
    }

    Eigen::MatrixXcd D = b * b.adjoint();

    for (auto nonzero_value : w.nonzeros())
    {
        const auto& indices = nonzero_value.indices();
        auto w_ijkl = nonzero_value.value();

        index_t i = indices[0];
        index_t j = indices[1];
        index_t k = indices[2];
        index_t l = indices[3];

        F(i, l) += w_ijkl * D(j, k);
        F(i, k) += -w_ijkl * D(j, l);
    }

    return F;
}

double calculate_HF_energy(const sparse_tensor<std::complex<double>, 2>& h,
                           const sparse_tensor<std::complex<double>, 4>& w,
                           const Eigen::MatrixXcd& b)
{
    auto F = calculate_fock_operator(h, w, b);

    Eigen::MatrixXcd F_HF = b.adjoint() * F * b;

    auto E = F_HF.trace();

    return real(E);
}
}

hartree_fock_state calculate_SCF_solution(const sparse_tensor<std::complex<double>, 2>& h,
                                          const sparse_tensor<std::complex<double>, 4>& w,
                                          index_t N_alpha, index_t N_beta, double epsilon)
{
    // FIXME: add asserts for the tensor dimensions

    QBB_ASSERT(N_alpha == 0 || N_beta == 0, "calculate_SCF_solution: only spin-polarized HF is "
                                            "implemented (N_alpha == 0 || N_beta == 0)");

    if(N_beta != 0)
    {
        N_alpha = N_beta;
        N_beta = 0;
    }
    
    index_t Nb = h.shape()[0];

    Eigen::MatrixXcd hf_basis = Eigen::MatrixXcd::Random(Nb, Nb);

    for (index_t i = 0; i < Nb; ++i)
    {
        hf_basis.col(i).normalize();
    }
    
    //FIXME: should we orthonormalize the basis here??
    
    Eigen::MatrixXcd b = hf_basis.block(0,0,Nb,N_alpha);

    double prev_energy;
    double current_energy = calculate_HF_energy(h, w, b);

    do
    {

        prev_energy = current_energy;

        std::cout << current_energy << std::endl;

        auto F = calculate_fock_operator(h, w, b);

        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver(F);

        hf_basis = eigensolver.eigenvectors();
        b = hf_basis.block(0, 0, Nb, N_alpha);

        current_energy = calculate_HF_energy(h, w, b);

    } while (std::abs(current_energy - prev_energy) > epsilon);

    return hartree_fock_state{hf_basis, N_alpha + N_beta};
}
}