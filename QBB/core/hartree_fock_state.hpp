#ifndef QBB_HARTREE_FOCK_STATE_HPP
#define QBB_HARTREE_FOCK_STATE_HPP

#include <QBB/support/integers.hpp>
#include <QBB/support/sparse_tensor.hpp>
#include <QBB/support/tensor.hpp>

#include <Eigen/Core>

namespace qbb
{

class transforming_basis
{
public:
    explicit transforming_basis(Eigen::MatrixXcd b_) : b_{b_}
    {
    }

    index_t Nb() const
    {
        return b_.rows();
    }

    index_t M() const
    {
        return b_.cols();
    }

    const Eigen::MatrixXcd& transformation_matrix() const
    {
        return b_;
    }

    tensor<std::complex<double>, 2>
    transform_operator(const sparse_tensor<std::complex<double>, 2>& op) const;
    tensor<std::complex<double>, 4>
    transform_operator(const sparse_tensor<std::complex<double>, 4>& op) const;
    tensor<std::complex<double>, 2>
    concatenate_transforms(const tensor<std::complex<double>, 2>& op) const;
    tensor<std::complex<double>, 2>
    concatenate_backtransforms(const tensor<std::complex<double>, 2>& op) const;
private:
    Eigen::MatrixXcd b_;
};

class spin_restricted_hartree_fock_many_particle_basis
{
public:
    explicit spin_restricted_hartree_fock_many_particle_basis(Eigen::MatrixXcd b_, index_t N_)
    : single_particle_basis_{b_}, N_{N_}
    {
    }

    index_t N() const
    {
        return N_;
    }

    const transforming_basis& single_particle_basis() const
    {
        return single_particle_basis_;
    }

private:
    transforming_basis single_particle_basis_;
    index_t N_;
};

class hartree_fock_state
{
public:
    explicit hartree_fock_state(Eigen::MatrixXcd b_, index_t N_);

    const spin_restricted_hartree_fock_many_particle_basis& basis() const
    {
        return hf_basis_;
    }

private:
    spin_restricted_hartree_fock_many_particle_basis hf_basis_;
};
}

#endif