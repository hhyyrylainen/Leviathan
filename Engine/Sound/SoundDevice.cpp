// ------------------------------------ //
#include "SoundDevice.h"

#include "SoundInternalTypes.h"

#include "Application/AppDefine.h"
#include "Common/StringOperations.h"
#include "Engine.h"
#include "ObjectFiles/ObjectFileProcessor.h"

#include "cAudio/cAudio.h"

#include <algorithm>
using namespace Leviathan;
using namespace Leviathan::Sound;
// ------------------------------------ //

SoundDevice::SoundDevice() {}
SoundDevice::~SoundDevice() {}
// ------------------------------------ //
bool SoundDevice::Init(bool simulatesound /*= false*/, bool noconsolelog /*= false*/)
{
    AudioLogPath = StringOperations::RemoveExtension(Logger::Get()->GetLogFile(), false) +
                   "cAudioLog.html";

    AudioManager = cAudio::createAudioManager(
        // No default init, we want to select the device
        false,
        // And write to a program specific log file
        AudioLogPath.c_str(), noconsolelog);

    LEVIATHAN_ASSERT(AudioManager, "Failed to create cAudio manager");

    size_t defaultDevice = 0;
    const auto devices = GetAudioDevices(&defaultDevice);

    if(simulatesound == true) {

        LOG_WARNING("SoundDevice: simulating not having a playing device");
        return true;
    }

    if(devices.empty() || defaultDevice >= devices.size()) {

        LOG_ERROR("SoundDevice: no sound devices detected");
        return false;
    }

    LOG_INFO("Detected audio devices: ");

    for(const auto& dev : devices)
        LOG_INFO("> " + dev);

    LOG_INFO("End of devices");

    std::string selectedDevice;
    // There's no print error here if missing to make tests run
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
        Engine::Get()->GetDefinition()->GetValues(), "AudioDevice", selectedDevice,
        devices[defaultDevice]);

    if(std::find(devices.begin(), devices.end(), selectedDevice) == devices.end()) {
        LOG_ERROR("SoundDevice: selected audio device \"" + selectedDevice +
                  "\" doesn't exists. Using default");
        selectedDevice = devices[defaultDevice];
    }

    LOG_INFO("SoundDevice: Initializing sound with device: " + selectedDevice);

    if(!AudioManager->initialize(selectedDevice.c_str())) {

        LOG_ERROR("SoundDevice: initializing failed");
        return false;
    }

    ListeningPosition = AudioManager->getListener();

    // setup global volume //
    SetGlobalVolume(1.f);

    ListeningPosition->setUpVector(cAudio::cVector3(0, 1, 0));

    return true;
}
void SoundDevice::Release()
{
    HandledAudioSources.clear();

    if(AudioManager) {

        cAudio::destroyAudioManager(AudioManager);
        AudioManager = nullptr;
    }
}
// ------------------------------------ //
void SoundDevice::Tick(int PassedMs)
{
    for(auto iter = HandledAudioSources.begin(); iter != HandledAudioSources.end(); ++iter) {

        if(!(*iter)->Get()->isPlaying()) {

            iter = HandledAudioSources.erase(iter);
        }
    }
}

DLLEXPORT void SoundDevice::SetSoundListenerPosition(
    const Float3& pos, const Float4& orientation)
{
    // we need to create a vector from the angles //
    // Float3 vec = Float3(-sin(pitchyawroll.X*DEGREES_TO_RADIANS),
    //     sin(pitchyawroll.Y*DEGREES_TO_RADIANS), -cos(pitchyawroll.X*DEGREES_TO_RADIANS));

    if(!ListeningPosition)
        return;

    ListeningPosition->move(cAudio::cVector3(pos.X, pos.Y, pos.Z));

    Ogre::Quaternion quaternion(orientation);

    Ogre::Radian angle;
    Ogre::Vector3 direction;

    quaternion.ToAngleAxis(angle, direction);

    ListeningPosition->setDirection(cAudio::cVector3(direction.x, direction.y, direction.z));
}

DLLEXPORT void SoundDevice::SetGlobalVolume(float vol)
{
    if(!AudioManager)
        return;

    std::clamp(vol, 0.f, 1.f);

    AudioManager->setMasterVolume(vol);
}
// ------------------------------------ //
DLLEXPORT void SoundDevice::Play2DSoundEffect(const std::string& filename)
{
    if(!AudioManager)
        return;

    cAudio::IAudioSource* source = AudioManager->play2D(filename.c_str(), false, false);

    if(source) {

        LOG_ERROR("SoundDevice: Play2DSoundEffect: shouldn't return a source but it did. "
                  "Babysitting it");

        Engine::Get()->RunOnMainThread(
            [=]() { this->BabysitAudio(AudioSource::MakeShared<AudioSource>(source, this)); });
    }
}

DLLEXPORT AudioSource::pointer SoundDevice::Play2DSound(
    const std::string& filename, bool looping, bool startpaused)
{
    if(!AudioManager)
        return nullptr;

    if(!looping && !startpaused)
        LOG_WARNING("SoundDevice: Play2DSoundEffect: called with same settings that "
                    "Play2DSoundEffect uses");

    cAudio::IAudioSource* source =
        AudioManager->play2D(filename.c_str(), looping, startpaused);

    if(!source)
        return nullptr;

    return AudioSource::MakeShared<AudioSource>(source, this);
}

DLLEXPORT AudioSource::pointer SoundDevice::CreateProceduralSound(
    ProceduralSoundData::pointer data, const char* soundname)
{
    if(!AudioManager || !data)
        return nullptr;

    return AudioSource::MakeShared<AudioSource>(
        AudioManager->createFromAudioDecoder(soundname, data->Properties.SourceName.c_str(),
            CAUDIO_NEW ProceduralSoundStream(data)),
        this);
}
// ------------------------------------ //
DLLEXPORT void SoundDevice::BabysitAudio(AudioSource::pointer audio)
{
    Engine::Get()->AssertIfNotMainThread();

    HandledAudioSources.push_back(audio);
}
// ------------------------------------ //
DLLEXPORT std::vector<std::string> SoundDevice::GetAudioDevices(
    size_t* indexofdefault /*= nullptr*/)
{
    std::vector<std::string> result;

    cAudio::IAudioDeviceList* devices = cAudio::createAudioDeviceList();

    if(!devices) {
        LOG_ERROR("SoundDevice: GetAudioDevices: failed to get audio device list");
        return {};
    }

    const auto deviceCount = devices->getDeviceCount();
    result.reserve(deviceCount);

    const auto defaultDeviceName = devices->getDefaultDeviceName();

    for(size_t i = 0; i < deviceCount; ++i) {

        const auto deviceName = devices->getDeviceName(i);

        if(deviceName.compare(defaultDeviceName) == 0 && indexofdefault) {

            *indexofdefault = i;
        }

#ifdef _WIN32
	    result.push_back(Convert::Utf16ToUtf8(deviceName));
#else
        result.push_back(deviceName);
#endif
    }

    CAUDIO_DELETE devices;

    return result;
}
