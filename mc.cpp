#include "mc_table_gen.cpp"
#include <bitset>

int main()
{
    gen_table();
    print_map_debug();
    std::cout << "=====" << std::endl;
    print_tri_debug();

    //uint8_t x = 1 << 1;
    //std::cout << (int) x << std::endl;
    //uint8_t n = cube_rotate_around_z(x);
    //std::cout << (int) n << std::endl;

    return 0;
}