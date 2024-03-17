# Flowfield Generator with Perlin Noise
This project presents a flowfield generator implemented in C, using the basic SDL graphic library for rendering. The core algorithm employs a custom implementation of Perlin noise, to generate intricate and dynamic flow patterns.

![FlowField_1](https://github.com/thomas-francois/FlowField/assets/103375765/e61e0c7f-eab5-476c-bf9d-2222accc36b2)
![FlowField_2](https://github.com/thomas-francois/FlowField/assets/103375765/8cd35151-b23d-4f9e-86df-4c4030e5c42f)
![FlowField_3](https://github.com/thomas-francois/FlowField/assets/103375765/6d1a46a8-d4fb-4020-9d2e-437c8d0316a6)

## Features
- **Advanced Perlin Noise Implementation:** The project features a custom implementation of Perlin noise generated by combining multiple gradient vectors at different frequencies and amplitudes to create smooth, coherent random patterns. This noise produces visually captivating flow fields with subtle variations and naturalistic movements.

- **Custom User Interface:** A custom set of widget (color pickers, sliders, buttons) was handcrafted to be able to control the flow field. This user-friendly interface empowers users to manipulate the flow field properties, allowing for real-time experimentation and exploration of possibilities.

- **Dynamic Rendering Options:** The project offers flexible rendering options, enabling users to seamlessly switch between the flow field renderer and the Perlin noise renderer. This dynamic functionality provides users with insights into the underlying noise generation process.

##  Requirements
To compile and run the project, ensure you have the following dependencies:

- SDL library: The SDL library provides essential functionality for graphics and input handling.
- C compiler: A compatible C compiler, such as GCC, is required to compile the project source code.

## Contributing
Contributions are welcome! Whether you're interested in fixing bugs, adding features, or improving documentation, feel free to submit a pull request.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
