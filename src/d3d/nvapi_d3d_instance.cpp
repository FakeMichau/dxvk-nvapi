#include "../util/util_log.h"
#include "nvapi_d3d_instance.h"
#include "util/util_env.h"

namespace dxvk {
    NvapiD3dInstance::NvapiD3dInstance(ResourceFactory& resourceFactory)
        : m_resourceFactory(resourceFactory) {
        auto latencyMarkersEnv = env::getEnvVariable("DXVK_NVAPI_USE_LATENCY_MARKERS");
        m_useLatencyMarkers = latencyMarkersEnv.empty() || latencyMarkersEnv != "0";
    }

    NvapiD3dInstance::~NvapiD3dInstance() = default;

    void NvapiD3dInstance::Initialize() {
        m_lfx = m_resourceFactory.CreateLfx();
        if (m_lfx->IsAvailable())
            log::write("LatencyFleX loaded and initialized successfully");
    }

    bool NvapiD3dInstance::IsReflexAvailable() {
        return m_lfx->IsAvailable();
    }

    Lfx2* NvapiD3dInstance::GetLfx2Instance() const {
        return &*m_lfx;
    }

    bool NvapiD3dInstance::IsReflexEnabled() const {
        return m_isLfxEnabled;
    }

    void NvapiD3dInstance::SetReflexEnabled(bool value) {
        m_isLfxEnabled = value;
        m_lfx->SetEnabled(value);
    }

    bool NvapiD3dInstance::UseLatencyMarkers() const {
        return m_useLatencyMarkers;
    }

    Com<ID3DLfx2ExtDevice> NvapiD3dInstance::GetLfx2DeviceExt(IUnknown* pDevice) {
        static std::mutex map_mutex;
        static std::unordered_map<IUnknown*, ID3DLfx2ExtDevice*> cacheMap;

        std::scoped_lock lock(map_mutex);
        auto it = cacheMap.find(pDevice);
        if (it != cacheMap.end())
            return it->second;
        Com<ID3DLfx2ExtDevice> lfx2Device;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&lfx2Device))))
            lfx2Device = nullptr;

        cacheMap.emplace(pDevice, lfx2Device.ptr());
        return lfx2Device;
    }
}