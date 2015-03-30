# Danger Ranger

----
## What is the Danger Ranger?
The danger ranger is an autonomous drone that performs onboard image processing to detect targets from the air. Images which contain points of interest (i.e. trained features) are sent over radio to a ground station for human inspection. In case the radio link goes down while the flight is underway, capture and detection will still take place. This data can be collected once the drone is retrieved. 

----
## How Does it Work?
1. Raw camera input (16-bit grayscale 640x480) is recieved using V4L2.
2. Grayscale stretching and 8-bit conversion is performed using OpenCL (Mali T628 GPU). This step reduces the amount of data to process by a factor of 2.
3. Keypoints are extracted using an OpenCL adapation of the OpenCV CUDA FAST detector.
4. An OpenCL implementation of the FREAK binary descriptor is used to describe each keypoint found.
5. To detect targets of interest, keypoint descriptors are matched to existing descriptors from trained data. Due to the fact that our aerial images are very similar in appearance, this step performs a very weak form of classification.
6. While keypoints are extracted and described on the GPU, the CPU uses libjpeg-turbo (NEON Accelerated) to compress the image to ~2% of its raw size. This produces heavy artifacts, but due to radio limitations, the amount of data to transfer must be minimized.
7. From here, zlib is used to perform lossless compression for a further 15% decrease in size. Note: The lossy jpeg compression level greatly influences the effectiveness of lossless zlib compression. The entire image pipeline compresses 614400 bytes of data into approximately 7000 bytes (~1% of initial).
8. The image buffer, any detection coordinates, and the current GPS location are sent over the air using the radio link. This is the major bottleneck in the process.
9. A base station receives this data where a human operator can make the final determination if the target is the 

----
## Build Requirements
### Hardware
1. OpenCL Enabled GPU
2. Camera that interfaces with V4L2

### Software
1. linux
2. opencl
3. opengl (glut/glew)
4. libjpeg-turbo
5. zlib
6. v4l2

----
## Building / Usage
    make
    ./out

----
## TODO
1. Port FAST (already implemented in CUDA)
2. Add CPU version of FREAK (already implemented)
3. Implement OpenCL FREAK (Check performance gains)
4. Add compression code using libjpeg-turbo + zlib (already implemented)
5. Add radio code (already implemented)
6. Create base station code (Only requires decompression and display)

----
## Memory Notes
All image buffers must be allocated by OpenCL. Though the CPU and GPU use shared memory, not all CPU memory is GPU accessible. In addition, V4L2 streaing enabled devices allow for user pointers to be used when allocating buffers, so that the CPU, GPU, and peripherals can all access the same memory locations.

----
## Camera Notes
When the camera resolution is set to 640x480, it captures in 16-bit grayscale mode. This captures a total of 614400 bytes per frame as opposed to the 3686400 byes from color capture (1280x720 32-bit bayer). Using the lower resolution provides 3 important benefits: (1) Higher capture framerate (30 at 640x480 vs 15 at 1280x720) (2) Decreased preprocessing due to demosaicing and grayscale conversion. (3) Subsequent image processing algorithms will be dramatically faster since fewer pixels need to be processed.