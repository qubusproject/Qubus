#include <QBB/algorithms/SCF.hpp>

#include "talmi_parser.hpp"

#include <echelon/echelon.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <iostream>

int main(int argc, char** argv)
{   
    std::string input_file;
    std::string output_file;
    std::string spatial_repr_file;
    qbb::index_t N_alpha;
    qbb::index_t N_beta;
    double lambda;
    
    namespace bpo = boost::program_options;
    
    bpo::options_description generic("Generic options");
    generic.add_options()
        ("version", "print version string")
        ("help", "produce help message")
        ("input-file,i", bpo::value<std::string>(&input_file), "path of the input file")
        ("output-file,o", bpo::value<std::string>(&output_file), "path of the output file")
        ("spatial-repr-file", bpo::value<std::string>(&spatial_repr_file), "path to the spatial representation file")
        ("N_alpha", bpo::value<qbb::index_t>(&N_alpha), "number of spin-up particles")
        ("N_beta", bpo::value<qbb::index_t>(&N_beta)->default_value(0), "number of spin-down particles")
        ("lambda", bpo::value<double>(&lambda)->default_value(1.0), "interaction strenght");

    bpo::positional_options_description p;
    p.add("input-file", 1);

    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(argc, argv)
                       .options(generic)
                       .positional(p)
                       .run(),
                   vm);
    bpo::notify(vm); 
    
    if (vm.count("help"))
    {
        std::cout << generic << "\n";
        return 0;
    }

    if (vm.count("version"))
    {
        std::cout << "hfme\n"
                  << "version 0.1" << '\n';
        return 0;
    }
    
    boost::filesystem::path input_path(input_file);
    if (!exists(input_path))
    {
        std::cout << "error: unable to open input file " << input_file << std::endl; 
        return -1;
    }
    
    if(vm.count("N_alpha") == 0)
    {
        std::cout << "error: no number of particles specified" << std::endl;
        
        return -1; 
    }
    
    if(N_alpha < 0 || N_beta < 0)
    {
        std::cout << "error: negative number of particles" << std::endl;
        
        return -1;
    }
    
    if (N_alpha != 0 && N_beta != 0)
    {
        std::cout << "error: only spin-polarized HF is implemented by now (N_alpha == 0 || N_beta == 0)" << std::endl;
        
        return -1;
    }
    
    qbb::index_t N = N_alpha + N_beta;
    
    std::cout << "number of particles = " << N << std::endl;
    std::cout << "lambda = " << lambda << std::endl;
        
    std::cout << "parsing the talmi input file..." << std::endl;
    qbb::talmi_matrix_elements_loader loader(input_file, 1.0);

    std::cout << "solving the self-consistent field equation..." << std::endl;
    auto hf_state =
        calculate_SCF_solution(loader.get_one_particle_hamiltonian(),
                               loader.get_two_particle_interaction_operator(), N, 0, 1e-10);
       
    const auto& hf_basis = hf_state.basis().single_particle_basis();    
        
    std::cout << "transforming the matrix elements..." << std::endl;

    std::cout << "\ttransforming the one-particle hamiltonian" << std::flush;
    
    auto hf_one_particle_hamiltonian = hf_basis.transform_operator(loader.get_one_particle_hamiltonian());
    
    std::cout << " - done" << std::endl;
    
    std::cout << "\ttransforming the interaction operator" << std::flush;
    
    auto hf_two_particle_interaction_operator =
        hf_basis.transform_operator(
            loader.get_two_particle_interaction_operator());
        
    std::cout << " - done" << std::endl;
        
    echelon::file fout(output_file, echelon::file::create_mode::truncate);
    fout.attributes.create<std::string>("version", "1.0");

    std::size_t Nb = hf_one_particle_hamiltonian.shape()[0];

    auto h_dataset = fout.create_dataset<std::complex<double>>("h", {Nb, Nb});
    h_dataset <<= hf_one_particle_hamiltonian;

    auto w_dataset = fout.create_dataset<std::complex<double>>("w", {Nb, Nb, Nb, Nb});
    w_dataset <<= hf_two_particle_interaction_operator;
    
    if (!spatial_repr_file.empty())
    {
        std::cout << "loading the talmi spatial representation..." << std::endl;
        
        qbb::talmi_spatial_representation_loader sp_loader(spatial_repr_file);
        
        std::size_t Nb_funcs = sp_loader.get_transformation_matrix().shape()[0];

        if(Nb != Nb_funcs)
        {
            std::cout << "error: number of basis function does not match the number of given spatial representations" << std::endl;
            return -1;
        }

        std::cout << "calculating the HF-spatial transformation matrix..." << std::endl;
        
        auto hf_spatial_transform = hf_basis.concatenate_backtransforms(sp_loader.get_transformation_matrix());
        
        qbb::index_t Nx = hf_spatial_transform.shape()[1];
        
        auto hf_sp_trans_dataset = fout.create_dataset<std::complex<double>>("hf_spatial_transform", {Nb, Nx});
        hf_sp_trans_dataset <<= hf_spatial_transform;
        
        hf_sp_trans_dataset.dimensions[1].relabel("x");
        auto sp_coords_scale = hf_sp_trans_dataset.dimensions[1].attach_dimension_scale<double>("x");

        sp_coords_scale <<= sp_loader.get_spatial_coordinates();
    }
    else
    {
        std::cout << "no spatial representation file given, skipping calculation" << std::endl;
    }
}