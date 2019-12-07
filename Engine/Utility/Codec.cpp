// ------------------------------------ //
#include "Codec.h"

#include "Utility/MatroskaParser.h"

#include "Exceptions.h"

#include "YUVToRGB.h"

// AV1
#include "aom/aom_decoder.h"
#include "aom/aomdx.h"

// Vorbis
#include "vorbis/codec.h"

// Opus
#include "opus/opus.h"

#include <array>
#include <cmath>
#include <cstring>

using namespace Leviathan;
// ------------------------------------ //
// DecodedFrame
DLLEXPORT DecodedFrame::DecodedFrame(aom_image* img)
{
    Image image;

    image.Width = img->w;
    image.Height = img->h;
    image.BitDepth = img->bit_depth;
    // img->bps

    image.ConvertImage = [img, image](uint8_t* target, size_t targetlength,
                             Image::IMAGE_TARGET_FORMAT format) -> bool {
        if(format != Image::IMAGE_TARGET_FORMAT::RGBA)
            return false;

        const auto targetStride = image.Width * 4;

        if(targetlength != targetStride * image.Height)
            return false;

        std::array<int, 3> planeIndices = {0, 1, 2};

        // Depending on the img->fmt the Y, U and V planes can be in different order
        if(img->fmt & AOM_IMG_FMT_UV_FLIP) {
            planeIndices = {0, 2, 1};
        }

        if(img->fmt & AOM_IMG_FMT_HAS_ALPHA) {
            LOG_WARNING("AOM alpha channel handling is not implemented");
        }

        if(img->fmt == AOM_IMG_FMT_AOMYV12 || img->fmt == AOM_IMG_FMT_AOMI420) {
            LOG_WARNING("AOM colourspace handling not implemented");
        }

        // Setup plane data for the conversion call
        std::array<const uint8_t*, 3> planes;
        std::array<int, 3> strides;
        std::array<std::tuple<int, int>, 3> planeSizes;

        for(int plane = 0; plane < 3; ++plane) {
            const auto aomPlane = planeIndices[plane];
            planes[plane] = img->planes[aomPlane];
            strides[plane] = img->stride[aomPlane];
            planeSizes[plane] = {
                aom_img_plane_width(img, aomPlane), aom_img_plane_height(img, aomPlane)};
        }

        return YUVToRGBA(planes, strides, planeSizes, img->fmt & AOM_IMG_FMT_HIGHBITDEPTH,
            target, image.Width, image.Height);
    };

    TypeSpecificData = image;
}
// ------------------------------------ //
DLLEXPORT DecodedFrame::DecodedFrame(const float* const* pcm, int samples, int channels)
{
    Sound sound;

    sound.Samples = samples;
    sound.Channels = channels;

    sound.ConvertSamples = [pcm, sound](uint8_t* target, size_t targetlength,
                               Sound::SOUND_TARGET_FORMAT format) -> bool {
        if(format != Sound::SOUND_TARGET_FORMAT::INTERLEAVED_INT16)
            return false;

        if(targetlength != sizeof(int16_t) * sound.Samples * sound.Channels)
            return false;

        bool clipped = false;

        // Converts floats to 16 bit signed ints in an interleaved form
        for(int channel = 0; channel < sound.Channels; ++channel) {
            int16_t* writePtr = reinterpret_cast<int16_t*>(target) + channel;

            const float* monoSamples = pcm[channel];

            for(int sample = 0; sample < sound.Samples; ++sample) {
                // The vorbis decoding sample uses dithering here, not sure what that does, but
                // we skip it here
                int val = std::floor(monoSamples[sample] * 32767.f + .5f);

                // Protect against clipping, skipping this might make things faster
                if(val > std::numeric_limits<int16_t>::max()) {
                    val = std::numeric_limits<int16_t>::max();
                    clipped = true;
                }
                if(val < std::numeric_limits<int16_t>::min()) {
                    val = std::numeric_limits<int16_t>::min();
                    clipped = true;
                }

                *writePtr = val;
                writePtr += sound.Channels;
            }
        }

        if(clipped)
            LOG_WARNING("DecodedFrame: detected audio clipping on conversion");

        return true;
    };

    TypeSpecificData = sound;
}
// ------------------------------------ //
// AV1Codec::Implementation
struct AV1Codec::Implementation {
    Implementation() : Codec(std::make_unique<aom_codec_ctx_t>()) {}

    ~Implementation()
    {
        CloseCodec();
    }

    void CloseCodec()
    {
        if(Codec) {
            auto error = aom_codec_destroy(Codec.get());

            if(error) {
                LOG_ERROR(std::string("AV1Codec: codec closing failed: ") +
                          aom_codec_error(Codec.get()));
            }

            Codec.reset();
        }
    }

    std::unique_ptr<aom_codec_ctx_t> Codec;
    aom_codec_iter_t FrameIter = nullptr;
};
// ------------------------------------ //
// AV1Codec
DLLEXPORT AV1Codec::AV1Codec() : Pimpl(std::make_unique<Implementation>())
{
    aom_codec_iface_t* codecInterface = aom_codec_av1_dx();
    aom_codec_dec_cfg_t config;
    std::memset(&config, 0, sizeof(config));
    // TODO: allow adding threads (it might help with decode performance)
    config.threads = 2;
    // We prefer 8 bit output (but we can also handle 16 bit output correctly)
    // in the hopes that it is faster
    config.allow_lowbitdepth = 1;

    auto error = aom_codec_dec_init(Pimpl->Codec.get(), codecInterface, &config, 0);

    if(error) {
        throw InvalidState(
            std::string("AV1 codec creation failed: ") + aom_codec_error(Pimpl->Codec.get()));
    }

    LOG_INFO(std::string("AV1Codec: using decoder interface: ") +
             aom_codec_iface_name(codecInterface));
}

DLLEXPORT AV1Codec::~AV1Codec() {}
// ------------------------------------ //
DLLEXPORT bool AV1Codec::FeedRawFrame(const uint8_t* data, size_t length)
{
    if(aom_codec_decode(Pimpl->Codec.get(), data, length, nullptr)) {
        // Failed
        LOG_ERROR(std::string("AV1Codec: decoding raw frame failed: ") +
                  aom_codec_error(Pimpl->Codec.get()));
        return false;
    }

    Pimpl->FrameIter = nullptr;

    return true;
}

DLLEXPORT void AV1Codec::ReceiveDecodedFrames(FrameCallback callback)
{
    while(true) {
        aom_image_t* img = aom_codec_get_frame(Pimpl->Codec.get(), &Pimpl->FrameIter);
        if(img == nullptr) {
            break;
        }

        if(!callback(DecodedFrame(img)))
            break;
    }
}
// ------------------------------------ //
// VorbisCodec::Implementation
struct VorbisCodec::Implementation {
    Implementation()
    {
        vorbis_info_init(&VorbisInfo);
        vorbis_comment_init(&VorbisComment);
    }

    ~Implementation()
    {
        vorbis_info_clear(&VorbisInfo);
        vorbis_comment_clear(&VorbisComment);

        if(Initialized) {
            vorbis_block_clear(&vb);
            vorbis_dsp_clear(&vd);
        }
    }

    vorbis_info VorbisInfo;
    vorbis_comment VorbisComment;

    bool Initialized = false;

    // decoder work space
    vorbis_dsp_state vd;
    vorbis_block vb;

    // Stream info
    int Channels;
    long Frequency;
};
// ------------------------------------ //
// VorbisCodec
DLLEXPORT VorbisCodec::VorbisCodec(const uint8_t* codecprivatedata, size_t datalength) :
    Pimpl(std::make_unique<Implementation>())
{
    // First split the private data into the separate packets that are needed
    const auto packets =
        MatroskaParser::SplitVorbisPrivateSetupData(codecprivatedata, datalength);

    if(packets.empty())
        throw InvalidArgument("Failed to split the vorbis codec private data to the distinct "
                              "initialization packets");

    bool first = true;

    // Then decode the private data
    for(const auto [ptr, size] : packets) {
        ogg_packet op;
        std::memset(&op, 0, sizeof(op));

        op.packet = reinterpret_cast<unsigned char*>(const_cast<uint8_t*>(ptr));
        op.bytes = size;

        // Needs to mark the first packet otherwise vorbis decoding just gives up
        if(first)
            op.b_o_s = 1;

        first = false;

        if(int error =
                vorbis_synthesis_headerin(&Pimpl->VorbisInfo, &Pimpl->VorbisComment, &op);
            error < 0) {
            throw InvalidArgument(
                "vorbis initialization packet processing failed, error code: " +
                std::to_string(error));
        }
    }

    Pimpl->Channels = Pimpl->VorbisInfo.channels;
    Pimpl->Frequency = Pimpl->VorbisInfo.rate;

    // Could store some of this info
    // char** ptr = Pimpl->VorbisComment.user_comments;
    // Pimpl->VorbisComment.vendor;

    if(vorbis_synthesis_init(&Pimpl->vd, &Pimpl->VorbisInfo) != 0)
        throw InvalidState("vorbis synthesis init failed");

    vorbis_block_init(&Pimpl->vd, &Pimpl->vb);
    Pimpl->Initialized = true;
}

DLLEXPORT VorbisCodec::~VorbisCodec() {}
// ------------------------------------ //
DLLEXPORT bool VorbisCodec::FeedRawFrame(const uint8_t* data, size_t length)
{
    ogg_packet op;
    std::memset(&op, 0, sizeof(op));

    op.packet = reinterpret_cast<unsigned char*>(const_cast<uint8_t*>(data));
    op.bytes = length;

    // if(!data || length == 0) {
    //     op.e_o_s = 1;
    // }

    if(vorbis_synthesis(&Pimpl->vb, &op) != 0) {
        LOG_ERROR("VorbisCodec: vorbis synthesis failed");
        return false;
    }

    if(vorbis_synthesis_blockin(&Pimpl->vd, &Pimpl->vb) != 0) {
        LOG_ERROR("VorbisCodec: vorbis_synthesis_blockin failed");
        return false;
    }

    return true;
}

DLLEXPORT void VorbisCodec::ReceiveDecodedFrames(FrameCallback callback)
{
    float** pcm;
    int samples;

    while((samples = vorbis_synthesis_pcmout(&Pimpl->vd, &pcm)) > 0) {

        bool continueLoop = callback(DecodedFrame(pcm, samples, Pimpl->Channels));

        // Mark all samples as read
        vorbis_synthesis_read(&Pimpl->vd, samples);

        if(!continueLoop)
            break;
    }
}
// ------------------------------------ //
// OpusCodec::Implementation
struct OpusCodec::Implementation {};
// ------------------------------------ //
// OpusCodec
DLLEXPORT OpusCodec::OpusCodec() : Pimpl(std::make_unique<Implementation>()) {}

DLLEXPORT OpusCodec::~OpusCodec() {}
