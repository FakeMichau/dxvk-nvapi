#pragma once

#include "../inc/latencyflex2.h"
#include "../nvapi_private.h"
#include "dxvk/dxvk_interfaces.h"
#include "util/com_pointer.h"
#include "vkd3d-proton/vkd3d-proton_interfaces.h"

namespace dxvk {
    class Lfx2 {
      public:
        Lfx2();
        virtual ~Lfx2();

        [[nodiscard]] virtual bool IsAvailable() const;
        virtual void Sleep();
        virtual void SleepImplicit(Com<ID3DLfx2ExtDevice>& extDevice);
        virtual void Mark(uint64_t frame_id, NV_LATENCY_MARKER_TYPE type, Com<ID3DLfx2ExtDevice>& extDevice);
        virtual void SetEnabled(bool enabled);

      private:
#define DECLARE_PFN(x) \
    decltype(&::lfx2##x) x {}

        DECLARE_PFN(ContextCreate);
        DECLARE_PFN(ContextAddRef);
        DECLARE_PFN(ContextRelease);
        DECLARE_PFN(FrameCreate);
        DECLARE_PFN(FrameAddRef);
        DECLARE_PFN(FrameRelease);
        DECLARE_PFN(MarkSection);
        DECLARE_PFN(SleepUntil);
        DECLARE_PFN(TimestampNow);
#ifdef _WIN32
        DECLARE_PFN(TimestampFromQpc);
#endif
        DECLARE_PFN(ImplicitContextCreate);
        DECLARE_PFN(ImplicitContextRelease);
        DECLARE_PFN(ImplicitContextReset);
        DECLARE_PFN(FrameCreateImplicit);
        DECLARE_PFN(FrameDequeueImplicit);

#undef DECLARE_PFN

        template <typename T>
        T GetProcAddress(const char* name);

        void EnsureFrame();

        std::mutex m_frameMapMutex;
        std::map<uint64_t, lfx2Frame*> m_frameMap;
        std::unordered_map<uint64_t, uint32_t> m_callsExpectedByFrame;

        HMODULE m_lfxModule{};
        lfx2Context* m_lfxContext{};
        lfx2Frame* m_nextFrame{};

        static constexpr uint64_t kMaxInflightFrames = 64;
    };
}