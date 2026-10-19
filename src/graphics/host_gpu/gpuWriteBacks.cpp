#include "graphics/host_gpu/gpuWriteBacks.h"

#include <array>
#include <chrono>
#include <fmt/format.h>
#include <mutex>

namespace Libs::Graphics::GpuWriteBacks {

namespace {

struct Entry {
	uint64_t address = 0;
	uint64_t size    = 0;
	int64_t  time_ms = 0;
	Source   source  = Source::BufferDownload;
};

constexpr size_t kEntries = 4096;

std::mutex                   g_mutex;
std::array<Entry, kEntries>  g_entries;
uint64_t                     g_count = 0;

int64_t NowMs() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

const char* SourceName(Source source) {
	switch (source) {
		case Source::BufferDownload: return "buffer download";
		case Source::BufferDownloadDeferred: return "deferred buffer download (garbage collection)";
		case Source::ImageDownload: return "image download";
		case Source::ImageFill: return "image fill (0xff)";
	}
	return "?";
}

} // namespace

void Record(uint64_t address, uint64_t size, Source source) noexcept {
	const auto      now = NowMs();
	std::lock_guard lock(g_mutex);
	g_entries[g_count % kEntries] = {address, size, now, source};
	g_count++;
}

std::string Describe(uint64_t address) {
	std::unique_lock lock(g_mutex, std::try_to_lock);
	if (!lock.owns_lock()) {
		return "GPU write-backs: being recorded";
	}
	const uint64_t recorded = g_count < kEntries ? g_count : kEntries;
	for (uint64_t i = 0; i < recorded; i++) {
		const auto e = g_entries[(g_count - 1 - i) % kEntries];
		if (e.address <= address && address < e.address + e.size) {
			return fmt::format("last GPU write-back here: {} of 0x{:x}..0x{:x}, {} ms ago, {} write-backs since", SourceName(e.source), e.address,
			                   e.address + e.size, NowMs() - e.time_ms, i);
		}
	}
	return fmt::format("none of the last {} GPU write-backs wrote here", recorded);
}

} // namespace Libs::Graphics::GpuWriteBacks
