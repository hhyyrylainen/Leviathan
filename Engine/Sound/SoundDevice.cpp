// ------------------------------------ //
#include "SoundDevice.h"

#include "AudioBuffer.h"

#include "Application/AppDefine.h"
#include "Common/StringOperations.h"
#include "Engine.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "TimeIncludes.h"

#include "alure2.h"

#include <boost/filesystem.hpp>

#include <algorithm>
using namespace Leviathan;
using namespace Leviathan::Sound;
// ------------------------------------ //

class SoundMessageHandler : public alure::MessageHandler {
public:
    virtual void deviceDisconnected(alure::Device device) noexcept override
    {
        LOG_INFO("[SOUND] Device disconnected: " + device.getName());
    }

    virtual void sourceForceStopped(alure::Source source) noexcept override
    {
        LOG_WARNING("[SOUND] Source force stopped.");
    }

    virtual alure::String resourceNotFound(alure::StringView name) noexcept override
    {
        LOG_ERROR(std::string("[SOUND] resource not found: ") + name);
        return "";
    }
};

struct SoundDevice::Implementation {

    alure::DeviceManager DeviceManager;
    alure::Device Device;
    alure::Context Context;
    alure::Listener Listener;
    Float3 PreviousPosition = {0, 0, 0};

    //! List of audio sources that this class will close on tick if they have stopped
    std::vector<AudioSource::pointer> HandledAudioSources;

    //! Cache of buffers for short sounds
    std::unordered_map<std::string, std::tuple<AudioBuffer::pointer, int64_t>> BufferCache;
};


// ------------------------------------ //
SoundDevice::SoundDevice() : Pimpl(std::make_unique<Implementation>()) {}
SoundDevice::~SoundDevice()
{
    Release();
    Pimpl.reset();
}
// ------------------------------------ //
bool SoundDevice::Init(bool simulatesound /*= false*/)
{
    if(simulatesound) {
        LOG_WARNING("SoundDevice: simulating not having a playing device");
        return true;
    }

    Pimpl->DeviceManager = alure::DeviceManager::getInstance();

    const auto defaultChoice =
        Pimpl->DeviceManager.defaultDeviceName(alure::DefaultDeviceType::Full);

    auto devices = Pimpl->DeviceManager.enumerate(alure::DeviceEnumeration::Full);

    if(defaultChoice.empty()) {

        LOG_ERROR("SoundDevice: no default sound device detected");
        return false;
    }

    if(devices.empty()) {

        LOG_ERROR("SoundDevice: no audio devices detected");
        return false;
    }

    std::string selectedDevice;
    // There's no print error here if missing to make tests run
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
        Engine::Get()->GetDefinition()->GetValues(), "AudioDevice", selectedDevice, "default");

    if(selectedDevice == "default") {

        selectedDevice = defaultChoice;

    } else if(std::find(devices.begin(), devices.end(), selectedDevice) == devices.end()) {
        LOG_ERROR("SoundDevice: selected audio device \"" + selectedDevice +
                  "\" doesn't exists. Using default");
        selectedDevice = defaultChoice;
    }

    LOG_INFO("SoundDevice: Initializing sound with device: " + selectedDevice);

    std::stringstream sstream;

    sstream << "Start of audio system information:\n"
            << "// ------------------------------------ //\n";

    auto defaultDeviceName =
        Pimpl->DeviceManager.defaultDeviceName(alure::DefaultDeviceType::Basic);
    devices = Pimpl->DeviceManager.enumerate(alure::DeviceEnumeration::Basic);

    sstream << "Available basic devices:\n";
    for(const auto& name : devices) {
        sstream << "\t> " << name;
        if(name == defaultDeviceName)
            sstream << " [DEFAULT]";
        sstream << "\n";
    }

    sstream << "\n";

    devices = Pimpl->DeviceManager.enumerate(alure::DeviceEnumeration::Full);
    defaultDeviceName = Pimpl->DeviceManager.defaultDeviceName(alure::DefaultDeviceType::Full);

    sstream << "Available devices:\n";
    for(const auto& name : devices) {
        sstream << "\t> " << name;
        if(name == defaultDeviceName)
            sstream << " [DEFAULT]";
        sstream << "\n";
    }
    sstream << "\n";

    devices = Pimpl->DeviceManager.enumerate(alure::DeviceEnumeration::Capture);
    defaultDeviceName =
        Pimpl->DeviceManager.defaultDeviceName(alure::DefaultDeviceType::Capture);
    sstream << "Available capture devices:\n";
    for(const auto& name : devices) {
        sstream << "\t> " << name;
        if(name == defaultDeviceName)
            sstream << " [DEFAULT]";
        sstream << "\n";
    }
    sstream << "\n";

    try {
        Pimpl->Device = Pimpl->DeviceManager.openPlayback(selectedDevice);
    } catch(const std::exception& e) {
        LOG_INFO(sstream.str());
        LOG_ERROR("SoundDevice: opening playback failed: " + std::string(e.what()));
        return false;
    }

    sstream << "Info for device \"" << Pimpl->Device.getName(alure::PlaybackName::Full)
            << "\":" << std::endl;
    auto version = Pimpl->Device.getALCVersion();
    sstream << "ALC version: " << version.getMajor() << "." << version.getMinor() << "\n";
    version = Pimpl->Device.getEFXVersion();
    if(!version.isZero()) {
        sstream << "EFX version: " << version.getMajor() << "." << version.getMinor() << "\n";
        sstream << "Max auxiliary sends: " << Pimpl->Device.getMaxAuxiliarySends() << "\n";
    } else
        sstream << "EFX not supported"
                << "\n";

    // TODO: extensions
    // Pimpl->Device.queryExtension(const String &name)

    sstream << "// ------------------------------------ //";

    LOG_INFO(sstream.str());

    try {
        Pimpl->Context = Pimpl->Device.createContext();

        if(!Pimpl->Context)
            throw Exception("returned context is null");

    } catch(const std::exception& e) {
        LOG_ERROR("SoundDevice: opening context failed: " + std::string(e.what()));
        return false;
    }

    alure::Context::MakeCurrent(Pimpl->Context);
    Pimpl->Listener = Pimpl->Context.getListener();

    Pimpl->Context.setMessageHandler(alure::MakeShared<SoundMessageHandler>());

    // Setup global volume //
    SetGlobalVolume(1.f);

    Pimpl->Listener.setPosition({0, 0, 0});
    Pimpl->Listener.set3DParameters(
        // Pos
        {0, 0, 0},
        // Velocity
        {0, 0, 0},
        // Orientation, at and up
        {{0, 0, 0}, {0, 1, 0}});

    return true;
}

void SoundDevice::Release()
{
    if(!Pimpl->Device)
        return;

    Pimpl->HandledAudioSources.clear();
    Pimpl->BufferCache.clear();

    alure::Context::MakeCurrent(nullptr);
    Pimpl->Listener = nullptr;
    Pimpl->Context.destroy();
    Pimpl->Device.close();

    Pimpl->Device = nullptr;
}
// ------------------------------------ //
void SoundDevice::Tick(int PassedMs)
{
    ElapsedSinceLastClean += PassedMs;

    Pimpl->Context.update();

    if(ElapsedSinceLastClean > 200) {
        ElapsedSinceLastClean = 0;

        for(auto iter = Pimpl->HandledAudioSources.begin();
            iter != Pimpl->HandledAudioSources.end();) {

            if(!(*iter)->IsPlaying()) {

                iter = Pimpl->HandledAudioSources.erase(iter);
            } else {
                ++iter;
            }
        }

        const auto now = Time::GetTimeMs64();

        // Clear cache entries
        for(auto iter = Pimpl->BufferCache.begin(); iter != Pimpl->BufferCache.end();) {

            if(std::get<1>(iter->second) - now > CacheSoundEffectMilliseconds) {
                // Remove from cache

                iter = Pimpl->BufferCache.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}

DLLEXPORT void SoundDevice::SetSoundListenerPosition(
    const Float3& pos, const Float4& orientation)
{
    if(!Pimpl->Listener)
        return;

    bs::Quaternion quaternion(orientation);

    bs::Radian angle;
    bs::Vector3 direction;

    // TODO: does this do the right thing?
    quaternion.toAxisAngle(direction, angle); // toAngleAxis(angle, direction);

    alure::Vector3 at(0, 0, 0);

    // TODO: better velocity calculation
    const auto temp = pos - Pimpl->PreviousPosition;
    alure::Vector3 velocity(temp.X, temp.Y, temp.Z);

    Pimpl->Listener.set3DParameters(
        {pos.X, pos.Y, pos.Z}, velocity, {at, {direction.x, direction.y, direction.z}});

    Pimpl->PreviousPosition = pos;
}

DLLEXPORT void SoundDevice::SetGlobalVolume(float vol)
{
    vol = std::clamp(vol, 0.f, 1.f);

    Pimpl->Listener.setGain(vol);
}
// ------------------------------------ //
DLLEXPORT void SoundDevice::Play2DSoundEffect(const std::string& filename)
{
    Engine::Get()->AssertIfNotMainThread();

    const auto buffer = GetBufferFromFile(filename);
    if(!buffer)
        return;

    const auto source = GetAudioSource();

    if(!source)
        return;

    source->Play2D(buffer);

    Engine::Get()->RunOnMainThread([=]() { this->BabysitAudio(source); });
}

DLLEXPORT AudioSource::pointer SoundDevice::Play2DSound(
    const std::string& filename, bool looping)
{
    Engine::Get()->AssertIfNotMainThread();

    const auto buffer = GetBufferFromFile(filename);
    if(!buffer)
        return nullptr;

    const auto source = GetAudioSource();

    if(!source)
        return nullptr;

    source->Play2D(buffer);

    if(looping)
        source->SetLooping(true);
    return source;
}

DLLEXPORT AudioSource::pointer SoundDevice::CreateProceduralSound(
    const ProceduralSoundData::pointer& data, size_t chunksize /*= 56000*/,
    size_t chunkstoqueue /*= 4*/)
{
    Engine::Get()->AssertIfNotMainThread();

    if(!Pimpl || !data)
        return nullptr;

    const auto source = GetAudioSource();

    if(!source)
        return nullptr;

    source->PlayWithDecoder(data, chunksize, chunkstoqueue);
    return source;
}
// ------------------------------------ //
DLLEXPORT Sound::AudioBuffer::pointer SoundDevice::GetBufferFromFile(
    const std::string& filename, bool cache /*= true*/)
{
    const auto now = Time::GetTimeMs64();

    AudioBuffer::pointer buffer;

    // Find buffer from cache
    if(cache) {
        const auto found = Pimpl->BufferCache.find(filename);

        if(found != Pimpl->BufferCache.end()) {
            buffer = std::get<0>(found->second);
            std::get<1>(found->second) = now;
        }
    }

    if(!buffer) {
        // Not cached
        auto alureBuffer = Pimpl->Context.getBuffer(filename);

        if(!alureBuffer) {
            LOG_ERROR("SoundDevice: failed to create buffer from file: " + filename);
            return nullptr;
        }

        buffer = AudioBuffer::MakeShared<AudioBuffer>(std::move(alureBuffer), this);

        if(cache)
            Pimpl->BufferCache[filename] = {buffer, now};
    }

    return buffer;
}

DLLEXPORT AudioSource::pointer SoundDevice::GetAudioSource()
{
    auto alureSource = Pimpl->Context.createSource();

    if(!alureSource) {
        LOG_ERROR("SoundDevice: GetAudioSource: couldn't create alure source");
        return nullptr;
    }

    return AudioSource::MakeShared<AudioSource>(alureSource);
}
// ------------------------------------ //
DLLEXPORT void SoundDevice::BabysitAudio(AudioSource::pointer audio)
{
    Engine::Get()->AssertIfNotMainThread();

    Pimpl->HandledAudioSources.push_back(audio);
}
// ------------------------------------ //
DLLEXPORT void SoundDevice::ReportDestroyedBuffer(Sound::AudioBuffer& buffer)
{
    Pimpl->Context.removeBuffer(buffer.GetBuffer());
}
