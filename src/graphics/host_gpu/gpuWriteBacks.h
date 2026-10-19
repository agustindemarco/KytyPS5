#ifndef EMULATOR_SRC_GRAPHICS_HOST_GPU_GPUWRITEBACKS_H_
#define EMULATOR_SRC_GRAPHICS_HOST_GPU_GPUWRITEBACKS_H_

#include <cstdint>
#include <string>

// Debugging the guest memory the GPU writes: the last write-backs of GPU contents into guest memory (buffer and image downloads), so that a crash
// can tell whether the memory a register points into came from the GPU, and when.
namespace Libs::Graphics::GpuWriteBacks {

enum class Source : uint8_t { BufferDownload, BufferDownloadDeferred, ImageDownload, ImageFill };

void Record(uint64_t address, uint64_t size, Source source) noexcept;

// The latest of the recorded write-backs that covers the address, as one line ("none of the last N write-backs" when none does). Does not wait for a
// write-back being recorded (the crash context calls it).
[[nodiscard]] std::string Describe(uint64_t address);

} // namespace Libs::Graphics::GpuWriteBacks

#endif // EMULATOR_SRC_GRAPHICS_HOST_GPU_GPUWRITEBACKS_H_
