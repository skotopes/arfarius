#ifndef MAC_MEDIA_KEYS_H
#define MAC_MEDIA_KEYS_H

#include <QObject>

class QColor;

class MacMediaKeys : public QObject
{
    Q_OBJECT
    enum PressState {
        NotPressed,
        Pressed,
        PressedEmmiting
    };

    int long_press_timeout;
    int long_press_cycle;

    PressState backward_state;
    int backward_timer_id;

    PressState forward_state;
    int forward_timer_id;

public:
    MacMediaKeys(QObject *parent = nullptr);
    virtual ~MacMediaKeys();

    void onPlayPauseKey(int keystate);
    void onBackwardKey(int keystate);
    void onForwardKey(int keystate);

protected:
    void timerEvent(QTimerEvent *event);

signals:
    void backward();
    void seekBackward();

    void playPause();

    void forward();
    void seekForward();
};

#endif
