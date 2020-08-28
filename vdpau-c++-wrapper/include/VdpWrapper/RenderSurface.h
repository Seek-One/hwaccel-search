#ifndef VW_RENDER_SURFACE_H
#define VW_RENDER_SURFACE_H

#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;

    /**
     * @brief DecodedSurface encapsules a VdpOutputSurface
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpOutputSurface. The class
     * respect the RAII programming idiom hence when a new object is created is owning
     * a VdpOutputSurface and when it is destroyed the VdpOutputSurface is freed.
     */
    class RenderSurface {
    public:
        /**
         * @brief Construct a empty RenderSurface
         *
         * @param device A reference to a valid Device
         * @param size The new surface size
         */
        RenderSurface(Device& device, const SizeU& size);
        /**
         * @brief Construct a new RenderSurface form an image
         *
         * The specified image is decompressed by OpenCV and the
         * new surface will be filled with this values
         *
         * @param device A reference to a valid Device
         * @param filename Filname of the source image
         */
        RenderSurface(Device& device, const std::string& filename);
        /**
         * @brief Destroy the Render Surface object
         */
        ~RenderSurface();

        RenderSurface(const RenderSurface&) = delete;
        /**
         * @brief Move constructor of RenderSurface
         *
         * @param other The old RenderSurface
         */
        RenderSurface(RenderSurface&& other);

        RenderSurface& operator=(const RenderSurface&) = delete;
        /**
         * @brief Move assignment operator of RenderSurface
         *
         * @param other The old RenderSurface
         * @return RenderSurface& The new RenderSurface
         */
        RenderSurface& operator=(RenderSurface&& other);

        /**
         * @brief Get the opaque VDPAU handle value
         *
         * @return VdpVideoSurface The opaque VDPAU handle value
         */
        VdpOutputSurface getVdpHandle() const;
        /**
         * @brief Set the Picture Order Count (POC)
         *
         * @param iPictureOrderCount New POC value
         */
        void setPictureOrderCount(int iPictureOrderCount);
        /**
         * @brief Get the Picture Order Count (POC)
         *
         * @return int The current POC value
         */
        int getPictureOrderCount() const;

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        VdpOutputSurface m_vdpOutputSurface;
        SizeU m_size;
        int m_iPictureOrderCount;
    };
}

#endif // VW_RENDER_SURFACE_H
