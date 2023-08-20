#include <glm/glm.hpp>
#include <cmath>
#include <cstdlib>

class DataMat
{
public:
    DataMat(int dim, float* data): dim(dim), dim2(dim * dim), size(dim * dim2), data(data) {}

    float get(int i, int j, int k) const
    {
        // Wrap the noise
        i %= dim;
        j %= dim;
        k %= dim;
        return f(i, j, k);
    }

    float get(glm::vec3 vec) const
    {
        vec = glm::abs(vec);
        double pixel_xd;
        float alpha_x = modf(vec.x, &pixel_xd);
        double pixel_yd;
        float alpha_y = modf(vec.y, &pixel_yd);
        double pixel_zd;
        float alpha_z = modf(vec.z, &pixel_zd);
        int px = static_cast<int>(pixel_xd) % dim;
        int py = static_cast<int>(pixel_yd) % dim;
        int pz = static_cast<int>(pixel_zd) % dim;

        float e0 = linear_interpolate(alpha_x, f(px, py, pz), f(px + 1, py, pz));
        float e1 = linear_interpolate(alpha_x, f(px, py + 1, pz), f(px + 1, py + 1, pz));
        float e2 = linear_interpolate(alpha_x, f(px, py, pz + 1), f(px + 1, py, pz + 1));
        float e3 = linear_interpolate(alpha_x, f(px, py + 1, pz + 1), f(px + 1, py + 1, pz + 1));

        float e4 = linear_interpolate(alpha_y, e0, e1);
        float e5 = linear_interpolate(alpha_y, e2, e3);

        float e6 = linear_interpolate(alpha_z, e4, e5);

      //  std::cout << px << ", " << py << ", " << pz << ", " << e6 << std::endl;
        return e6;
    }

private:
    float* data;
    int dim;
    int dim2;
    int size;

    inline float f(int i, int j, int k) const
    {
        int index = ((i * dim2) + (j * dim) + k);
        return data[index];
    }

    inline float linear_interpolate(float alpha, float v1, float v2) const
    {
        return ((1 - alpha) * v1) + (alpha * v2);
    }
};

DataMat gen_3d_noise(int dim);

#define TAB_SIZE 512
#include <math.h>

extern float gradient_vecs[TAB_SIZE * 3]; 
extern int permuli[TAB_SIZE];
extern void init_grad_vecs(int seed);

class GradNoise
{
public:

  float sample(glm::vec3 p)
  {
    float lat_x, lat_y, lat_z, fx, fy, fz;
    fx = fade(modff(p.x, &lat_x));
    fy = fade(modff(p.y, &lat_y));
    fz = fade(modff(p.z, &lat_z));
    
    int ix, iy, iz;
    ix = static_cast<int>(lat_x);
    iy = static_cast<int>(lat_y);
    iz = static_cast<int>(lat_z);

    glm::vec3 v1 = get_noise_vec(ix, iy, iz + 1);
    glm::vec3 v2 = get_noise_vec(ix + 1, iy, iz + 1);
    glm::vec3 v3 = get_noise_vec(ix+ 1, iy, iz);
    glm::vec3 v4 = get_noise_vec(ix, iy, iz);
    glm::vec3 v5 = get_noise_vec(ix, iy + 1, iz + 1);
    glm::vec3 v6 = get_noise_vec(ix + 1, iy + 1, iz + 1);
    glm::vec3 v7 = get_noise_vec(ix+ 1, iy + 1, iz);
    glm::vec3 v8 = get_noise_vec(ix, iy + 1, iz);


    glm::vec3 u1(fx, fy, fz - 1.0);
    glm::vec3 u2(fx - 1.0, fy, fz - 1.0);
    glm::vec3 u3(fx - 1.0, fy, fz);
    glm::vec3 u4(fx, fy, fz);
    glm::vec3 u5(fx, fy - 1.0, fz - 1.0);
    glm::vec3 u6(fx - 1.0, fy - 1.0, fz - 1.0);
    glm::vec3 u7(fx - 1.0, fy - 1.0, fz);
    glm::vec3 u8(fx, fy - 1.0, fz);

    float val1 = glm::dot(v1, u1);
    float val2 = glm::dot(v2, u2);
    float val3 = glm::dot(v3, u3);
    float val4 = glm::dot(v4, u4);
    float val5 = glm::dot(v5, u5);
    float val6 = glm::dot(v6, u6);
    float val7 = glm::dot(v7, u7);
    float val8 = glm::dot(v8, u8);

    float a12 = linear_interpolate(fx, val1, val2);
    float a43 = linear_interpolate(fx, val4, val3);
    float a56 = linear_interpolate(fx, val5, val6);
    float a87 = linear_interpolate(fx, val8, val7);

    float b12_56 = linear_interpolate(fy, a12, a56);
    float b43_87 = linear_interpolate(fy, a43, a87);

    float c = linear_interpolate(fz, b43_87, b12_56);

    return c;
  }

private:
  inline float linear_interpolate(float alpha, float v1, float v2) const
  {
      return ((1 - alpha) * v1) + (alpha * v2);
  }

  inline float fade(float x)
  {
    return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
  }

  glm::vec3 get_noise_vec(int x, int y, int z);
};

