# Danger Ranger

----
## What is the Danger Ranger?
The danger ranger is an autonomous drone that performs onboard image processing to detect targets from the air. Images which contain points of interest (i.e. trained features) are sent over radio to a ground station for human inspection. In case the radio link goes down, capture and detection will still take place. This data can be collected once the drone is retrieved. 

----
## How Does it Work?
1. Raw camera input (RGBG) is recieved using V4L2.
2. Demosaicing and grayscale conversion is performed using OpenCL (Mali T628 GPU). This step reduces the amount of data to process by a factor of 4.
3. Keypoints are extracted using an OpenCL adapation of the OpenCV CUDA FAST detector.
4. An OpenCL implementation of the FREAK binary descriptor is used to describe each keypoint found.
5. To detect targets of interest, keypoint descriptors are matched to existing descriptors from trained data. Due to the fact that our aerial images are very similar in appearance, this step performs a very weak form of classification.
6. While keypoints are extracted and described on the GPU, the CPU uses libjpeg-turbo (NEON Accelerated) to compress the image to ~2% of its raw size.
7. From here, zlib is used to perform lossless compression for a further 15% decrease in size. Note: The lossy jpeg compression level greatly influences the effectiveness of lossless zlib compression. The entire image pipeline compresses 1228800 bytes of data into approximately ~7000 bytes (<1% of RAW).
8. Finally, the image buffer and any detection coordinates are sent over the air using the radio link. This is the major bottleneck in the process.

----
## Build Requirements
### Hardware
1. OpenCL Enabled GPU
2. Camera with RAW video input

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

