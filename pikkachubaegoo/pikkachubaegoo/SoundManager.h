#pragma once
#include <xaudio2.h>
#include <map>
#include <vector>
#include <string>

#define SOUND_KEY_BGM L"bgm"
#define SOUND_KEY_CHU L"chu"
#define SOUND_KEY_PIKA L"pika"
#define SOUND_KEY_SPIKE L"spike"
#define SOUND_KEY_BALL_LAND L"ballLand"

struct SoundData
{
    WAVEFORMATEX wfx;
    std::vector<BYTE> audioBytes;
};

struct WavChunk
{
    char id[4];
    DWORD size;
};

class USoundManager
{
// Singleton
private:
    USoundManager();
    ~USoundManager() = default;

public:
    static USoundManager* GetInstance();

private:
    static USoundManager* instance;

// Sound
public:
    void Init();
    void Release();

    void PlayBGM(const std::wstring& key, bool loop = true);
    void PlaySFX(const std::wstring& key);

    void StopBGM();

private:
    bool LoadWavFile(const std::wstring& filePath, SoundData& outData);
    bool LoadBGM(const std::wstring& key, const std::wstring& filePath);
    bool LoadSFX(const std::wstring& key, const std::wstring& filePath);

    // XAudio2
    IXAudio2* XAudio2;
    IXAudio2MasteringVoice* MasterVoice;

    // Sound Resource
    std::map<std::wstring, SoundData> bgmMap;
    std::map<std::wstring, SoundData> sfxDataMap;
    std::map<std::wstring, std::vector<IXAudio2SourceVoice*>> sfxPoolMap;
    IXAudio2SourceVoice* currentBGMVoice;
};

