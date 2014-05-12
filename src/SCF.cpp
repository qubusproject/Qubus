#include <QBB/algorithms/SCF.hpp>
#include <QBB/utility/assert.hpp>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/SparseCore>

#include <cmath>
#include <tuple>

#include <iostream>
#include <fstream>

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

        F(i, l) += 0.5 * w_ijkl * D(j, k);
        F(i, k) += -0.5 * w_ijkl * D(j, l);
    }

    Eigen::MatrixXcd F_HF = b.adjoint() * F * b;

    auto E = F_HF.trace();

    return real(E);
}

std::tuple<double, double>
calculate_tims_quantities(const hartree_fock_state& state,
                          const sparse_tensor<std::complex<double>, 2>& h,
                          const sparse_tensor<std::complex<double>, 4>& w)
{
    auto hf_w = state.basis().single_particle_basis().transform_operator(w);
    auto hf_h = state.basis().single_particle_basis().transform_operator(h);

    auto M = state.basis().single_particle_basis().M();

    double sum_T4 = 0;
    for (index_t i = 0; i < M; ++i)
    {
        for (index_t j = i; j < M; ++j)
        {
            for (index_t k = 0; k < M; ++k)
            {
                for (index_t l = k; l < M; ++l)
                {
                    if (i != k && j != l)
                    {
                        //sum_T4 += std::abs(hf_w(i, k, j, l) - hf_w(i, l, j, k));
                        sum_T4 += std::abs(hf_w(i, j, l, k) - hf_w(i, j, k, l));
                    }
                }
            }
        }
    }

    double sum_T2 = 0;
    for (index_t i = 0; i < M; ++i)
    {
        for (index_t j = 0; j < M; ++j)
        {
            if (i != j)
            {
                auto temp = hf_h(i, j);
                for (index_t k = 0; k < M; ++k)
                {
                    if (k != i && k != j)
                    {
                        temp += hf_w(i, k, k, j) - hf_w(i, k, j, k);
                    }
                }
                sum_T2 += std::abs(temp);
            }
        }
    }
    
    return std::make_tuple(sum_T4, sum_T2);
}
}

hartree_fock_state calculate_SCF_solution(const sparse_tensor<std::complex<double>, 2>& h,
                                          const sparse_tensor<std::complex<double>, 4>& w,
                                          index_t N_alpha, index_t N_beta, double epsilon)
{
    // FIXME: add asserts for the tensor dimensions

    QBB_ASSERT(N_alpha == 0 || N_beta == 0, "calculate_SCF_solution: only spin-polarized HF is "
                                            "implemented (N_alpha == 0 || N_beta == 0)");

    if (N_beta != 0)
    {
        N_alpha = N_beta;
        N_beta = 0;
    }

    index_t Nb = h.shape()[0];

    Eigen::MatrixXcd hf_basis = Eigen::MatrixXcd::Random(Nb, Nb);

    {

    Eigen::MatrixXcd h_ideal = Eigen::MatrixXcd::Zero(Nb,Nb);

    for (auto nonzero_value : h.nonzeros())
    {
        const auto& indices = nonzero_value.indices();

        index_t i = indices[0];
        index_t l = indices[1];

        h_ideal(i,l) = nonzero_value.value();
    }

    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver(h_ideal);

    hf_basis = eigensolver.eigenvectors();

    }

    Eigen::MatrixXcd b = hf_basis.block(0, 0, Nb, N_alpha);

    double prev_energy;
    double current_energy = calculate_HF_energy(h, w, b);

    Eigen::MatrixXcd dD;
    Eigen::MatrixXcd prev_D = b * b.adjoint();

    std::ofstream fout("tims_quantities.dat");
    
    index_t iteration = 0;
    
    do
    {

        prev_energy = current_energy;

        std::cout << "current_energy: " << current_energy << std::endl;

        auto F = calculate_fock_operator(h, w, b);

        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver(F);

        hf_basis = eigensolver.eigenvectors();
        b = hf_basis.block(0, 0, Nb, N_alpha);

        current_energy = calculate_HF_energy(h, w, b);

        double sum_T4, sum_T2;
        
        std::tie(sum_T4, sum_T2) = calculate_tims_quantities(hartree_fock_state{hf_basis, N_alpha + N_beta},h,w);
        
        fout << iteration << "  " << sum_T4 << "  " << sum_T2 << "\n";
        
        Eigen::MatrixXcd D = b * b.adjoint();
        dD = D - prev_D;
        prev_D = D;
        
        ++iteration;

    } while (std::abs(current_energy - prev_energy) > epsilon || dD.lpNorm<2>() > epsilon || iteration < 100);

    return hartree_fock_state{hf_basis, N_alpha + N_beta};
}
}