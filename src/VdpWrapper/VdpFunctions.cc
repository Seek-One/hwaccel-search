#include <VdpWrapper/VdpFunctions.h>

#include <stdexcept>
#include <string>

#include <VdpWrapper/Device.h>

namespace vw {
    VdpFunctions::VdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress)
    : getInformationString(nullptr)
    , deviceDestroy(nullptr)
    , outputSurfaceCreate(nullptr)
    , outputSurfaceDestroy(nullptr)
    , outputSurfacePutBitsNative(nullptr)
    , outputSurfaceGetParameters(nullptr)
    , presentationQueueTargetCreateX11(nullptr)
    , presentationQueueTargetDestroy(nullptr)
    , presentationQueueCreate(nullptr)
    , presentationQueueDestroy(nullptr)
    , presentationQueueSetBackgroundColor(nullptr)
    , presentationQueueGetTime(nullptr)
    , presentationQueueDisplay(nullptr)
    , m_pGetProcAddress(pGetProcAddress)
    , m_pGetErrorString(nullptr) {
        storeFunction(vdpDevice, VDP_FUNC_ID_GET_ERROR_STRING);
        storeFunction(vdpDevice, VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(vdpDevice, VDP_FUNC_ID_DEVICE_DESTROY);
        storeFunction(vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_CREATE);
        storeFunction(vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY);
        storeFunction(vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE);
        storeFunction(vdpDevice, VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME);
        storeFunction(vdpDevice, VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY);
    }

    std::string VdpFunctions::getErrorString(VdpStatus status) const {
        return std::string(m_pGetErrorString(status));
    }

    void VdpFunctions::throwExceptionOnFail(VdpStatus vdpStatus, const std::string& message) {
        if (vdpStatus != VDP_STATUS_OK) {
            auto szError = getErrorString(vdpStatus);
            throw std::runtime_error(
                message + "\n" +
                "\tError code: " + std::to_string(vdpStatus) + "\n" +
                "\tError string: " + szError);
        }
    }

    void VdpFunctions::storeFunction(VdpDevice& vdpDevice, VdpFuncId functionID) {
        // Get the callback
        void* func = nullptr;
        VdpStatus vdpStatus = m_pGetProcAddress(vdpDevice, functionID, &func);
        throwExceptionOnFail(vdpStatus, "[VdpFunctions] Error getting the #" + std::to_string(functionID) + " callback");

        // Store the callback
        switch (functionID) {
            case VDP_FUNC_ID_GET_ERROR_STRING:
                m_pGetErrorString = reinterpret_cast<VdpGetErrorString*>(func);
                break;

            case VDP_FUNC_ID_GET_INFORMATION_STRING:
                getInformationString = reinterpret_cast<VdpGetInformationString*>(func);
                break;

            case VDP_FUNC_ID_DEVICE_DESTROY:
                deviceDestroy = reinterpret_cast<VdpDeviceDestroy*>(func);
                break;

            case VDP_FUNC_ID_OUTPUT_SURFACE_CREATE:
                outputSurfaceCreate = reinterpret_cast<VdpOutputSurfaceCreate*>(func);
                break;

            case VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY:
                outputSurfaceDestroy = reinterpret_cast<VdpOutputSurfaceDestroy*>(func);
                break;

            case VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE:
                outputSurfacePutBitsNative = reinterpret_cast<VdpOutputSurfacePutBitsNative*>(func);
                break;

            case VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS:
                outputSurfaceGetParameters = reinterpret_cast<VdpOutputSurfaceGetParameters*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11:
                presentationQueueTargetCreateX11 = reinterpret_cast<VdpPresentationQueueTargetCreateX11*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY:
                presentationQueueTargetDestroy = reinterpret_cast<VdpPresentationQueueTargetDestroy*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE:
                presentationQueueCreate = reinterpret_cast<VdpPresentationQueueCreate*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY:
                presentationQueueDestroy = reinterpret_cast<VdpPresentationQueueDestroy*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR:
                presentationQueueSetBackgroundColor = reinterpret_cast<VdpPresentationQueueSetBackgroundColor*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME:
                presentationQueueGetTime = reinterpret_cast<VdpPresentationQueueGetTime*>(func);
                break;

            case VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY:
                presentationQueueDisplay = reinterpret_cast<VdpPresentationQueueDisplay*>(func);
                break;

            default:
                break;
        }
    }

    VdpFunctionsInstance::VdpFunctionsInstance()
    : m_pVdpFunction(nullptr) {

    }

    VdpFunctionsInstance::~VdpFunctionsInstance() {
        delete m_pVdpFunction;
    }

    void VdpFunctionsInstance::create(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress) {
        if (m_pVdpFunction != nullptr) {
            throw std::runtime_error("The VDPAU functions are already initialized");
        }

        m_pVdpFunction = new VdpFunctions(vdpDevice, pGetProcAddress);
    }

    void VdpFunctionsInstance::dispose() {
        delete m_pVdpFunction;
        m_pVdpFunction = nullptr;
    }

    VdpFunctions* VdpFunctionsInstance::operator()() {
        if (m_pVdpFunction == nullptr) {
            throw std::runtime_error("The VDPAU functions are not initialized. You must first create a vw::Device.");
        }

        return m_pVdpFunction;
    }

    VdpFunctionsInstance gVdpFunctionsInstance;
}
