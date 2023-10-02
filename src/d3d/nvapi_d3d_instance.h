#pragma once

#include "../nvapi_private.h"
#include "../resource_factory.h"

namespace dxvk {
    class NvapiD3dInstance {
      public:
        explicit NvapiD3dInstance(ResourceFactory& resourceFactory);
        ~NvapiD3dInstance();

        void Initialize();
        [[nodiscard]] bool IsReflexAvailable();
        [[nodiscard]] bool IsReflexEnabled() const;
        void SetReflexEnabled(bool value);
        [[nodiscard]] bool UseLatencyMarkers() const;
        [[nodiscard]] Lfx2* GetLfx2Instance() const;

        Com<ID3DLfx2ExtDevice> GetLfx2DeviceExt(IUnknown* pDevice);

      private:
        ResourceFactory& m_resourceFactory;
        std::unique_ptr<Lfx2> m_lfx;
        bool m_isLfxEnabled = false;
        bool m_useLatencyMarkers;
    };
}