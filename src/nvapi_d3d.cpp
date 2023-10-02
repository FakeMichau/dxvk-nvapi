#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "dxvk/dxvk_interfaces.h"
#include "d3d12/nvapi_d3d12_device.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        static bool alreadyLogged = false;
        // Fake-implement with a dumb passthrough, though no other NvAPI entry points
        // we're likely to implement should care about the actual handle value.
        *pHandle = (NVDX_ObjectHandle)pResource;
        return Ok(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown* pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32* pdwHintValue) {
        static bool alreadyLogged = false;
        return NoImplementation(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_BeginResourceRendering(IUnknown* pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        static bool alreadyLogged = false;
        // Synchronisation hints for SLI...
        return Ok(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_EndResourceRendering(IUnknown* pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        static bool alreadyLogged = false;
        return Ok(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr || pSliState == nullptr)
            return InvalidArgument(n);

        switch (pSliState->version) {
            case NV_GET_CURRENT_SLI_STATE_VER1: {
                auto pSliStateV1 = reinterpret_cast<NV_GET_CURRENT_SLI_STATE_V1*>(pSliState);
                // Report that SLI is not available
                pSliStateV1->maxNumAFRGroups = 1;
                pSliStateV1->numAFRGroups = 1;
                pSliStateV1->currentAFRIndex = 0;
                pSliStateV1->nextFrameAFRIndex = 0;
                pSliStateV1->previousFrameAFRIndex = 0;
                pSliStateV1->bIsCurAFRGroupNew = false;
                break;
            }
            case NV_GET_CURRENT_SLI_STATE_VER2:
                // Report that SLI is not available
                pSliState->maxNumAFRGroups = 1;
                pSliState->numAFRGroups = 1;
                pSliState->currentAFRIndex = 0;
                pSliState->nextFrameAFRIndex = 0;
                pSliState->previousFrameAFRIndex = 0;
                pSliState->bIsCurAFRGroupNew = false;
                pSliState->numVRSLIGpus = 0;
                break;
            default:
                return IncompatibleStructVersion(n);
        }

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_ImplicitSLIControl(IMPLICIT_SLI_CONTROL implicitSLIControl) {
        constexpr auto n = __func__;

        if (implicitSLIControl == ENABLE_IMPLICIT_SLI)
            return Error(n); // No SLI with this implementation

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D1x_GetGraphicsCapabilities(IUnknown* pDevice, NvU32 structVersion, NV_D3D1x_GRAPHICS_CAPS* pGraphicsCaps) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        switch (structVersion) {
            case NV_D3D1x_GRAPHICS_CAPS_VER1: {
                auto pGraphicsCapsV1 = reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V1*>(pGraphicsCaps);
                *pGraphicsCapsV1 = {};
                pGraphicsCapsV1->bExclusiveScissorRectsSupported = 0;
                pGraphicsCapsV1->bVariablePixelRateShadingSupported = 0;
                break;
            }
            case NV_D3D1x_GRAPHICS_CAPS_VER2:
                *pGraphicsCaps = {};
                // bFastUAVClearSupported is reported mostly for the sake of DLSS.
                // All NVIDIA Vulkan drivers support this.
                pGraphicsCaps->bFastUAVClearSupported = 1;
                // dummy SM version number (unused by DLSS):
                pGraphicsCaps->majorSMVersion = 0;
                pGraphicsCaps->minorSMVersion = 0;
                pGraphicsCaps->bExclusiveScissorRectsSupported = 0;
                pGraphicsCaps->bVariablePixelRateShadingSupported = 0;
                break;
            default:
                return IncompatibleStructVersion(n);
        }

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_Sleep(IUnknown* pDevice) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;
        static bool alreadyLoggedNoLfx = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (!nvapiD3dInstance->IsReflexAvailable())
            return NoImplementation(n, alreadyLoggedNoLfx);

        nvapiD3dInstance->GetLfx2Instance()->Sleep();
        if (nvapiD3dInstance->IsReflexEnabled() && !nvapiD3dInstance->UseLatencyMarkers()) {
            auto lfx2Ext = nvapiD3dInstance->GetLfx2DeviceExt(pDevice);
            if (lfx2Ext.ptr()) {
                nvapiD3dInstance->GetLfx2Instance()->SleepImplicit(lfx2Ext);
            }
        }

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoLfx = false;

        Com<ID3DLfx2ExtDevice> context = nvapiD3dInstance->GetLfx2DeviceExt(pDevice);
        if (!context.ptr())
            return NoImplementation(n, alreadyLoggedNoLfx);

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pSetSleepModeParams->version != NV_SET_SLEEP_MODE_PARAMS_VER1)
            return IncompatibleStructVersion(n);

        if (!nvapiD3dInstance->IsReflexAvailable())
            return NoImplementation(n, alreadyLoggedNoLfx);

        nvapiD3dInstance->SetReflexEnabled(pSetSleepModeParams->bLowLatencyMode);
        return Ok(str::format(n, " (", pSetSleepModeParams->bLowLatencyMode ? str::format("Enabled/", pSetSleepModeParams->minimumIntervalUs, "us") : "Disabled", ")"));
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoLfx = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pGetSleepStatusParams->version != NV_GET_SLEEP_STATUS_PARAMS_VER1)
            return IncompatibleStructVersion(n);

        if (!nvapiD3dInstance->IsReflexAvailable())
            return NoImplementation(n, alreadyLoggedNoLfx);

        pGetSleepStatusParams->bLowLatencyMode = nvapiD3dInstance->IsReflexEnabled();
        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
        static bool alreadyLogged = false;
        return NoImplementation(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        static bool alreadyLogged = false;
        if (pSetLatencyMarkerParams->version != NV_LATENCY_MARKER_PARAMS_VER1)
            return IncompatibleStructVersion(__func__);

        Com<ID3DLfx2ExtDevice> context = nvapiD3dInstance->GetLfx2DeviceExt(pDev);
        Lfx2* lfx2 = nvapiD3dInstance->GetLfx2Instance();
        lfx2->Mark(pSetLatencyMarkerParams->frameID, pSetLatencyMarkerParams->markerType, context);

        return Ok(__func__, alreadyLogged);
    }
}
