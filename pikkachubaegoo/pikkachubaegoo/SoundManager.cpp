#include "SoundManager.h"
#include <fstream>
#include "Define.h"

USoundManager* USoundManager::instance = nullptr;

USoundManager::USoundManager()
{ }

USoundManager* USoundManager::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new USoundManager();
    }
    return instance;
}

bool USoundManager::LoadWavFile(const std::wstring& filePath, SoundData& outData)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) { return false; }

    char riffId[4], waveId[4];
    DWORD fileSize;
    file.read(riffId, 4);
    file.read(reinterpret_cast<char*>(&fileSize), sizeof(DWORD));
    file.read(waveId, 4);

    if (memcmp(riffId, "RIFF", 4) != 0 || memcmp(waveId, "WAVE", 4) != 0) {
        file.close();
        return false;
    }

    // fmt 청크와 data 청크를 찾을 때까지 반복
    WavChunk chunk;
    bool fmtFound = false;
    bool dataFound = false;
    DWORD dataStartOffset = 0;

    while (file.read(reinterpret_cast<char*>(&chunk), sizeof(WavChunk)))
    {
        if (memcmp(chunk.id, "fmt ", 4) == 0)
        {
            file.read(reinterpret_cast<char*>(&outData.wfx), chunk.size);
            fmtFound = true;
        }
        else if (memcmp(chunk.id, "data", 4) == 0)
        {
            dataStartOffset = (DWORD)file.tellg();
            dataFound = true;
            break;
        }
        else
        {
            // 건너뛰기
            file.seekg(chunk.size, std::ios::cur);
        }
    }

    // fmt/data 청크 확인
    if (!fmtFound || !dataFound) {
        file.close();
        return false;
    }

    // 오디오 데이터 읽기
    outData.audioBytes.resize(chunk.size);
    file.seekg(dataStartOffset);
    file.read(reinterpret_cast<char*>(outData.audioBytes.data()), chunk.size);

    file.close();
    return true;

}

void USoundManager::Init()
{
    XAudio2Create(&XAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    XAudio2->CreateMasteringVoice(&MasterVoice);

    if (LoadBGM(SOUND_KEY_BGM, L".\\Resource\\Sound\\bgm.wav"))
    {
        LogDebug(L"Load!!");
    }
}

void USoundManager::Release()
{
    if (currentBGMVoice)
    {
        currentBGMVoice->DestroyVoice();
        currentBGMVoice = nullptr;
    }
    for (auto& pair : sfxPoolMap)
    {
        for (IXAudio2SourceVoice* pVoice : pair.second)
        {
            pVoice->DestroyVoice();
        }
    }
    sfxPoolMap.clear();

    if (XAudio2)
    {
        XAudio2->Release();
        XAudio2 = nullptr;
    }
}

void USoundManager::PlayBGM(const std::wstring& key, bool loop)
{
    StopBGM();

    auto it = bgmMap.find(key);
    if (it == bgmMap.end()) return;

    SoundData& data = it->second;

    IXAudio2SourceVoice* pVoice = nullptr;
    HRESULT hr = XAudio2->CreateSourceVoice(&pVoice, &data.wfx);

    if (SUCCEEDED(hr))
    {
        XAUDIO2_BUFFER buffer = { 0 };
        buffer.AudioBytes = (UINT32)data.audioBytes.size();
        buffer.pAudioData = data.audioBytes.data();
        buffer.Flags = XAUDIO2_END_OF_STREAM;

        if (loop)
        {
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        }

        hr = pVoice->SubmitSourceBuffer(&buffer);
        if (SUCCEEDED(hr))
        {
            hr = pVoice->Start();
            currentBGMVoice = pVoice; // 현재 BGM 보이스 저장
        }
    }
}

void USoundManager::PlaySFX(const std::wstring& key)
{
    auto dataIt = sfxDataMap.find(key);
    if (dataIt == sfxDataMap.end()) return;
    SoundData& data = dataIt->second;

    auto poolIt = sfxPoolMap.find(key);
    if (poolIt == sfxPoolMap.end()) {
        sfxPoolMap[key] = std::vector<IXAudio2SourceVoice*>();
        poolIt = sfxPoolMap.find(key);
    }
    std::vector<IXAudio2SourceVoice*>& voicePool = poolIt->second;

    IXAudio2SourceVoice* pVoiceToPlay = nullptr;

    for (IXAudio2SourceVoice* pVoice : voicePool)
    {
        XAUDIO2_VOICE_STATE state;
        pVoice->GetState(&state);
        if (state.BuffersQueued == 0)
        {
            pVoiceToPlay = pVoice;
            break;
        }
    }

    if (pVoiceToPlay == nullptr)
    {
        HRESULT hr = XAudio2->CreateSourceVoice(&pVoiceToPlay, &data.wfx);
        if (SUCCEEDED(hr))
        {
            voicePool.push_back(pVoiceToPlay);
        }
        else
        {
            return;
        }
    }

    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = (UINT32)data.audioBytes.size();
    buffer.pAudioData = data.audioBytes.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    HRESULT hr = pVoiceToPlay->SubmitSourceBuffer(&buffer);
    if (SUCCEEDED(hr))
    {
        pVoiceToPlay->Start();
    }
}

void USoundManager::StopBGM()
{
    if (currentBGMVoice)
    {
        currentBGMVoice->Stop();
        currentBGMVoice->FlushSourceBuffers();
        currentBGMVoice->DestroyVoice();
        currentBGMVoice = nullptr;
    }
}

bool USoundManager::LoadBGM(const std::wstring& key, const std::wstring& filePath)
{    
    SoundData soundData;
    if (LoadWavFile(filePath, soundData))
    {
        bgmMap[key] = soundData;
        return true;
    }

    return false;
}

bool USoundManager::LoadSFX(const std::wstring& key, const std::wstring& filePath)
{    
    SoundData soundData;
    if (LoadWavFile(filePath, soundData))
    {
        sfxDataMap[key] = soundData;
        return true;
    }

    return false;
}
