Qubus
=====

A parallel virtual machine for heterogeneous and distributed architectures.

Goals
-----

Qubus aims to provide a unified platform to program highly parallel
systems from single cores to an entire cluster of heterogeneous nodes
while still maintaining support for legacy codes.

At its core, Qubus implements a virtual machine natively supporting parallelism
and concurrency, abstracting away the differences between different hardware
architectures as much as possible. The VM is programmed using Qubus' own
intermediate language QIR, which can be written manually by a user or
generated from a domain-specific problem description.