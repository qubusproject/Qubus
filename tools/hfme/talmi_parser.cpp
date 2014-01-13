#include "talmi_parser.hpp"

#include <fstream>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <limits>

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

const sparse_tensor<std::complex<double>, 2>&
talmi_matrix_elements_loader::get_one_particle_hamiltonian() const
{
    return one_particle_hamiltonian_;
}

const sparse_tensor<std::complex<double>, 4>&
talmi_matrix_elements_loader::get_two_particle_interaction_operator() const
{
    return two_particle_interaction_operator_;
}

talmi_spatial_representation_loader::talmi_spatial_representation_loader(const std::string& path)
{
    std::ifstream fin(path);

    if (!fin.is_open())
    {
        throw std::runtime_error("unable to open talmi spatial representation file " + path);
    }

    double x;
    double coeff;

    double prev_x = std::numeric_limits<double>::min();

    std::vector<std::vector<double>> func_coeffs;
    std::vector<double> current_function;

    bool collect_sp_coordinates = true;

    while (fin >> x >> coeff)
    {
        if (prev_x > x)
        {
            func_coeffs.push_back(std::move(current_function));
            current_function = std::vector<double>();
            collect_sp_coordinates = false;
            prev_x = std::numeric_limits<double>::min();
        }

        if (collect_sp_coordinates)
        {
            spatial_coordinates_.push_back(x);
        }

        current_function.push_back(coeff);

        prev_x = x;
    }

    index_t Nb = func_coeffs.size();

    if (Nb == 0)
    {
        throw std::runtime_error("no function coefficients loaded??");
    }

    index_t Nx = func_coeffs.at(0).size();

    transformation_matrix_ = tensor<std::complex<double>, 2>(Nb, Nx);

    for (index_t k = 0; k < Nb; ++k)
    {
        for (index_t r = 0; r < Nx; ++r)
        {
            transformation_matrix_(k, r) = func_coeffs[k][r];
        }
    }

    assert(spatial_coordinates_.size() == Nx);
}

const std::vector<double>& talmi_spatial_representation_loader::get_spatial_coordinates() const
{
    return spatial_coordinates_;
}

const tensor<std::complex<double>, 2>& talmi_spatial_representation_loader::get_transformation_matrix()
    const
{
    return transformation_matrix_;
}
}