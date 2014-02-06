% USE_INV
% Demontrates the use of the matrix inversion function
function [a,b,sigma_a,sigma_b]=use_inv (alpha, beta, lambda, gamma, rho)

W = [[ alpha, beta ] ; [beta, lambda ]] % this is the uninverted matrix

M = inv(W)

M * W				% believe no one
W * M				% ... ever!

ab = M * [ gamma ; rho ]

a = ab(1,1)
b = ab(2,1)

sigma_a = M(1,1)
sigma_b = M(2,2)
