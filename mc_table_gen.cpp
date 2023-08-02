#include <iostream>
#include <vector>
#include <set>

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

/** * Represents the edge list for each of the base cases.
 * Will be used to calculate all 256 triangulations for quick lookup in the shader code.
*/
const uint8_t base_triangulations[15][12] {
    { },
    { 0, 4, 3 },
    { 3, 1, 4,  1, 5, 4},
    { 0, 4, 3,  5, 9, 8},
    { 0, 4, 3,  6, 10, 9},
    { 5, 6, 7,  5, 7, 3,  0, 5, 3},
    { 3, 1, 4,  1, 5, 4,  6, 10, 9},
    { 4, 8, 11,  0, 1, 5,  6, 10, 9},
    { 4, 5, 7,  5, 6, 7},  // 8
    { 0, 1, 6,  0, 6, 10,  0, 10, 4,  4, 10, 11},
    { 0, 8, 3,  3, 8, 11,  1, 2, 9,  2, 10, 9},
    { 0, 1, 9,  0, 9, 7,  7, 10, 9,  0, 4, 7},
    { 4, 8, 11,  3, 5, 0,  3, 7, 5,  7, 6, 5},
    { 0, 4, 3,  2, 6, 1,  5, 8, 9,  7, 11, 10},
    { 3, 11, 0,  0, 5, 6,  0, 5, 11,  11, 6, 10}
};

const uint8_t base_triangulations_count[15] {
    0,
    3,
    6,
    6,
    6,
    9,
    9,
    9,
    6,
    12,
    12,
    12,
    12,
    12,
    12,
};

uint8_t triangulations[256][12] {};

/**
 * Use this to convert the edge lists to their vertex coordinates.
*/
const float vertex_coords[12][3] {
    {  0.0f, -0.5f,  0.5f },
    {  0.5f, -0.5f,  0.0f },
    {  0.0f, -0.5f, -0.5f },
    { -0.5f, -0.5f,  0.0f },
    { -0.5f,  0.0f,  0.5f },
    {  0.5f,  0.0f,  0.5f },
    {  0.5f,  0.0f, -0.5f },
    { -0.5f,  0.0f, -0.5f },
    {  0.0f,  0.5f,  0.5f },
    {  0.5f,  0.5f,  0.0f },
    {  0.0f,  0.5f, -0.5f },
    { -0.5f,  0.5f,  0.0f }
};


const uint8_t edge_rotation_map_y[12] {
    3,
    0,
    1,
    2,

    7,
    4,
    5,
    6,

    11,
    8,
    9,
    10,
};

const uint8_t edge_rotation_map_x[12] {
    2,
    6,
    10,
    7,

    3,
    1,
    9,
    11,

    0,
    5,
    8,
    4,
};

const uint8_t edge_rotation_map_z[12] {
    5,
    9,
    6,
    1,

    0,
    8,
    10,
    2,

    4,
    11,
    7,
    3,
};

const uint8_t edge_to_vertex_map[12][2] {
    {    0, 1 },
    {    1, 2 },
    {    2, 3 },
    {    3, 0 },

    {    0, 4 },
    {    1, 5 },
    {    2, 6 },
    {    3, 7 },

    {    4, 5 },
    {    5, 6 },
    {    6, 7 },
    {    7, 4 },
};

glm::vec3 corner_vertices[8] {
    glm::vec3(0.0, 0.0, 1.0),
    glm::vec3( 1.0, 0.0, 1.0 ),
    glm::vec3( 1.0, 0.0, 0.0 ),
    glm::vec3( 0.0, 0.0, 0.0 ),

    glm::vec3( 0.0, 1.0, 1.0 ),
    glm::vec3( 1.0, 1.0, 1.0 ),
    glm::vec3( 1.0, 1.0, 0.0 ),
    glm::vec3( 0.0, 1.0, 0.0 ),
};


uint8_t vertex_map[256] {};

/**
 * Records the rotations per config.
*/
uint8_t rotation_map[256][3] {};

std::vector<float> getInterpolatedVertices(uint8_t code, float* corner_values)
{
    uint8_t base_case_index = vertex_map[code];
    uint8_t vertex_count = base_triangulations_count[base_case_index];
    uint8_t* edges = triangulations[code];

    std::vector<float> vertices;
    for (int i = 0; i < vertex_count; i++)
    {
        uint8_t edge = edges[i];
        const uint8_t* corners = edge_to_vertex_map[edge];
        float val1 = glm::abs(corner_values[corners[0]]);
        float val2 = glm::abs(corner_values[corners[1]]);
        float d = val1 / (val1 + val2);

        glm::vec3 finalVec = (corner_vertices[corners[0]] * (1 - d)) + (corner_vertices[corners[1]] * d);
        vertices.push_back(finalVec.x);
        vertices.push_back(finalVec.y);
        vertices.push_back(finalVec.z);
    }
    return vertices;
}

std::vector<float> getVertices(uint8_t config_index) {
	uint8_t base_case_index = vertex_map[config_index];
	uint8_t vertex_count = base_triangulations_count[base_case_index];
	uint8_t* elements = (triangulations[config_index]);

    std::vector<float> vertices;
    for (int i=0; i < vertex_count; i++) {
        uint8_t edge = elements[i];
        for (float vertex: vertex_coords[edge]) {
            vertices.push_back(vertex);
        }
    }
    return vertices;
};


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

void get_base_case_from_bits(uint8_t code)
{
    uint8_t code_copy = code;

    int v_code = vertex_map[code];
    if (v_code) return; 

    uint8_t code_compliment = ~code;
    v_code = vertex_map[code_compliment];
    if (v_code) {
        vertex_map[code] = v_code;
        rotation_map[code][0] = rotation_map[code_compliment][0];
        rotation_map[code][1] = rotation_map[code_compliment][1];
        rotation_map[code][2] = rotation_map[code_compliment][2];
        return;
    }

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
                    // Match on base cases to get exact rotation needed
                    for (int w = 0; w  < 15; w++)
                    {
                        if (base_cases[w] == c_code)
                        {
                            vertex_map[code_copy] = w;
                            rotation_map[code_copy][0] = i;
                            rotation_map[code_copy][1] = j;
                            rotation_map[code_copy][2] = k;
                            return;
                        }
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

void update_triangulations(uint8_t code)
{
    uint8_t base_case_index = vertex_map[code];
    uint8_t* rotations = rotation_map[code];

    const uint8_t* edge_list = base_triangulations[base_case_index];
    const uint8_t edge_count = base_triangulations_count[base_case_index];
    for (int edge_index = 0; edge_index < edge_count; edge_index++)
    {
        uint8_t edge = edge_list[edge_index];
        for (int i = 0; i < rotations[2]; i++) edge = edge_rotation_map_z[edge];
        for (int i = 0; i < rotations[1]; i++) edge = edge_rotation_map_x[edge];
        for (int i = 0; i < rotations[0]; i++) edge = edge_rotation_map_y[edge];

        triangulations[code][edge_index] = edge;
    }
}

#include <bitset>
void print_map_debug()
{
    for (int i = 0; i < 256; i++)
    {
        std::bitset<8> bs(i);
        std::cout << i << " :: " << (int) vertex_map[i] << " --- " << bs;
        std::cout << std::endl;
    }
}

void print_tri_debug()
{
    for (int i = 0; i < 256; i++)
    {
        std::cout << i << " ::: " << (int) rotation_map[i][0] << "," << (int) rotation_map[i][1] << "," << (int) rotation_map[i][2] << " :=:";
        for (auto edge : triangulations[i])
        {
            std::cout << (int) edge << ", ";
        }
        std::cout << std::endl;
    }
}

void gen_table()
{
    for (auto i = 0; i < 15; i++)
    {
        vertex_map[base_cases[i]] = i;
    }

    for (int i = 1; i < 255; i++)
    {
        get_base_case_from_bits(i);
    }

    for (int i = 1; i < 255; i++)
    {
        update_triangulations(i);
    }

}

void print_array()
{
    std::cout << "{";
    for (int i = 0; i < 256; i++)
    {
        std::cout << (int) vertex_map[i] << "," << std::endl;
    }
    std::cout << std::endl;
}