#version 430 core

const vec3 CORNER_VECTORS[8] = vec3[8](
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 1.0, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0)
);

const int CONFIG_TO_EDGE_LIST[256][12] = int[256][12]( 
    int[](0,0,0,0,0,0,0,0,0,0,0,0),
    int[](0,4,3,0,0,0,0,0,0,0,0,0),
    int[](5,0,1,0,0,0,0,0,0,0,0,0),
    int[](3,1,4,1,5,4,0,0,0,0,0,0),
    int[](1,2,6,0,0,0,0,0,0,0,0,0),
    int[](1,2,6,0,4,3,0,0,0,0,0,0),
    int[](6,5,2,5,0,2,0,0,0,0,0,0),
    int[](4,5,6,4,6,2,3,4,2,0,0,0),
    int[](2,3,7,0,0,0,0,0,0,0,0,0),
    int[](4,7,0,7,2,0,0,0,0,0,0,0),
    int[](2,3,7,1,5,0,0,0,0,0,0,0),
    int[](7,4,5,7,5,1,2,7,1,0,0,0),
    int[](7,6,3,6,1,3,0,0,0,0,0,0),
    int[](6,7,4,6,4,0,1,6,0,0,0,0),
    int[](5,6,7,5,7,3,0,5,3,0,0,0),
    int[](4,5,7,5,6,7,0,0,0,0,0,0),
    int[](4,8,11,0,0,0,0,0,0,0,0,0),
    int[](11,3,8,3,0,8,0,0,0,0,0,0),
    int[](5,0,1,8,11,4,0,0,0,0,0,0),
    int[](1,3,11,1,11,8,5,1,8,0,0,0),
    int[](4,8,11,2,6,1,0,0,0,0,0,0),
    int[](11,3,8,3,0,8,2,6,1,0,0,0),
    int[](6,5,2,5,0,2,8,11,4,0,0,0),
    int[](2,3,11,2,11,5,5,8,11,2,6,5),
    int[](3,7,2,4,8,11,0,0,0,0,0,0),
    int[](8,0,2,8,2,7,11,8,7,0,0,0),
    int[](5,0,1,8,11,4,7,2,3,0,0,0),
    int[](7,11,8,7,8,5,7,5,2,2,5,1),
    int[](1,3,6,3,7,6,4,8,11,0,0,0),
    int[](7,6,11,11,8,0,11,8,6,6,0,1),
    int[](4,8,11,3,5,0,3,7,5,7,6,5),
    int[](7,6,5,7,5,8,11,7,8,0,0,0),
    int[](8,5,9,0,0,0,0,0,0,0,0,0),
    int[](0,4,3,5,9,8,0,0,0,0,0,0),
    int[](1,9,0,9,8,0,0,0,0,0,0,0),
    int[](9,1,3,9,3,4,8,9,4,0,0,0),
    int[](5,9,8,1,2,6,0,0,0,0,0,0),
    int[](8,5,9,4,3,0,2,6,1,0,0,0),
    int[](2,0,8,2,8,9,6,2,9,0,0,0),
    int[](2,3,4,2,4,8,2,8,6,6,8,9),
    int[](8,5,9,7,2,3,0,0,0,0,0,0),
    int[](2,0,7,0,4,7,5,9,8,0,0,0),
    int[](8,0,9,0,1,9,3,7,2,0,0,0),
    int[](1,9,2,2,7,4,2,7,9,9,4,8),
    int[](7,6,3,6,1,3,9,8,5,0,0,0),
    int[](5,9,8,0,6,1,0,4,6,4,7,6),
    int[](9,6,7,9,7,0,0,3,7,9,8,0),
    int[](4,7,6,4,6,9,8,4,9,0,0,0),
    int[](9,11,5,11,4,5,0,0,0,0,0,0),
    int[](3,11,9,3,9,5,0,3,5,0,0,0),
    int[](11,9,1,11,1,0,4,11,0,0,0,0),
    int[](1,3,9,3,11,9,0,0,0,0,0,0),
    int[](4,5,11,5,9,11,1,2,6,0,0,0),
    int[](1,2,6,5,3,0,5,9,3,9,11,3),
    int[](9,11,6,6,2,0,6,2,11,11,0,4),
    int[](9,11,3,9,3,2,6,9,2,0,0,0),
    int[](9,11,5,11,4,5,7,2,3,0,0,0),
    int[](7,11,9,7,9,0,0,5,9,7,2,0),
    int[](3,7,2,0,11,4,0,1,11,1,9,11),
    int[](1,9,11,1,11,7,2,1,7,0,0,0),
    int[](4,5,11,11,5,9,3,7,1,7,6,1),
    int[](11,9,7,9,6,7,5,0,1,0,0,0),
    int[](6,7,9,7,11,9,3,0,4,0,0,0),
    int[](11,9,7,9,6,7,0,0,0,0,0,0),
    int[](6,10,9,0,0,0,0,0,0,0,0,0),
    int[](0,4,3,6,10,9,0,0,0,0,0,0),
    int[](9,6,10,5,0,1,0,0,0,0,0,0),
    int[](3,1,4,1,5,4,6,10,9,0,0,0),
    int[](9,1,10,1,2,10,0,0,0,0,0,0),
    int[](9,1,10,1,2,10,0,4,3,0,0,0),
    int[](10,2,0,10,0,5,9,10,5,0,0,0),
    int[](5,4,9,9,10,2,9,10,4,4,2,3),
    int[](6,10,9,2,3,7,0,0,0,0,0,0),
    int[](4,7,0,7,2,0,10,9,6,0,0,0),
    int[](0,1,5,3,7,2,10,9,6,0,0,0),
    int[](6,10,9,1,7,2,1,5,7,5,4,7),
    int[](3,1,9,3,9,10,7,3,10,0,0,0),
    int[](0,1,9,0,9,7,7,10,9,0,4,7),
    int[](5,9,10,5,10,7,5,7,0,0,7,3),
    int[](5,4,7,5,7,10,9,5,10,0,0,0),
    int[](8,11,4,9,6,10,0,0,0,0,0,0),
    int[](0,8,3,8,11,3,9,6,10,0,0,0),
    int[](4,8,11,0,1,5,6,10,9,0,0,0),
    int[](9,6,10,8,1,5,8,11,1,11,3,1),
    int[](2,10,1,10,9,1,11,4,8,0,0,0),
    int[](0,8,3,3,8,11,1,2,9,2,10,9),
    int[](8,11,4,5,10,9,5,0,10,0,2,10),
    int[](3,11,2,11,10,2,8,5,9,0,0,0),
    int[](6,10,9,2,3,7,4,8,11,0,0,0),
    int[](10,9,6,7,8,11,7,2,8,2,0,8),
    int[](0,4,3,2,6,1,5,8,9,7,11,10),
    int[](2,6,1,7,11,10,8,5,9,0,0,0),
    int[](11,4,8,10,3,7,10,9,3,9,1,3),
    int[](1,9,0,9,8,0,10,7,11,0,0,0),
    int[](0,4,3,5,9,8,10,7,11,0,0,0),
    int[](9,8,5,10,7,11,0,0,0,0,0,0),
    int[](5,6,8,6,10,8,0,0,0,0,0,0),
    int[](10,8,6,8,5,6,4,3,0,0,0,0),
    int[](0,8,10,0,10,6,1,0,6,0,0,0),
    int[](6,1,3,6,3,8,8,4,3,6,10,8),
    int[](8,10,2,8,2,1,5,8,1,0,0,0),
    int[](0,4,3,1,8,5,1,2,8,2,10,8),
    int[](0,8,2,8,10,2,0,0,0,0,0,0),
    int[](2,10,8,2,8,4,3,2,4,0,0,0),
    int[](5,6,8,6,10,8,2,3,7,0,0,0),
    int[](2,0,7,7,0,4,6,10,5,10,8,5),
    int[](2,3,7,6,0,1,6,10,0,10,8,0),
    int[](8,10,4,10,7,4,6,1,2,0,0,0),
    int[](1,3,5,5,8,10,5,8,3,3,10,7),
    int[](7,4,10,4,8,10,0,1,5,0,0,0),
    int[](10,8,0,10,0,3,7,10,3,0,0,0),
    int[](7,4,10,4,8,10,0,0,0,0,0,0),
    int[](6,5,4,6,4,11,10,6,11,0,0,0),
    int[](3,11,0,0,5,6,0,5,11,11,6,10),
    int[](0,1,6,0,6,10,0,10,4,4,10,11),
    int[](11,3,1,11,1,6,10,11,6,0,0,0),
    int[](1,5,4,1,4,10,10,11,4,1,2,10),
    int[](10,2,11,2,3,11,1,5,0,0,0,0),
    int[](0,2,10,0,10,11,4,0,11,0,0,0),
    int[](3,11,2,11,10,2,0,0,0,0,0,0),
    int[](7,2,3,11,6,10,11,4,6,4,5,6),
    int[](0,2,5,2,6,5,7,11,10,0,0,0),
    int[](1,2,6,0,4,3,11,10,7,0,0,0),
    int[](10,7,11,6,1,2,0,0,0,0,0,0),
    int[](5,4,1,4,3,1,11,10,7,0,0,0),
    int[](5,0,1,10,7,11,0,0,0,0,0,0),
    int[](4,3,0,11,10,7,0,0,0,0,0,0),
    int[](10,7,11,0,0,0,0,0,0,0,0,0),
    int[](10,7,11,0,0,0,0,0,0,0,0,0),
    int[](4,3,0,11,10,7,0,0,0,0,0,0),
    int[](5,0,1,10,7,11,0,0,0,0,0,0),
    int[](5,4,1,4,3,1,11,10,7,0,0,0),
    int[](10,7,11,6,1,2,0,0,0,0,0,0),
    int[](1,2,6,0,4,3,11,10,7,0,0,0),
    int[](0,2,5,2,6,5,7,11,10,0,0,0),
    int[](7,2,3,11,6,10,11,4,6,4,5,6),
    int[](3,11,2,11,10,2,0,0,0,0,0,0),
    int[](0,2,10,0,10,11,4,0,11,0,0,0),
    int[](10,2,11,2,3,11,1,5,0,0,0,0),
    int[](1,5,4,1,4,10,10,11,4,1,2,10),
    int[](11,3,1,11,1,6,10,11,6,0,0,0),
    int[](0,1,6,0,6,10,0,10,4,4,10,11),
    int[](3,11,0,0,5,6,0,5,11,11,6,10),
    int[](6,5,4,6,4,11,10,6,11,0,0,0),
    int[](7,4,10,4,8,10,0,0,0,0,0,0),
    int[](10,8,0,10,0,3,7,10,3,0,0,0),
    int[](7,4,10,4,8,10,0,1,5,0,0,0),
    int[](1,3,5,5,8,10,5,8,3,3,10,7),
    int[](8,10,4,10,7,4,6,1,2,0,0,0),
    int[](2,3,7,6,0,1,6,10,0,10,8,0),
    int[](2,0,7,7,0,4,6,10,5,10,8,5),
    int[](5,6,8,6,10,8,2,3,7,0,0,0),
    int[](2,10,8,2,8,4,3,2,4,0,0,0),
    int[](0,8,2,8,10,2,0,0,0,0,0,0),
    int[](0,4,3,1,8,5,1,2,8,2,10,8),
    int[](8,10,2,8,2,1,5,8,1,0,0,0),
    int[](6,1,3,6,3,8,8,4,3,6,10,8),
    int[](0,8,10,0,10,6,1,0,6,0,0,0),
    int[](10,8,6,8,5,6,4,3,0,0,0,0),
    int[](5,6,8,6,10,8,0,0,0,0,0,0),
    int[](9,8,5,10,7,11,0,0,0,0,0,0),
    int[](0,4,3,5,9,8,10,7,11,0,0,0),
    int[](1,9,0,9,8,0,10,7,11,0,0,0),
    int[](11,4,8,10,3,7,10,9,3,9,1,3),
    int[](2,6,1,7,11,10,8,5,9,0,0,0),
    int[](0,4,3,2,6,1,5,8,9,7,11,10),
    int[](10,9,6,7,8,11,7,2,8,2,0,8),
    int[](6,10,9,2,3,7,4,8,11,0,0,0),
    int[](3,11,2,11,10,2,8,5,9,0,0,0),
    int[](8,11,4,5,10,9,5,0,10,0,2,10),
    int[](0,8,3,3,8,11,1,2,9,2,10,9),
    int[](2,10,1,10,9,1,11,4,8,0,0,0),
    int[](9,6,10,8,1,5,8,11,1,11,3,1),
    int[](4,8,11,0,1,5,6,10,9,0,0,0),
    int[](0,8,3,8,11,3,9,6,10,0,0,0),
    int[](8,11,4,9,6,10,0,0,0,0,0,0),
    int[](5,4,7,5,7,10,9,5,10,0,0,0),
    int[](5,9,10,5,10,7,5,7,0,0,7,3),
    int[](0,1,9,0,9,7,7,10,9,0,4,7),
    int[](3,1,9,3,9,10,7,3,10,0,0,0),
    int[](6,10,9,1,7,2,1,5,7,5,4,7),
    int[](0,1,5,3,7,2,10,9,6,0,0,0),
    int[](4,7,0,7,2,0,10,9,6,0,0,0),
    int[](6,10,9,2,3,7,0,0,0,0,0,0),
    int[](5,4,9,9,10,2,9,10,4,4,2,3),
    int[](10,2,0,10,0,5,9,10,5,0,0,0),
    int[](9,1,10,1,2,10,0,4,3,0,0,0),
    int[](9,1,10,1,2,10,0,0,0,0,0,0),
    int[](3,1,4,1,5,4,6,10,9,0,0,0),
    int[](9,6,10,5,0,1,0,0,0,0,0,0),
    int[](0,4,3,6,10,9,0,0,0,0,0,0),
    int[](6,10,9,0,0,0,0,0,0,0,0,0),
    int[](11,9,7,9,6,7,0,0,0,0,0,0),
    int[](6,7,9,7,11,9,3,0,4,0,0,0),
    int[](11,9,7,9,6,7,5,0,1,0,0,0),
    int[](4,5,11,11,5,9,3,7,1,7,6,1),
    int[](1,9,11,1,11,7,2,1,7,0,0,0),
    int[](3,7,2,0,11,4,0,1,11,1,9,11),
    int[](7,11,9,7,9,0,0,5,9,7,2,0),
    int[](9,11,5,11,4,5,7,2,3,0,0,0),
    int[](9,11,3,9,3,2,6,9,2,0,0,0),
    int[](9,11,6,6,2,0,6,2,11,11,0,4),
    int[](1,2,6,5,3,0,5,9,3,9,11,3),
    int[](4,5,11,5,9,11,1,2,6,0,0,0),
    int[](1,3,9,3,11,9,0,0,0,0,0,0),
    int[](11,9,1,11,1,0,4,11,0,0,0,0),
    int[](3,11,9,3,9,5,0,3,5,0,0,0),
    int[](9,11,5,11,4,5,0,0,0,0,0,0),
    int[](4,7,6,4,6,9,8,4,9,0,0,0),
    int[](9,6,7,9,7,0,0,3,7,9,8,0),
    int[](5,9,8,0,6,1,0,4,6,4,7,6),
    int[](7,6,3,6,1,3,9,8,5,0,0,0),
    int[](1,9,2,2,7,4,2,7,9,9,4,8),
    int[](8,0,9,0,1,9,3,7,2,0,0,0),
    int[](2,0,7,0,4,7,5,9,8,0,0,0),
    int[](8,5,9,7,2,3,0,0,0,0,0,0),
    int[](2,3,4,2,4,8,2,8,6,6,8,9),
    int[](2,0,8,2,8,9,6,2,9,0,0,0),
    int[](8,5,9,4,3,0,2,6,1,0,0,0),
    int[](5,9,8,1,2,6,0,0,0,0,0,0),
    int[](9,1,3,9,3,4,8,9,4,0,0,0),
    int[](1,9,0,9,8,0,0,0,0,0,0,0),
    int[](0,4,3,5,9,8,0,0,0,0,0,0),
    int[](8,5,9,0,0,0,0,0,0,0,0,0),
    int[](7,6,5,7,5,8,11,7,8,0,0,0),
    int[](4,8,11,3,5,0,3,7,5,7,6,5),
    int[](7,6,11,11,8,0,11,8,6,6,0,1),
    int[](1,3,6,3,7,6,4,8,11,0,0,0),
    int[](7,11,8,7,8,5,7,5,2,2,5,1),
    int[](5,0,1,8,11,4,7,2,3,0,0,0),
    int[](8,0,2,8,2,7,11,8,7,0,0,0),
    int[](3,7,2,4,8,11,0,0,0,0,0,0),
    int[](2,3,11,2,11,5,5,8,11,2,6,5),
    int[](6,5,2,5,0,2,8,11,4,0,0,0),
    int[](11,3,8,3,0,8,2,6,1,0,0,0),
    int[](4,8,11,2,6,1,0,0,0,0,0,0),
    int[](1,3,11,1,11,8,5,1,8,0,0,0),
    int[](5,0,1,8,11,4,0,0,0,0,0,0),
    int[](11,3,8,3,0,8,0,0,0,0,0,0),
    int[](4,8,11,0,0,0,0,0,0,0,0,0),
    int[](4,5,7,5,6,7,0,0,0,0,0,0),
    int[](5,6,7,5,7,3,0,5,3,0,0,0),
    int[](6,7,4,6,4,0,1,6,0,0,0,0),
    int[](7,6,3,6,1,3,0,0,0,0,0,0),
    int[](7,4,5,7,5,1,2,7,1,0,0,0),
    int[](2,3,7,1,5,0,0,0,0,0,0,0),
    int[](4,7,0,7,2,0,0,0,0,0,0,0),
    int[](2,3,7,0,0,0,0,0,0,0,0,0),
    int[](4,5,6,4,6,2,3,4,2,0,0,0),
    int[](6,5,2,5,0,2,0,0,0,0,0,0),
    int[](1,2,6,0,4,3,0,0,0,0,0,0),
    int[](1,2,6,0,0,0,0,0,0,0,0,0),
    int[](3,1,4,1,5,4,0,0,0,0,0,0),
    int[](5,0,1,0,0,0,0,0,0,0,0,0),
    int[](0,4,3,0,0,0,0,0,0,0,0,0),
    int[](0,0,0,0,0,0,0,0,0,0,0,0)
);

const int CONFIG_TO_VERTEX_COUNT[256] = int[256](
    0,
    3,
    3,
    6,
    3,
    6,
    6,
    9,
    3,
    6,
    6,
    9,
    6,
    9,
    9,
    6,
    3,
    6,
    6,
    9,
    6,
    9,
    9,
    12,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    3,
    6,
    6,
    9,
    6,
    9,
    9,
    12,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    6,
    9,
    9,
    6,
    9,
    12,
    12,
    9,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    3,
    6,
    6,
    9,
    6,
    9,
    9,
    12,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    6,
    9,
    9,
    12,
    9,
    12,
    6,
    9,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    12,
    9,
    9,
    6,
    9,
    6,
    6,
    3,
    3,
    6,
    6,
    9,
    6,
    9,
    9,
    12,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    9,
    6,
    12,
    9,
    12,
    9,
    9,
    6,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    12,
    9,
    9,
    6,
    9,
    6,
    6,
    3,
    6,
    9,
    9,
    12,
    9,
    12,
    12,
    9,
    9,
    12,
    12,
    9,
    6,
    9,
    9,
    6,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    12,
    9,
    9,
    6,
    9,
    6,
    6,
    3,
    9,
    12,
    12,
    9,
    12,
    9,
    9,
    6,
    12,
    9,
    9,
    6,
    9,
    6,
    6,
    3,
    6,
    9,
    9,
    6,
    9,
    6,
    6,
    3,
    9,
    6,
    6,
    3,
    6,
    3,
    3,
    0
);

const ivec2 EDGE_TO_VERTICES_LIST[12] = ivec2[12](
    ivec2(0, 1),
    ivec2(1, 2),
    ivec2(2, 3),
    ivec2(3, 0),

    ivec2(0, 4),
    ivec2(1, 5),
    ivec2(2, 6),
    ivec2(3, 7),

    ivec2(4, 5),
    ivec2(5, 6),
    ivec2(6, 7),
    ivec2(7, 4)
);

const int uPerm[512] = int[512] (
    151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
   151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
);

const vec3[15] GRAD_MAP = vec3[15] (
    vec3(1.0, 1.0, 0.0),
    vec3(-1.0, 1.0, 0.0),
    vec3(1.0, -1.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    vec3(-1.0, 0.0, 1.0),
    vec3(1.0, 0.0, -1.0),
    vec3(-1.0, 0.0, -1.0),
    vec3(0.0, 1.0, 1.0),
    vec3(0.0, -1.0, 1.0),
    vec3(0.0, 1.0, -1.0),
    vec3(0.0, -1.0, -1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, -1.0, 1.0),
    vec3(-1.0, 1.0, 0.0),
    vec3(0.0, -1.0, -1.0)
);

uniform sampler3D noiseTexture1;
uniform sampler3D noiseTexture2;
uniform sampler3D noiseTexture3;
uniform sampler3D noiseTexture4;

uniform vec3 uChunkPosition;
uniform float uNumVoxelsPerDim;
uniform float uVoxelSize;

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

out vec3 outVec;
out vec3 outNormal;

float grad(int hash, float x, float y, float z)
{
  hash = hash & 0xF;
  return dot(vec3(x, y, z), GRAD_MAP[hash]);
}

float fade(float t)
{
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

int inc(int t)
{
  // TODO: May need to add repeat logic here!
  return t + 1;
}

int triple_hash_search(int a, int b, int c)
{
  return uPerm[uPerm[uPerm[a] + b] + c];
}

float perlin_noise(vec3 vec)
{
  // TODO: May need to add something here to slow down the rate of sampling. i.e. One marching cube != One noise cube
  // TODO: Add repeat logic
  float x = vec.x;
  float y = vec.y;
  float z = vec.z;

  int xi = int(x) & 255;
  int yi = int(y) & 255;
  int zi = int(z) & 255;

  float xf = fract(x);
  float yf = fract(y);
  float zf = fract(z);

  float u = fade(xf);
  float v = fade(yf);
  float w = fade(zf);

  int aaa = triple_hash_search(xi,yi,zi);
  int aba = triple_hash_search(xi,inc(yi),zi);
  int aa1 = triple_hash_search(xi,inc(yi),zi);
  int ab1 = triple_hash_search(xi,inc(yi),inc(zi));
  int baa = triple_hash_search(inc(xi),yi,zi);
  int bba = triple_hash_search(inc(xi),inc(yi),zi);
  int ba1 = triple_hash_search(inc(xi),yi,inc(zi));
  int bb1 = triple_hash_search(inc(xi),inc(yi),inc(zi));

  float x1, x2, y1, y2;
  x1 = mix(grad(aaa, xf, yf, zf), grad(baa, xf-1.0, yf, zf), u);
  x2 = mix(grad(aba, xf, yf-1.0, zf), grad(bba, xf-1.0, yf-1.0, zf), u);
  y1 = mix(x1, x2, v);

  x1 = mix(grad(aa1, xf, yf, zf-1.0), grad(ba1, xf-1.0, yf, zf-1.0), u);
  x2 = mix(grad(ab1, xf, yf-1.0, zf-1.0), grad(bb1, xf-1.0, yf-1.0, zf-1.0), u);
  y2 = mix(x1, x2, v);

  // Could bind the result to 0 and 1, is that a good idea?
  return mix(y1, y2, w);
};


/**
* p is in worldl space.
**/
float sampleDensityFn(vec3 p)
{
    p = p / 8.0;
    float density = 0.0;// -p.y;
    density += perlin_noise(p );
    return density;
}

/**
* IMPORTANT! Assumes vector is one of the corner vectors
*/
vec3 get_normal_for_vec(vec3 corner_vector)
{
    const vec3 offset_x = vec3(uVoxelSize, 0.0, 0.0);
    const vec3 offset_y = vec3(0.0, uVoxelSize, 0.0);
    const vec3 offset_z = vec3(0.0, 0.0, uVoxelSize);

    vec3 normal;
    normal.x = sampleDensityFn(corner_vector + offset_x) - sampleDensityFn(corner_vector - offset_x);
    normal.y = sampleDensityFn(corner_vector + offset_y) - sampleDensityFn(corner_vector - offset_y);
    normal.z = sampleDensityFn(corner_vector + offset_z) - sampleDensityFn(corner_vector - offset_z);

    return -normalize(normal);
}

void emitVertex(int edge, float corner_vals[8], int config)
{
    ivec2 edge_vertices = EDGE_TO_VERTICES_LIST[edge];
    vec3 v1 = CORNER_VECTORS[edge_vertices.x];
    vec3 v2 = CORNER_VECTORS[edge_vertices.y];
    float vertex_val1 = abs(corner_vals[edge_vertices.x]);
    float vertex_val2 = abs(corner_vals[edge_vertices.y]);
    float alpha = vertex_val1 / (vertex_val1 + vertex_val2);

    vec3 interpolated_vertex = ((1 - alpha) * v1)  + (alpha * v2);

    // Move the interpolated poitn into worls space
    vec3 vertex_chunk_space = (interpolated_vertex * uVoxelSize) + gl_in[0].gl_Position.xyz;
    outVec.xyz = vertex_chunk_space + uChunkPosition;
    outNormal = get_normal_for_vec(outVec);// normalize(normal);
    EmitVertex();
}

void main()
{

    float corner_vals[8];
    vec3 global_voxel_pos = gl_in[0].gl_Position.xyz + uChunkPosition;
    corner_vals[0] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[0] * uVoxelSize));
    corner_vals[1] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[1] * uVoxelSize));
    corner_vals[2] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[2] * uVoxelSize));
    corner_vals[3] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[3] * uVoxelSize));
    corner_vals[4] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[4] * uVoxelSize));
    corner_vals[5] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[5] * uVoxelSize));
    corner_vals[6] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[6] * uVoxelSize));
    corner_vals[7] = sampleDensityFn(global_voxel_pos + (CORNER_VECTORS[7] * uVoxelSize));

    int config = 0;
    for (int i = 0; i < 8; i++)
    {
        config |= (int((corner_vals[i] <= 0.0)) << i);
    }
    
    int edges[12] = CONFIG_TO_EDGE_LIST[config];
    int edge_count = CONFIG_TO_VERTEX_COUNT[config];
    for (int i = 0; i < edge_count; i += 3)
    {
        emitVertex(edges[i], corner_vals, config);
        emitVertex(edges[i+1], corner_vals, config);
        emitVertex(edges[i+2], corner_vals, config);
        EndPrimitive();
    }   
}
