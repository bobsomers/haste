#include "main.h"

// Beginning of GPU Architecture definitions
inline int ConvertSMVer2Cores(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
    typedef struct {
        int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
        int Cores;
    } sSMtoCores;

    sSMtoCores nGpuArchCoresPerSM[] =
    { { 0x10,  8 },
      { 0x11,  8 },
      { 0x12,  8 },
      { 0x13,  8 },
      { 0x20, 32 },
      { 0x21, 48 },
      {   -1, -1 }
    };

    int index = 0;
    while (nGpuArchCoresPerSM[index].SM != -1) {
        if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor) ) {
            return nGpuArchCoresPerSM[index].Cores;
        }
        index++;
    }
    printf("MapSMtoCores undefined SMversion %d.%d!\n", major, minor);
    return -1;
}
// end of GPU Architecture definitions

int main(int argc, char *argv[]) {
    // check arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <main.lua>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // create lua state
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    
    // load the script
    luaL_dofile(L, argv[1]);

    // call saySomething()
    luabind::call_function<void>(L, "saySomething", "This string is in C++");
    
    printf("Checking for CUDA devices...");

    int deviceCount = 0;
    if (cudaGetDeviceCount(&deviceCount) != cudaSuccess) {
        fprintf(stderr, "cudaGetDeviceCount failed!\n");
        exit(EXIT_FAILURE);
    }

    printf(" found %d!\n", deviceCount);

    int totalCoreCount = 0;
    if (deviceCount > 0) {
        for (int i = 0; i < deviceCount; i++) {
            cudaDeviceProp deviceProp;
            cudaGetDeviceProperties(&deviceProp, i);
            
            printf("\n===== #%d: %s =====\n", i, deviceProp.name);

            int driverVersion, runtimeVersion;
            cudaDriverGetVersion(&driverVersion);
            cudaRuntimeGetVersion(&runtimeVersion);
            printf("\tDriver/Runtime Versions: %d.%d/%d.%d\n", driverVersion / 1000, driverVersion % 100, runtimeVersion / 1000, runtimeVersion % 100);

            printf("\tCompute Capability: %d.%d\n", deviceProp.major, deviceProp.minor);

            int coreCount = ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount;
            printf("\tCore Count: %d\n", coreCount);
            totalCoreCount += coreCount;

            printf("\tClock Speed: %.2f GHz\n", deviceProp.clockRate * 1e-6f);

            float mem = deviceProp.totalGlobalMem / 1024.0 / 1024.0;
            printf("\tAvailable Memory: %.2f MB\n", mem);
        }
    }

    printf("\nTotal CUDA Cores: %d\n", totalCoreCount);

    return EXIT_SUCCESS;
}
