#include <iostream>

const uint8_t v0 = 1;
const uint8_t v1 = v0 << 1;
const uint8_t v2 = v1 << 1;
const uint8_t v3 = v2 << 1;
const uint8_t v4 = v3 << 1;
const uint8_t v5 = v4 << 1;
const uint8_t v6 = v5 << 1;
const uint8_t v7 = v6 << 1;


const uint8_t base_cases[15] = {
    0,
    v0,
    v0 | v1,
    v0 | v5,
    v0 | v6,
    v1 | v2 | v3,
    v0 | v1 | v6,
    v1 | v4 | v6,
    v0 | v1 | v2 | v3,
    v0 | v2 | v3 | v7,
    v0 | v2 | v4 | v6,
    v0 | v2 | v3 | v6,
    v1 | v2 | v3 | v4,
    v0 | v2 | v5 | v7,
    v1 | v2 | v3 | v7,
};

int vertex_map[256] {};

const uint8_t upper = 0b1111'0000;
const uint8_t lower = 0b0000'1111;
inline uint8_t cube_rotate_around_y(uint8_t code)
{
    int c_up = upper & code;
    c_up = (c_up << 1) | (c_up >> 3);

    int c_down = lower & code;
    c_down = (c_down << 1) | (c_down >> 3);

    return (c_up & upper) | (c_down & lower);
}

inline uint8_t cube_rotate_around_x(uint8_t code)
{
    uint8_t new_code = 0;
    new_code |= (code & v3) >> 3;
    new_code |= (code & v2) >> 1;
    new_code |= (code & v6) >> 4;
    new_code |= (code & v7) >> 4;
    new_code |= (code & v0) << 4;
    new_code |= (code & v1) << 4;
    new_code |= (code & v5) << 1;
    new_code |= (code & v4) << 3;
    return new_code;
}

inline uint8_t cube_rotate_around_z(uint8_t code)
{
    uint8_t new_code = 0;
    new_code |= (code & v1) >> 1;
    new_code |= (code & v5) >> 4;
    new_code |= (code & v6) >> 4;
    new_code |= (code & v2) << 1;
    new_code |= (code & v0) << 4;
    new_code |= (code & v4) << 1;
    new_code |= (code & v7) >> 1;
    new_code |= (code & v3) << 4;
    return new_code;
}

int get_base_case_from_bits(uint8_t code)
{
    int v_code = vertex_map[code];
    if (v_code) return v_code; 

    v_code = vertex_map[!code];
    if (v_code) return v_code;

    // handle the rotational symmetry
    for (int c = 0; c < 2; c++)
    {
        uint8_t a_code = code;
        uint8_t b_code = code;
        uint8_t c_code = code;
        for (int i = 0; i < 4; i++)
        {
            b_code = a_code;
            for (int j = 0; j < 4; j++)
            {
                c_code = b_code;
                for (int k = 0; k < 4; k++)
                {
                    if (vertex_map[c_code])
                    {
                        return vertex_map[c_code];
                    }
                    c_code = cube_rotate_around_z(c_code);
                }
                b_code = cube_rotate_around_x(b_code);
            }
            a_code = cube_rotate_around_y(a_code);
        }
        code = ~code;
    }

    std::cout << "Could not find base case for case :: " << static_cast<int>(code) << std::endl;
    throw std::runtime_error("Could not find base case");
}

void gen_table()
{
    for (auto i = 0; i < 15; i++)
    {
        vertex_map[base_cases[i]] = i;
    }

    for (int i = 1; i < 255; i++)
    {
        //std::cout << "Looking for " << std::to_string(i) << "  : " << std::endl;
        int base_case = get_base_case_from_bits(i);
        //std::cout << " :: found -> " << base_case << std::endl;
        vertex_map[i] = base_case;
    }
}

#include <bitset>
void print_map_debug()
{
    for (int i = 0; i < 256; i++)
    {
        std::bitset<8> bs(i);
        std::cout << i << " :: " << vertex_map[i] << " --- " << bs;
        std::cout << std::endl;
    }
}

void print_array()
{
    std::cout << "{";
    for (int i = 0; i < 256; i++)
    {
        std::cout << vertex_map[i] << "," << std::endl;
    }
    std::cout << std::endl;
}