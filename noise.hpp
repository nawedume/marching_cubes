#include <glm/glm.hpp>
#include <cmath>

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
