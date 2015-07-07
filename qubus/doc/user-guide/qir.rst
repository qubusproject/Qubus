*****************
QIR specification
*****************

Type system
===========

Primitive data types
--------------------

Floating point types
^^^^^^^^^^^^^^^^^^^^

Integral types
^^^^^^^^^^^^^^

Compound types
--------------

Tensor types
------------


Expressions
===========

for expression
--------------

**Syntax:**
::

    for i in [a, b]
    {
        loop-body
    }

**Semantics:**

Iterate over all integers in the half-open interval [a, b[. A new integer variable i is introduced as a loop counter, which is only accessible from within the loop body.
All values of i are traversed in ascending order.

for all expression
------------------

**Syntax:**
::

    for all i
    {
        loop-body
    }

**Semantics:**

Introduce a new abstract index i.

subscription expression
-----------------------

**Syntax:**
::

    A[i, j, ...]

**Semantics:**

Access the element (i, j, ...) of the tensor A.

Examples
========

Matrix-Matrix multiplication
----------------------------
::

    def mat_mul(tensor!(double) A, tensor!(double) B) -> tensor!(double) C
    {
        for all i
        {
            for all j
            {
                for all k
                {
                    C[i, j] += A[i,k] * B[k, j];
                }
            }
        }
    }
