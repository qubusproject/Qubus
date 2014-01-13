#include <QBB/core/hartree_fock_state.hpp>

#include <utility>

namespace qbb
{

tensor<std::complex<double>, 2>
transforming_basis::transform_operator(const sparse_tensor<std::complex<double>, 2>& op) const
{
    const auto& b = transformation_matrix();

    index_t Nb = op.shape()[0];

    tensor<std::complex<double>, 2> hf_op(Nb, Nb);

    for (auto nonzero_value : op.nonzeros())
    {
        auto op_ij = nonzero_value.value();

        index_t i = nonzero_value.indices()[0];
        index_t j = nonzero_value.indices()[1];

        for (index_t m = 0; m < Nb; ++m)
        {
            for (index_t n = 0; n < Nb; ++n)
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

    index_t Nb = op.shape()[0];

    tensor<std::complex<double>, 4> hf_op(Nb, Nb, Nb, Nb);

    for (auto nonzero_value : op.nonzeros())
    {
        auto op_ijkl = nonzero_value.value();

        index_t i = nonzero_value.indices()[0];
        index_t j = nonzero_value.indices()[1];
        index_t k = nonzero_value.indices()[2];
        index_t l = nonzero_value.indices()[3];

        for (index_t m = 0; m < Nb; ++m)
        {
            for (index_t n = 0; n < Nb; ++n)
            {
                for (index_t p = 0; p < Nb; ++p)
                {
                    for (index_t q = 0; q < Nb; ++q)
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

hartree_fock_state::hartree_fock_state(Eigen::MatrixXcd b_, index_t N_)
: hf_basis_{std::move(b_), N_}
{
}
}