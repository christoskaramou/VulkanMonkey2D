# VulkanMonkey

A small demo about this project engine: [DEMO](https://www.dropbox.com/s/vfkxy1qfr16ljkw/demo.7z?dl=0) 

Controls: move -> WASD, bar rotate -> Space, Pause -> P, ambient light on/off -> PGUP/PGDN, player light radius -> Left/Right Arrow, FPS control-> +/- 

Play around with vulkan and box2D

Dependencies: glm, glfw, vulkan, stb_image, box2d (already imported, just link)

For validation_layers install LunarG Vulkan and add to environment variables of your pc this

Variable name: VK_INSTANCE_LAYERS

Variable value: VK_LAYER_LUNARG_standard_validation

Variable name: VK_LAYER_PATH

Variable value: "path/to/build/layers" (release or debug build)
