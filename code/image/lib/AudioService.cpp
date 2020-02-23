#include "AudioService.h"
#include "constants.h"

AudioService::AudioService(int initialLevel)
{
    this->crossingSound.setSource(QUrl::fromLocalFile(CROSSING_SOUND));
    this->crossingSound.setLoopCount(QSoundEffect::Infinite);
    this->setLevel(initialLevel);
}

void AudioService::playCrossing()
{
    if (this->crossingSound.isPlaying()) return;
    this->crossingSound.play();
}

void AudioService::stopCrossing()
{
    if (!this->crossingSound.isPlaying()) return;
    this->crossingSound.stop();
}

void AudioService::setLevel(int level)
{
    this->level = level;
    this->crossingSound.setVolume(float(level)/100.0);
}

int AudioService::getLevel()
{
    return this->level;
}
