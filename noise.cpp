#include <iostream>
#include "noise.hpp"
#include <time.h>
#include <cstdlib>

DataMat gen_3d_noise(int dim)
{
    int size = dim * dim * dim; 
    float* noise_arr = new float[size];
    for (int i = 0; i < size; i++)
    {
       noise_arr[i] = ((static_cast<float>(rand()) / RAND_MAX) * 2.0f) - 1.0f;
    
    }

    return DataMat(dim, noise_arr);
}