C:\VulkanSDK\1.3.204.1\Bin\glslc.exe .\Square.comp -o ./Square.spv
g++ -I"C:\VulkanSDK\1.3.204.1\Include\" -L"C:\VulkanSDK\1.3.204.1\Lib" ./main.cpp -lvulkan