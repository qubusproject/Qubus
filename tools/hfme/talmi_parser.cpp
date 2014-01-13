#include "talmi_parser.hpp"

namespace qbb
{

talmi_matrix_elements_loader::talmi_matrix_elements_loader(const std::string& path, double lambda)
{
    std::ifstream fin(path);

    if (!fin.is_open())
    {
        throw std::runtime_error("unable to open talmi file " + path);
    }

    std::string garbage;
    std::getline(fin, garbage);

    assert(garbage == "one electron integrals:");

    index_t Nb;

    {
        index_t index;
        double value;

        std::vector<qbb::indices_value_pair<std::complex<double>, 2>> nonzeros;

        while (fin >> index >> value)
        {
            if (value != 0.0)
            {
                nonzeros.push_back(
                    qbb::indices_value_pair<std::complex<double>, 2>({index, index}, value));
            }
        }

        Nb = qbb::to_uindex(nonzeros.size());

        one_particle_hamiltonian_ = sparse_tensor<std::complex<double>, 2>(Nb, Nb);

        one_particle_hamiltonian_.set(std::move(nonzeros));

        fin.clear();
    }

    std::getline(fin, garbage);

    assert(garbage == "two electron integrals:");

    {
        index_t counter = 0;

        double value;

        std::vector<qbb::indices_value_pair<std::complex<double>, 4>> nonzeros;

        while (fin >> value)
        {

            // counter = ((i * Nb + j) * Nb + k) * Nb + l
            index_t l = counter % Nb;
            index_t ijk = counter / Nb;
            index_t k = ijk % Nb;
            index_t ij = ijk / Nb;
            index_t j = ij % Nb;
            index_t i = ij / Nb;

            if (value != 0.0)
            {
                nonzeros.push_back(
                    qbb::indices_value_pair<std::complex<double>, 4>({i, j, k, l}, lambda * value));
            }

            ++counter;
        }

        two_particle_interaction_operator_ = sparse_tensor<std::complex<double>, 4>(Nb, Nb, Nb, Nb);

        two_particle_interaction_operator_.set(std::move(nonzeros));
    }
}

const sparse_tensor<std::complex<double>, 2>& talmi_matrix_elements_loader::get_one_particle_hamiltonian() const
{
    return one_particle_hamiltonian_;
}

const sparse_tensor<std::complex<double>, 4>&
talmi_matrix_elements_loader::get_two_particle_interaction_operator() const
{
    return two_particle_interaction_operator_;
}
}