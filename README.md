# CS354H Final Project: Metaballs

### Kaylan Tchamdjou

Build with `cmake --build <YOUR BUILD FOLDER NAME>`. Executable will be in the build folder.

For this project I implemented Marching Cubes along with various Metaballs. The project is done in C++ using GLFW, GLAD, and GLM. When you run the project you can click left and right to change the scene of metaballs on display.

### Basic Features

- Function to Mesh Conversion via Marching Cubes
- Metaball Mesh Generation
- Metaball Normal Generation
- Diffuse Lighting / Shading
- Camera Controls (camera moves on drag)

### Update: 5/3/2025 (10:09 PM)

- Minor optimizations to increase speed
    - Instead of reallocating space for the Vector3D<ctrl_pt_t> buffer on each `refresh()` call, I instead maintain a buffer field in MetaballEngine.
    - Using `MeshView` instead of copying the meshes in the render loop
    - Skipping cubes that produce a bitfield of `0x0` or `0xFF` (0 and 255 are "empty space")

- If I really want to speed this up I need to resort to some kind of caching of Vertices

#### References:

[1] William E. Lorensen and Harvey E. Cline. 1987. Marching cubes: A high resolution 3D surface construction algorithm. SIGGRAPH Comput. Graph. 21, 4 (July 1987), 163–169. https://doi.org/10.1145/37402.37422


William E. Lorensen and Harvey E. Cline. 1987. Marching cubes: A high resolution 3D surface construction algorithm. In Proceedings of the 14th annual conference on Computer graphics and interactive techniques (SIGGRAPH '87). Association for Computing Machinery, New York, NY, USA, 163–169. https://doi.org/10.1145/37401.37422


[2] http://www.paulbourke.net/geometry/polygonise/

### EXAMPLES

![Marching Cube Metaball Example - Cube](./assets/mc_example_cube.PNG)

![Marching Cube Metaball Example - Ball plus Cube](./assets/mc_example_cube_plus_ball.PNG)

![Marching Cube Metaball Example - Gyroid](./assets/mc_example_gyroid.PNG)

![Marching Cube Metaball Example - Balls Multiple](./assets/mc_example_balls_multiple.png)

![Marching Cube Metaball Example - Warped Plane](./assets/mc_example_plane_warped.png)