#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include <QSoundEffect>



class AudioService
{
public:
    AudioService(int initialLevel);

    void playCrossing();
    void stopCrossing();

    void setLevel(int level);
    int getLevel();
private:
    QSoundEffect crossingSound;
    int level;
};

#endif // AUDIOSERVICE_H
