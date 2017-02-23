.. role:: cpp(code)
    :language: C++

************
Introduction
************

General program structure
=========================

To enable its extensive set of featues, code using Qubus has to be executed
in a special runtime environment.

This requirements leads to a common structure of Qubus-driven programs:

.. code-block:: C++

    int hpx_main(int argc, char** argv)
    {
        qubus::init(argc, argv);

        Your code using Qubus ...

        qubus::finalize();

        return hpx::finalize();
    }

    int main(int argc, char** argv)
    {
        return hpx::init(argc, argv);
    }

As usual, the program features the C++ entry point :cpp:`main`. Usually, the only thing the main function will
do is to initialize the HPX runtime

.. code-block:: C++

    return hpx::init(argc, argv);

and return the exit code from the program.

During the initialization process, the HPX runtime will start an HPX thread which will execute the
HPX entry point :cpp:`hpx_main`.

To setup the Qubus runtime environment, the program has to call the function :cpp:`qubus::init` before using any
other Qubus code. All components of Qubus, for example the initialization function,
can **only** be used within an HPX thread like the one executing :cpp:`hpx_main`
(unless specified otherwise).

After the runtime is successfully initialized, the actual program can be executed. Afterwards, both the
Qubus and the HPX runtime are shutdown using the :cpp:`qubus::finalize` and the :cpp:`hpx::finalize`
function, respectively. If some computations are pending upon termination, Qubus will finish these jobs
before shutting down the runtime.

While this is not the only valid structure of a program using Qubus, it is recommended that the presented one
should be used as a starting point. In principe, any of the startup methods provided by HPX can be used
to setup the necessary environment for Qubus. For further details, please consult the
`HPX documentation <http://stellar-group.github.io/hpx/docs/html/hpx.html>`_.

The first program - Vector addition
===================================

After we have delved the general structure of a Qubus-based program, it is now time
for our first program featuring *vector addition*.

.. code-block:: C++

    #include <qubus/qtl/all.hpp>
    #include <qubus/qubus.hpp>

    #include <hpx/hpx_init.hpp>

    #include <iostream>

    int hpx_main(int argc, char** argv)
    {
        using namespace qubus;

        qubus::init(argc, argv);

        constexpr long int N = 10;

        qtl::tensor<double, 1> A(N);
        qtl::tensor<double, 1> B(N);
        qtl::tensor<double, 1> C(N);

        {
            auto A_view = get_view<host_tensor_view<double, 1>>(A).get();

            for (long int i = 0; i < N; ++i)
            {
                A_view(i) = i;
            }

            auto B_view = get_view<host_tensor_view<double, 1>>(B).get();

            for (long int i = 0; i < N; ++i)
            {
                B_view(i) = 42 * i;
            }
        }

        static const qtl::tensor_expr<double, 1> C_def = [A, B](qtl::index i) {
            return A(i) + B(i);
        };

        C = C_def;

        {
            auto C_view = get_view<host_tensor_view<const double, 1>>(C).get();

            for (long int i = 0; i < N; ++i)
            {
                std::cout << C_view(i) << '\n';
            }
        }

        qubus::finalize();

        return hpx::finalize();
    }

    int main(int argc, char** argv)
    {
        return hpx::init(argc, argv);
    }

After the usual initialization, we create some vectors (tensors of order 1)

.. code-block:: C++

    qtl::tensor<double, 1> A(N);
    qtl::tensor<double, 1> B(N);
    qtl::tensor<double, 1> C(N);

with a given size :cpp:`N`.

.. code-block:: C++

    auto A_view = get_view<host_tensor_view<double, 1>>(A).get();

will request a view on the specified object :cpp:`A`, in this case a :cpp:`host_tensor_view` for a
double-precision tensor of order 1. Views are used in Qubus to obtain direct access to
the in-memory representation of an object. In this case, the memory of the host process, alias the main memory,
is referenced. There are views for each type of object in Qubus and most forms of memory.

After one has obtained a view, one can use it to interact with the object within normal C++ code. In this case,
we initialize the tensor with some values. Usually, any view will try to mimick the behaviour of
an equivalent C++ object. For example, :`host_tensor_view` will provide all operations one would expect from
an array.

After the view has been used, it should be destoryed as early as possible. During the existance of
the view any other access of the object will be blocked by the runtime. Therefore, subsequent operations
on that object will not proceed, which might hamper an efficient execution, until one destorys the view.
Therefore, one should usually wrap the code using the view into a scope or restrict the lifetime
of the view otherwise.

.. code-block:: C++

        static const qtl::tensor_expr<double, 1> C_def = [A, B](qtl::index i) {
            return A(i) + B(i);
        };

does specify the calculation which we want to perform and stores it in a tensor expression object.
It should be noted that this only **defines** the operation but does not execute it yet.

in Qubus, a tensor expression is a first-class object which can store a definition of a tensor.
Since it is a first-class object it can be passed around the program or stored for eventual execution.
In this case, we store the tensor expression in a local static variable. This has the advantage
that the tensor expression can be easily used within that function but is only initialized once.
Since the creating and destruction of a tensor expression can be extremely costly, it should be usualy
avoided.

To initialize the tensor expression, one can use, in principle, any function with a certain structure.
As parameters, the funciton should take the indices, represented by :cpp:`index` objects,
of the defined tensor which can be used later within the function to construct the actual definition
and should return an expression defining the calculation which is used to form the new tensor.
Since the calculation depends on the tensors :cpp:`A` and :cpp:`B`, we specify these in the capture list
of the lambda to be able to use them in the definition. Since the objects corresponding to :cpp:`A` and :cpp:`B`
can not be changed afterwards, we refer to these dependencies as *static* dependencies. Later on, we will learn
how to specify *dynamic* depedencies to change the used objects between calculations.

To actually execute the calculation, one can simply assign the definition to a suitable tensor object:

.. code-block:: C++

    C = C_def;

Afterwards, we finish the program by writing the result to the standard output using a view.
It should be noted that a read-only view is requested by prefixing the value type of the view with :cpp:`const`.
If possible, read-only access should be prefered as it allows for a more optimized exection of the code.

.. TODO: We need to write these guides first.
.. To learn how to build and execute the program, please refer to the Build Guide and the Execution Guide,
respectively.