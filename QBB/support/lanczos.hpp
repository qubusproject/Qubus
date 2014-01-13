#ifndef QBB_LANCZOS_HPP
#define QBB_LANCZOS_HPP

#include <QBB/support/integers.hpp>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <vector>
#include <cassert>

namespace qbb
{

template <typename Matrix>
class LanczosEigensolver
{
public:
    LanczosEigensolver(const Matrix& A, index_t L)
    {
        compute(A, L);
    }

    const Eigen::VectorXd& eigenvalues() const
    {
        return eigenvalues_;
    }

    const Eigen::MatrixXcd& eigenvectors() const
    {
        return eigenvectors_;
    }

private:
    void compute(const Matrix& A, int L)
    {
        assert(A.rows() == A.cols());

        Eigen::VectorXcd alpha(L);
        Eigen::VectorXcd beta(L);

        beta(0) = 0;

        Eigen::MatrixXcd V(A.rows(), L);

        V.col(0) = Eigen::VectorXcd::Random(A.rows());
        V.col(0).normalize();

        Eigen::VectorXcd w(A.rows());

        for (index_t j = 0; j < L - 1; ++j)
        {
            w = A * V.col(j);

            alpha(j) = w.dot(V.col(j));

            w -= alpha(j) * V.col(j);

            if (j > 0)
                w -= beta(j) * V.col(j - 1);

            Eigen::VectorXcd w_prime(w);
            for (index_t k = 0; k <= j; ++k)
            {
                w_prime -= (V.col(k).dot(w)) * V.col(k);
            }
            w = w_prime;

            beta(j + 1) = w.norm();

            V.col(j + 1) = w / beta(j + 1);
        }

        w = A * V.col(L - 1);
        alpha(L - 1) = w.dot(V.col(L - 1));

        Eigen::MatrixXcd T = Eigen::MatrixXcd::Zero(L, L);

        T.diagonal(0) = alpha;
        T.diagonal(-1) = beta.segment(1, L - 1);
        T.diagonal(1) = beta.segment(1, L - 1);

        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> eigensolver(T);

        eigenvalues_ = eigensolver.eigenvalues();

        eigenvectors_ = V * eigensolver.eigenvectors();
    }

    Eigen::MatrixXcd eigenvectors_;
    Eigen::VectorXd eigenvalues_;
};

}

#endif
