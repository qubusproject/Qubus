#include <QBB/core/hartree_fock_state.hpp>

#include <utility>

namespace qbb
{

tensor<std::complex<double>, 2>
transforming_basis::transform_operator(const sparse_tensor<std::complex<double>, 2>& op) const
{
    const auto& b = transformation_matrix();

    index_t M = this->M();

    tensor<std::complex<double>, 2> hf_op(M, M);

    for (auto nonzero_value : op.nonzeros())
    {
        auto op_ij = nonzero_value.value();

        index_t i = nonzero_value.indices()[0];
        index_t j = nonzero_value.indices()[1];

        for (index_t m = 0; m < M; ++m)
        {
            for (index_t n = 0; n < M; ++n)
            {
                hf_op(m, n) += conj(b(i, m)) * op_ij * b(j, n);
            }
        }
    }

    return hf_op;
}

tensor<std::complex<double>, 4>
transforming_basis::transform_operator(const sparse_tensor<std::complex<double>, 4>& op) const
{
    const auto& b = transformation_matrix();

    index_t M = this->M();

    tensor<std::complex<double>, 4> hf_op(M, M, M, M);

    for (auto nonzero_value : op.nonzeros())
    {
        auto op_ijkl = nonzero_value.value();

        index_t i = nonzero_value.indices()[0];
        index_t j = nonzero_value.indices()[1];
        index_t k = nonzero_value.indices()[2];
        index_t l = nonzero_value.indices()[3];

        for (index_t m = 0; m < M; ++m)
        {
            for (index_t n = 0; n < M; ++n)
            {
                for (index_t p = 0; p < M; ++p)
                {
                    for (index_t q = 0; q < M; ++q)
                    {
                        hf_op(m, n, p, q) +=
                            conj(b(i, m)) * conj(b(j, n)) * op_ijkl * b(k, p) * b(l, q);
                    }
                }
            }
        }
    }
    
    return hf_op;
}

tensor<std::complex<double>, 2>
transforming_basis::concatenate_transforms(const tensor<std::complex<double>, 2>& op) const
{
    const auto& b = transformation_matrix();

    index_t Nb = op.shape()[0];
    index_t Nb2 = op.shape()[1];
    index_t M = this->M();

    assert(Nb2 == this->Nb());

    tensor<std::complex<double>, 2> hf_op(Nb, M);

    for (index_t i = 0; i < Nb; ++i)
    {
        for (index_t m = 0; m < M; ++m)
        {
            for (index_t j = 0; j < Nb2; ++j)
            {
                hf_op(i, m) += op(i, j) * b(j, m);
            }
        }
    }

    return hf_op;
}

tensor<std::complex<double>, 2>
transforming_basis::concatenate_backtransforms(const tensor<std::complex<double>, 2>& op) const
{
    const auto& b = transformation_matrix();

    index_t Nb = op.shape()[0];
    index_t Nb2 = op.shape()[1];
    index_t M = this->M();

    assert(Nb2 == this->Nb());

    tensor<std::complex<double>, 2> hf_op(M,Nb2);

    for (index_t i = 0; i < Nb2; ++i)
    {
        for (index_t m = 0; m < M; ++m)
        {
            for (index_t j = 0; j < Nb; ++j)
            {
                hf_op(m, i) += conj(b(j, m)) * op(j, i);
            }
        }
    }

    return hf_op;
}

hartree_fock_state::hartree_fock_state(Eigen::MatrixXcd b_, index_t N_)
: hf_basis_{std::move(b_), N_}
{
}
}