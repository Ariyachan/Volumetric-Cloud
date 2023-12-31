This project introduces an efficient and effective method to render animated, lifelike clouds in real-time. Our clouds dynamically react to other world-parameters like the position of the camera or the sun. Our clouds are also conveniently parameterized to allow for flexible customization of its density, position, sizes, etc.

## Results
<img src="res/readme/r1.gif" width=50%><img src="res/readme/r2.gif" width=50%>

## Method and Features
* Interactive real-time smoke rendering
* Spherical voxelization of a 3D texture using billboards
* Voxel cone tracing

## Libraries Used
* [GLFW](http://www.glfw.org/)
* [GLM](https://glm.g-truc.net/0.9.8/index.html)
* [glad](https://github.com/Dav1dde/glad)
* [ImGui](https://github.com/ocornut/imgui)
* [stb_image.h](https://github.com/nothings/stb)
